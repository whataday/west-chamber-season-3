#! /bin/sh

ipset -R < CHINA

iptables -A INPUT -p tcp -m tcp --tcp-flags RST RST -m set --match-set CHINA src -j DROP
