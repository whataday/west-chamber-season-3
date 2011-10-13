/*
 *	"ZHANG" target extension for Xtables
 *	copied from "DELUDE" target
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2007 - 2008
 *      Klzgrad <klzgrad@gmail.com>, 2010
 *
 *	Based upon linux-2.6.18.5/net/ipv4/netfilter/ipt_REJECT.c:
 *	(C) 1999-2001 Paul `Rusty' Russell
 *	(C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/netfilter/x_tables.h>
#ifdef CONFIG_BRIDGE_NETFILTER
#	include <linux/netfilter_bridge.h>
#endif
#include <net/tcp.h>
#include "compat_xtables.h"
#define PFX KBUILD_MODNAME ": "

static void zhang_send_reset(struct sk_buff *oldskb, unsigned int hook)
{
	struct tcphdr _otcph, *tcph, *tcph2;
	const struct tcphdr *oth;
	const struct iphdr *oiph;
	struct sk_buff *skb, *skb2;
	struct iphdr *iph;
	unsigned int addr_type;

	oiph = ip_hdr(oldskb);

	/* do not deal with fragment */
	if (oiph->frag_off & htons(IP_OFFSET))
		return;

	oth = skb_header_pointer(oldskb, ip_hdrlen(oldskb),
				 sizeof(_otcph), &_otcph);
	if (oth == NULL)
		return;

	/* syn[ack] only */
	if (!oth->syn || !oth->ack || oth->rst || oth->fin)
		return;

	/* check checksum */
	if (nf_ip_checksum(oldskb, hook, ip_hdrlen(oldskb), IPPROTO_TCP))
		return;

	skb = alloc_skb(sizeof(struct iphdr) + sizeof(struct tcphdr) +
	                 LL_MAX_HEADER, GFP_ATOMIC);
	if (skb == NULL)
		return;

	/* construct from scratch */
	skb_reserve(skb, LL_MAX_HEADER);
	skb_reset_network_header(skb);
	iph = (struct iphdr *)skb_put(skb, sizeof(struct iphdr));
	iph->version  = 4;
	iph->ihl      = sizeof(struct iphdr) / 4;
	iph->tos      = 0;
	iph->id       = 0;
	iph->frag_off = htons(IP_DF);
	iph->protocol = IPPROTO_TCP;
	iph->check    = 0;
	iph->saddr    = oiph->daddr;
	iph->daddr    = oiph->saddr;

	tcph = (struct tcphdr *)skb_put(skb, sizeof(struct tcphdr));
	memset(tcph, 0, sizeof(*tcph));
	tcph->source = oth->dest;
	tcph->dest   = oth->source;
	tcph->doff   = sizeof(struct tcphdr) / 4;
	tcph->window = 0xffffU;

	/*
	 * essential part 1
	 * inject an FIN with bad sequence number, obfuscating the handshake.
	 * it will be dropped by rfc-compliant endpoint, 
	 * meanwhile thwarting eavesdroppers on the same direction (c -> s).
	 */
	tcph->fin     = true;
	tcph->seq     = oth->ack_seq;
	tcph->ack_seq = oth->seq;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 20)
	tcph->check = tcp_v4_check(tcph, sizeof(struct tcphdr), iph->saddr,
	              iph->daddr, csum_partial((char *)tcph,
	              sizeof(struct tcphdr), 0));
#else
	tcph->check = tcp_v4_check(sizeof(struct tcphdr), iph->saddr,
	              iph->daddr, csum_partial((char *)tcph,
	              sizeof(struct tcphdr), 0));
#endif

	addr_type = RTN_UNSPEC;
#ifdef CONFIG_BRIDGE_NETFILTER
	if (hook != NF_INET_FORWARD || (skb->nf_bridge != NULL &&
	    skb->nf_bridge->mask & BRNF_BRIDGED))
#else
	if (hook != NF_INET_FORWARD)
#endif
		addr_type = RTN_LOCAL;

	/* ip_route_me_harder expects skb->dst to be set */
	skb_dst_set(skb, dst_clone(skb_dst(oldskb)));

	if (ip_route_me_harder(&skb, addr_type))
		goto free_skb;
	else
		iph = ip_hdr(skb);

	iph->ttl       = dst_metric(skb_dst(skb), RTAX_HOPLIMIT);
	skb->ip_summed = CHECKSUM_NONE;

	/* "Never happens" */
	if (skb->len > dst_mtu(skb_dst(skb)))
		goto free_skb;

	skb2 = skb_copy(skb, GFP_ATOMIC);
	if (skb2 == NULL)
		goto free_skb;

	/*
	 * essential part 2
	 * inject an ACK with correct SEQ but bad ACK.
	 * this causes an RST from server which should have no real impact on 
	 * the original connection,
	 * thus thwarts eavesdroppers on the other direction (s -> c).
	 * 
	 * RFC793: 
	 *   2.  If the connection is in any non-synchronized state (LISTEN,
	 *   SYN-SENT, SYN-RECEIVED), and the incoming segment acknowledges
	 *   something not yet sent (the segment carries an unacceptable ACK),
	 *   ..., a reset is sent.
	 * 
	 *   If the incoming segment has an ACK field, the reset takes its
	 *   sequence number from the ACK field of the segment, otherwise the
	 *   reset has sequence number zero and the ACK field is set to the sum
	 *   of the sequence number and segment length of the incoming segment.
	 *   The connection remains in the same state.
	 * 
	 * sometimes certain kind of rfc non-compliant tcp stacks or firewalls
	 * may have unexpected response or no reply at all.
	 *
	 * seems that the seq is not nessesarily correct.
	 */
	tcph2 = (struct tcphdr *)(skb_network_header(skb2) + ip_hdrlen(skb2));
	tcph2->fin     = false;
	tcph2->ack     = true;
	//tcph2->seq     = oth->ack_seq;
	//tcph2->ack_seq = oth->seq;

	/* checksum shortcut */
	//csum_replace4(&tcph2->check, tcph->seq, tcph2->seq);
	//csum_replace4(&tcph2->check, tcph->ack_seq, tcph2->ack_seq);
	csum_replace2(&tcph2->check, htons(((u_int8_t *)tcph)[13]), 
			htons(((u_int8_t *)tcph2)[13]));

	ip_local_out(skb);
	ip_local_out(skb2);

	return;
free_skb:
	kfree_skb(skb);
}

static unsigned int
zhang_tg(struct sk_buff **pskb, const struct xt_action_param *par)
{
	/* WARNING: This code causes reentry within iptables.
	   This means that the iptables jump stack is now crap.  We
	   must return an absolute verdict. --RR */
	zhang_send_reset(*pskb, par->hooknum);
	return NF_ACCEPT;
}

static struct xt_target zhang_tg_reg __read_mostly = {
	.name     = "ZHANG",
	.revision = 0,
	.family   = NFPROTO_IPV4,
	.table    = "filter",
	.hooks    = (1 << NF_INET_LOCAL_IN),
	.proto    = IPPROTO_TCP,
	.target   = zhang_tg,
	.me       = THIS_MODULE,
};

static int __init zhang_tg_init(void)
{
	return xt_register_target(&zhang_tg_reg);
}

static void __exit zhang_tg_exit(void)
{
	xt_unregister_target(&zhang_tg_reg);
}

module_init(zhang_tg_init);
module_exit(zhang_tg_exit);
MODULE_DESCRIPTION("Xtables: Scholar Zhang");
MODULE_AUTHOR("Jan Engelhardt <jengelh@medozas.de>");
MODULE_AUTHOR("Klzgrad <klzgrad@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_ZHANG");
