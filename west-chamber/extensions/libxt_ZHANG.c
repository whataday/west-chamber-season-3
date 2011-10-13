/*
 *	"ZHANG" target extension for iptables
 *		copied from "DELUDE" target
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2008
 *	Klzgrad <klzgrad@gmail.com>, 2010
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License; either
 *	version 2 of the License, or any later version, as published by the
 *	Free Software Foundation.
 */
#include <stdio.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>

static void zhang_tg_help(void)
{
	printf("ZHANG takes no options\n");
}

static int zhang_tg_parse(int c, char **argv, int invert, unsigned int *flags,
    const void *entry, struct xt_entry_target **target)
{
	return 0;
}

static struct xtables_target zhang_tg_reg = {
	.version       = XTABLES_VERSION,
	.name          = "ZHANG",
	.revision      = 0,
	.family        = PF_INET,
	.help          = zhang_tg_help,
	.parse         = zhang_tg_parse,
};

static __attribute__((constructor)) void zhang_tg_ldr(void)
{
	xtables_register_target(&zhang_tg_reg);
}
