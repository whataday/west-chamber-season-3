/*
 *	"UDPENCAP" target extension for iptables
 *	Copyright (c) Klzgrad <klzgrad@gmail.com>, 2011
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License; either
 *	version 2 of the License, or any later version, as published by the
 *	Free Software Foundation.
 */
#include <stdio.h>
#include <getopt.h>
#include <xtables.h>
#include <arpa/inet.h>
#include "xt_UDPENCAP.h"


enum {
	DECAP = 1 << 0,
	HAS_SPORT = 1 << 1,
	HAS_DPORT = 1 << 2,
};

static const struct option udpencap_tg_opts[] = {
	{.name = "decap", .has_arg = true, .val = 'p'},
	{.name = "sport", .has_arg = true, .val = 's'},
	{.name = "dport", .has_arg = true, .val = 'd'},
	{},
};

static void udpencap_tg_help(void)
{
	printf("UDPENCAP target options:\n"
"  --decap protocol		Decapsulate to this protocol\n"
"  --sport port                 Encapsulate with the source port\n"
"  --dport port			Encapsulate with the destination port\n");
}

static void udpencap_tg_init(struct xt_entry_target *target)
{
	struct xt_udpencap_tginfo *info = (void *)target->data;
	info->encap = true;
	info->proto = IPPROTO_UDP;
}

static int udpencap_tg_parse(int c, char **argv, int invert, unsigned int *flags,
    const void *entry, struct xt_entry_target **target)
{
	struct xt_udpencap_tginfo *info = (void *)(*target)->data;
	unsigned int proto, port;
	if (invert)
		xtables_error(PARAMETER_PROBLEM, "Can't specify ! before parameters");
	switch (c) {
	case 'p':
		xtables_param_act(XTF_ONLY_ONCE, "UDPENCAP", "--decap", *flags & DECAP);
		info->encap = false;
		if (!xtables_strtoui(optarg, NULL, &proto, 0, 255))
			xtables_param_act(XTF_BAD_VALUE, "UDPENCAP", "--decap", optarg);
		info->proto = proto;
		*flags |= DECAP;
		return true;
	case 's':
		xtables_param_act(XTF_ONLY_ONCE, "UDPENCAP", "--sport", *flags & HAS_SPORT);
		if (!xtables_strtoui(optarg, NULL, &port, 0, 65535))
			xtables_param_act(XTF_BAD_VALUE, "UDPENCAP", "--sport", optarg);
		info->sport = htons(port);
		*flags |= HAS_SPORT;
		return true;
	case 'd':
		xtables_param_act(XTF_ONLY_ONCE, "UDPENCAP", "--dport", *flags & HAS_DPORT);
		if (!xtables_strtoui(optarg, NULL, &port, 0, 65535))
			xtables_param_act(XTF_BAD_VALUE, "UDPENCAP", "--dport", optarg);
		info->dport = htons(port);
		*flags |= HAS_DPORT;
		return true;
	}
	return false;
}

static void udpencap_tg_check(unsigned int flags)
{
	bool encap = !(flags & DECAP);
	bool has_sport = flags & HAS_SPORT;
	bool has_dport = flags & HAS_DPORT;
	if (encap && (!has_sport || !has_dport))
		xtables_error(PARAMETER_PROBLEM, "UDPENCAP: \"--sport\" and \"--dport\" are required.");
}

static void udpencap_tg_print(const void *entry, const struct xt_entry_target *target, int numeric)
{
	const struct xt_udpencap_tginfo *info = (const void *)target->data;
	if (info->encap)
		printf(" encap udp sport %u dport %u\n", ntohs(info->sport), ntohs(info->dport));
	else
		printf(" decap %u", info->proto);
}

static void udpencap_tg_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_udpencap_tginfo *info = (const void *)target->data;
	if (info->encap)
		printf(" --sport %u --dport %u", ntohs(info->sport), ntohs(info->dport));
	else
		printf(" --decap %u", info->proto);
}

static struct xtables_target udpencap_tg4_reg = {
	.version       = XTABLES_VERSION,
	.name          = "UDPENCAP",
	.revision      = 0,
	.family        = NFPROTO_IPV4,
	.size          = XT_ALIGN(sizeof(struct xt_udpencap_tginfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_udpencap_tginfo)),
	.help          = udpencap_tg_help,
	.init          = udpencap_tg_init,
	.parse         = udpencap_tg_parse,
	.final_check   = udpencap_tg_check,
	.print         = udpencap_tg_print,
	.save          = udpencap_tg_save,
	.extra_opts    = udpencap_tg_opts,
};

static struct xtables_target udpencap_tg6_reg = {
	.version       = XTABLES_VERSION,
	.name          = "UDPENCAP",
	.revision      = 0,
	.family        = NFPROTO_IPV6,
	.size          = XT_ALIGN(sizeof(struct xt_udpencap_tginfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_udpencap_tginfo)),
	.help          = udpencap_tg_help,
	.init          = udpencap_tg_init,
	.parse         = udpencap_tg_parse,
	.final_check   = udpencap_tg_check,
	.print         = udpencap_tg_print,
	.save          = udpencap_tg_save,
	.extra_opts    = udpencap_tg_opts,
};
static __attribute__((constructor)) void udpencap_tg_ldr(void)
{
	xtables_register_target(&udpencap_tg4_reg);
	xtables_register_target(&udpencap_tg6_reg);
}
