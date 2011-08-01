#! /bin/sh
# make sure to run as root

if [ ! -f CHINA ] 
    then wget "https://raw.github.com/liruqi/west-chamber-season-3/master/CHINA"
fi

ipset -R < CHINA

iptables -A INPUT -p tcp -m tcp --tcp-flags RST RST -m set ! --match-set CHINA src -j DROP

# you need to compile west-chamber from http://code.google.com/p/scholarzhang in advance
iptables -A INPUT -p udp -m udp --sport 53 -m state --state ESTABLISHED -m gfw -j DROP
