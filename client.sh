#! /bin/sh
# make sure to run as root

if [ ! -f CHINA ] 
    then wget "https://raw.github.com/liruqi/west-chamber-season-3/master/CHINA"
fi

ipset -R < CHINA

iptables -A INPUT -p tcp -m tcp --tcp-flags RST RST -m set ! --match-set CHINA src -j DROP
iptables -A INPUT -p tcp -m tcp --tcp-flags RST,ACK RST,ACK -m set ! --match-set CHINA src -j DROP
