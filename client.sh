#! /bin/sh
# make sure to run as root

IPTABLES=`which iptables`
IPSET=`which ipset`

if [ -x "$IPTABLES" ]; then 
match_set=""

if [ -x "$IPSET" ]; then
    if $IPSET --version; then 
    if [ ! -f CHINA ]; then 
        wget "https://raw.github.com/liruqi/west-chamber-season-3/master/CHINA"
    fi
    match_set="-m set ! --match-set CHINA src"
    $IPSET -R < CHINA
    fi
fi

$IPTABLES -A INPUT -p tcp -m tcp --tcp-flags RST RST -m state --state ESTABLISHED $match_set -j DROP

# you need to compile west-chamber from http://code.google.com/p/scholarzhang in advance
$IPTABLES -A INPUT -p udp -m udp --sport 53 -m state --state ESTABLISHED -m gfw -j DROP

exit 0
fi


IPFW=`which ipfw`
if [ -x "$IPFW" ]; then
    $IPFW add 1000 drop tcp from any to me tcpflags rst in
fi
