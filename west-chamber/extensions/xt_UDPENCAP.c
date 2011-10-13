/*
 *	"UDPENCAP" target extension for Xtables
 *      Klzgrad <kizdiv@gmail.com>, 2011
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <linux/netfilter/x_tables.h>
#include "compat_xtables.h"

#include "xt_UDPENCAP.h"

#define PFX KBUILD_MODNAME ": "

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
# define WITH_IPV6 1
#endif

static bool udpencap_insert_header(struct sk_buff *skb, const struct xt_udpencap_tginfo *info)
{
	struct udphdr *uh;
	if (!skb_make_writable(&skb, skb_transport_offset(skb)))
		return false;

	if (skb->len + sizeof(struct udphdr) > 65535)
		return false;

	if (skb_cow(skb, sizeof(struct udphdr) + LL_RESERVED_SPACE(skb_dst(skb)->dev)))
		return false;

	memmove(skb->data - sizeof(struct udphdr), skb->data, skb_transport_offset(skb));
	__skb_push(skb, sizeof(struct udphdr));
	skb->network_header -= sizeof(struct udphdr);
	skb->transport_header -= sizeof(struct udphdr);

	uh = udp_hdr(skb);
	uh->source = info->sport;
	uh->dest = info->dport;
	uh->len = htons(skb->len - skb_transport_offset(skb));
	uh->check = 0;

	return true;
}

static bool udpencap_remove_header(struct sk_buff *skb, const struct xt_udpencap_tginfo *info)
{
	unsigned int nlen = skb_transport_offset(skb);
	if (!skb_make_writable(&skb, skb_transport_offset(skb) + sizeof(struct udphdr)))
		return false;

	if (skb->len < nlen + sizeof(struct udphdr))
		return false;

	if (!pskb_pull(skb, sizeof(struct udphdr)))
		return false;
	skb_postpull_rcsum(skb, skb_transport_header(skb), sizeof(struct udphdr));
	memmove(skb->data, skb->data - sizeof(struct udphdr), nlen);
	skb->network_header += sizeof(struct udphdr);
	skb->transport_header += sizeof(struct udphdr);

	return true;
}

static void udpencap_fix4(struct sk_buff *skb, const struct xt_udpencap_tginfo *info)
{
	struct iphdr *iph = ip_hdr(skb);
	bool fix_csum = (skb->ip_summed == CHECKSUM_COMPLETE && !info->encap);
	__be16 newlen = htons(ntohs(iph->tot_len) + (info->encap ? 1 : -1) * sizeof(struct udphdr));
	if (fix_csum) {
		skb->csum = csum_sub(skb->csum, csum_partial(&iph->tot_len, 2, 0));
		skb->csum = csum_sub(skb->csum, csum_partial(&iph->protocol, 3, 0));
	}
	csum_replace2(&iph->check, iph->tot_len, newlen);
	iph->tot_len = newlen;
	if (iph->protocol != info->proto) {
		csum_replace2(&iph->check, htons(iph->protocol), htons(info->proto));
		iph->protocol = info->proto;
	}
	if (fix_csum) {
		skb->csum = csum_add(skb->csum, csum_partial(&iph->tot_len, 2, 0));
		skb->csum = csum_add(skb->csum, csum_partial(&iph->protocol, 3, 0));
	}
}

static void udpencap_fix6(struct sk_buff *skb, const struct xt_udpencap_tginfo *info)
{
	struct ipv6hdr *iph = ipv6_hdr(skb);
	bool fix_csum = (skb->ip_summed == CHECKSUM_COMPLETE && !info->encap);
	if (fix_csum)
		skb->csum = csum_sub(skb->csum, csum_partial(&iph->payload_len, 3, 0));
	iph->payload_len = htons(ntohs(iph->payload_len) + (info->encap ? 1 : -1) * sizeof(struct udphdr));
	iph->nexthdr = info->proto;
	if (fix_csum)
		skb->csum = csum_add(skb->csum, csum_partial(&iph->payload_len, 3, 0));
}

static unsigned int udpencap_tg(struct sk_buff **pskb, const struct xt_action_param *par)
{
	const struct xt_udpencap_tginfo *info = par->targinfo;
	struct sk_buff *skb = *pskb;
	unsigned int tproto, nlen;
	bool ipv4 = (par->family == NFPROTO_IPV4);

	if (ipv4) {
		nlen = ip_hdrlen(skb);
		if (nlen < sizeof(struct iphdr))
			return NF_DROP;
		tproto = ip_hdr(skb)->protocol;
	} else {
		nlen = sizeof(struct ipv6hdr);
		tproto = ipv6_hdr(skb)->nexthdr;
	}
	if (!info->encap && tproto != IPPROTO_UDP)
		return NF_DROP;
	skb_set_transport_header(skb, skb_network_offset(skb) + nlen);
	if (!(info->encap ? udpencap_insert_header : udpencap_remove_header)(skb, info))
		return NF_DROP;
	(ipv4 ? udpencap_fix4 : udpencap_fix6)(skb, info);
	return XT_CONTINUE;
}

static int udpencap_tg_check(const struct xt_tgchk_param *par)
{
	const struct xt_udpencap_tginfo *info = par->targinfo;
	if (info->encap && info->proto != IPPROTO_UDP) {
		pr_err(PFX "encap protocol is not UDP\n");
		return -EINVAL;
	}
	return 0;
}

static struct xt_target udpencap_tg_reg[] __read_mostly = {
{
	.name       = "UDPENCAP",
	.revision   = 0,
	.family     = NFPROTO_IPV4,
	.table      = "mangle",
	.target     = udpencap_tg,
	.targetsize = sizeof(struct xt_udpencap_tginfo),
	.checkentry = udpencap_tg_check,
	.me         = THIS_MODULE,
},
#ifdef WITH_IPV6
{
	.name       = "UDPENCAP",
	.revision   = 0,
	.family     = NFPROTO_IPV6,
	.table      = "mangle",
	.target     = udpencap_tg,
	.targetsize = sizeof(struct xt_udpencap_tginfo),
	.checkentry = udpencap_tg_check,
	.me         = THIS_MODULE,
},
#endif
};

static int __init udpencap_tg_init(void)
{
	return xt_register_targets(udpencap_tg_reg, ARRAY_SIZE(udpencap_tg_reg));
}

static void __exit udpencap_tg_exit(void)
{
	xt_unregister_targets(udpencap_tg_reg, ARRAY_SIZE(udpencap_tg_reg));
}

module_init(udpencap_tg_init);
module_exit(udpencap_tg_exit);
MODULE_DESCRIPTION("Xtables: Unconditional transport layer UDP encapsulation and decapsulation");
MODULE_AUTHOR("Klzgrad <kizdiv@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_UDPENCAP");
MODULE_ALIAS("ip6t_UDPENCAP");
