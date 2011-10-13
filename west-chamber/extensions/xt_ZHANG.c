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
#include <linux/delay.h>
#include <linux/netfilter/x_tables.h>
#ifdef CONFIG_BRIDGE_NETFILTER
#	include <linux/netfilter_bridge.h>
#endif
#include <net/tcp.h>
#include "compat_xtables.h"
#define PFX KBUILD_MODNAME ": "

static int zhang_inject_tcp_output_from_params(struct sk_buff *oldskb, unsigned int hook, __be32 tcph_flags, __be32 iph_saddr, __be32 iph_daddr, __be16 tcph_sport, __be16 tcph_dport, __be32 tcph_seq, __be32 tcph_ack_seq, __be16 tcph_window) 
{
    struct tcphdr *tcph;
    struct sk_buff *skb;
	struct iphdr *iph;
	unsigned int addr_type;
    
	skb = alloc_skb(sizeof(struct iphdr) + sizeof(struct tcphdr) +
	                 LL_MAX_HEADER, GFP_ATOMIC);
	if (skb == NULL)
        return 1;

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
	iph->saddr    = iph_saddr;
	iph->daddr    = iph_daddr;

	tcph = (struct tcphdr *)skb_put(skb, sizeof(struct tcphdr));
	memset(tcph, 0, sizeof(*tcph));
	tcph->source = tcph_sport;
	tcph->dest   = tcph_dport;
	tcph->doff   = sizeof(struct tcphdr) / 4;
	tcph->window = tcph_window;
    
    if (tcph_flags & TCP_FLAG_RST) tcph->rst     = true;
    if (tcph_flags & TCP_FLAG_ACK) tcph->ack     = true;

    tcph->seq     = tcph_seq;
    tcph->ack_seq = tcph_ack_seq;
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

	ip_local_out(skb);
    return 0;

free_skb:
	kfree_skb(skb);
    return 2;
}

static void zhang_send_reset(struct sk_buff *oldskb, unsigned int hook)
{
	struct tcphdr _otcph;
	const struct tcphdr *oth;
	const struct iphdr *oiph;

    __be32 saddr;
	__be32 daddr;
	__be16 sport;
	__be16 dport;
	__be32 ack;
	__be32 seq;
    int retval = 0;
	
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
    
    saddr = oiph->daddr;
    daddr = oiph->saddr;
    
    sport = oth->dest;
    dport = oth->source;


	/*
	 * 1. RST
	 */
    seq = htonl(ntohl(oth->ack_seq) - 1);
    ack = oth->seq;

    retval = zhang_inject_tcp_output_from_params(oldskb, hook, TCP_FLAG_RST, saddr, daddr, sport, dport, seq, ack, htons(0x0001U));
    if (retval) {
        return;
    }

	/*
	 * 2. ACK 
	 */
    seq = oth->ack_seq;
    ack = oth->seq;

    retval = zhang_inject_tcp_output_from_params(oldskb, hook, TCP_FLAG_ACK, saddr, daddr, sport, dport, seq, ack, htons(0x0001U));
    if (retval) {
        return;
    }
    
    /*
     * 3. RST + ACK
     */
    seq = htonl(ntohl(oth->ack_seq) + 0);
    ack = htonl(ntohl(oth->seq) + 0);
    retval = zhang_inject_tcp_output_from_params(oldskb, hook, TCP_FLAG_ACK|TCP_FLAG_RST, saddr, daddr, sport, dport, seq, ack, htons(0x0001U));
    if (retval) {
        return;
    }

    /*
     * 4. RST + ACK
     */
    seq = htonl(ntohl(oth->ack_seq) + 2);
    ack = htonl(ntohl(oth->seq) + 2);
    retval = zhang_inject_tcp_output_from_params(oldskb, hook, TCP_FLAG_ACK|TCP_FLAG_RST, saddr, daddr, sport, dport, seq, ack, htons(0x0001U));
    if (retval) {
        return;
    }
    
    // the delay may hold up the system
    // mdelay(200);
	return;
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
