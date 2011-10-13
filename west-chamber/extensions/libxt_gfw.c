#include <stdio.h>
#include <xtables.h>

static void gfw_mt_help(void)
{
	printf("gfw takes no options\n");
}

static int gfw_mt_parse(int c, char **argv, int invert, unsigned int *flags,
                           const void *entry, struct xt_entry_match **match)
{
	return 0;
}

static struct xtables_match gfw_mt_reg = {
	.version        = XTABLES_VERSION,
	.name           = "gfw",
	.revision       = 0,
	.family         = PF_INET,
	.help           = gfw_mt_help,
	.parse          = gfw_mt_parse,
};

static void _init(void)
{
	xtables_register_match(&gfw_mt_reg);
}
