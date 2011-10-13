#ifndef _LINUX_NETFILTER_XT_TARGET_UDPENCAP
#define _LINUX_NETFILTER_XT_TARGET_UDPENCAP 1

struct xt_udpencap_tginfo {
	bool encap;
	__u8 proto;
	__be16 sport;
	__be16 dport;
};
#endif /* _LINUX_NETFILTER_XT_TARGET_UDPENCAP */
