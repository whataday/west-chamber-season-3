/*
 *	"gfw" match extension for Xtables
 *
 *	Copyright © Klzgrad <klzgrad@gmail.com>, 2010
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License; either version 2
 *	or 3 of the License, as published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter/x_tables.h>
#include "compat_xtables.h"

static bool
gfw_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct iphdr *iph;
	struct tcphdr _tcph, *th;
	struct udphdr _udph, *uh;
	__u16 window;
	__be32 addr, ttl;
	__be16 name;
	__u8 *end;
	__be16 *dns;

	iph = ip_hdr(skb);

	/* no fragment */
	if (iph->frag_off & htons(IP_OFFSET))
		return false;

	if (iph->protocol == IPPROTO_TCP) {
		th = skb_header_pointer(skb, par->thoff,
					sizeof(_tcph), &_tcph);
		if (th == NULL || th->doff * 4 < sizeof(struct tcphdr)) {
			par->hotdrop = true;
			return false;
		}

		/* no tcp options */
		if (th->doff * 4 != sizeof(struct tcphdr))
			return false;

		window = ntohs(th->window);
		if (iph->frag_off & htons(IP_DF)) {
			if ((th->rst || th->syn) && th->ack && !th->fin
			&& (ntohs(iph->id) == (__u16)(-1 - window * 13)
			|| ntohs(iph->id) == (__u16)(62753 - window * 79)))
				return true; /* type2[a] */
		} else {
			if (iph->id == htons(64)
			&& th->rst && !th->ack && !th->syn && !th->fin
			&& (window % 17 == 0
			|| (window - ntohs(th->source) / 2) % 9 == 0))
				return true; /* type1[a] */
		}
	} else if (iph->protocol == IPPROTO_UDP) {
		uh = skb_header_pointer(skb, par->thoff,
					sizeof(_udph), &_udph);
		if (uh == NULL || ntohs(uh->len) < sizeof(struct udphdr)) {
			par->hotdrop = true;
			return false;
		}

		/* DF not set and enough space for dns record */
		if (iph->frag_off & htons(IP_DF)
		|| ntohs(uh->len) < sizeof(struct udphdr) + 12 + 16)
			return false;

		end = (__u8 *)uh + ntohs(uh->len);
		dns = (__be16 *)((__u8 *)uh + sizeof(struct udphdr));

		/*
		 * questions: 1; answer RRs: 1; 
		 * authority RRs: 0; Additional RRs: 0;
		 * Type: A; Class: IN;
		 */
		if (dns[2] != htons(1) || dns[3] != htons(1)
		|| dns[4] != 0 || dns[5] != 0
		|| *(__be32 *)(end - 14) != htonl(0x00010001))
			return false;

		addr = *(__be32 *)(end - 4);
		ttl = *(__be32 *)(end - 10);
		name = *(__be16 *)(end - 16);

		/* dns[1]: dns flags */
		if (addr == htonl(0x5d2e0859) || addr == htonl(0xcb620741) ||
		    addr == htonl(0x0807c62d) || addr == htonl(0x4e10310f) ||
		    addr == htonl(0x2e52ae44) || addr == htonl(0xf3b9bb27) ||
		    addr == htonl(0xf3b9bb1e) || addr == htonl(0x9f6a794b) ||
		    addr == htonl(0x253d369e) || addr == htonl(0x9f1803ad) ||
		    addr == htonl(0x3b1803ad))
			return true;
	}

	return false;
}

static struct xt_match gfw_mt_reg __read_mostly = {
	.name       = "gfw",
	.revision   = 0,
	.family     = NFPROTO_IPV4,
	.match      = gfw_mt,
	.me         = THIS_MODULE,
};

static int __init gfw_mt_init(void)
{
	return xt_register_match(&gfw_mt_reg);
}

static void __exit gfw_mt_exit(void)
{
	xt_unregister_match(&gfw_mt_reg);
}

module_init(gfw_mt_init);
module_exit(gfw_mt_exit);
MODULE_AUTHOR("Klzgrad <klzgrad@gmail.com>");
MODULE_DESCRIPTION("match gfw fingerprints");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_gfw");
