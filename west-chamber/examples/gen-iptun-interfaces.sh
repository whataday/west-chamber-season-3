#!/bin/sh
# Generate a pair of ipip or ipip6 tunnels interfaces(5) configuration for
# ifup and ifdownwith minimal IPsec encryption and UDP encapsulation. In this
# script the two endpoints are referred to as server and client for easy
# understanding, which are not necessarily server and client.

# Define interface names on the server and client.
prefix=tun-
sname=server
cname=client

# Tunnel interfaces are bound to the devices on the server and client.
sdev=eth0
cdev=eth0

# External addresses of the server and client. Protocol family is
# automatically determined.
sa_ex=2001:db8::1
ca_ex=2001:db8::2
#sa_ex=192.0.2.1
#ca_ex=192.0.2.2

# Internal addresses of the server and client.
sa_in=10.0.0.1
ca_in=10.0.0.2

# Port pair used in UDP encapsulation.
sp=53
cp=14404

# Generate random SPI and encryption key
alias random='printf 0x%s `od -N4 -tx4 -An /dev/urandom`'
s2c_spi=`random`
s2c_key=`random`
c2s_spi=`random`
c2s_key=`random`

## Configuration ends here. ##

if [ ${sa_ex#*:} != $sa_ex ]; then
	v=6
	ipf=' -6'
fi
# ether(1500)-ip6(40)-udp(8)-esp(10+padding)
#	pre-up ip link set \$IFACE mtu 1442
echo "# ip6_tunnel MUST be inserted before.
# server $prefix$cname /etc/network/interfaces:
iface $prefix$cname inet manual
	pre-up ip${v}tables -t mangle -A POSTROUTING -d $ca_ex -p ipencap -j UDPENCAP --sport $sp --dport $cp
	pre-up ip${v}tables -t mangle -A INPUT -s $ca_ex -p udp --sport $cp --dport $sp -j UDPENCAP --decap 4
	pre-up ip xfrm state add src $sa_ex dst $ca_ex proto esp spi $s2c_spi enc blowfish $s2c_key
	pre-up ip xfrm state add src $ca_ex dst $sa_ex proto esp spi $c2s_spi enc blowfish $c2s_key
	pre-up ip xfrm policy add dir out src $sa_ex dst $ca_ex proto udp sport $sp dport $cp tmpl proto esp
	pre-up ip xfrm policy add dir in src $ca_ex dst $sa_ex proto udp sport $cp dport $sp tmpl proto esp
	pre-up ip$ipf tunnel add \$IFACE mode ipip$v remote $ca_ex local $sa_ex dev $sdev
	pre-up sysctl -q -w net.ipv6.conf.\$IFACE.disable_ipv6=1
	pre-up ip link set \$IFACE txqueuelen 1000
	up ip link set \$IFACE up
	post-up ip addr add $sa_in peer $ca_in dev \$IFACE
	down ip link set \$IFACE down
	post-down ip$ipf tunnel del \$IFACE
	post-down ip xfrm policy del dir in src $ca_ex dst $sa_ex proto udp sport $cp dport $sp
	post-down ip xfrm policy del dir out src $sa_ex dst $ca_ex proto udp sport $sp dport $cp
	post-down ip xfrm state del src $ca_ex dst $sa_ex proto esp spi $c2s_spi
	post-down ip xfrm state del src $sa_ex dst $ca_ex proto esp spi $s2c_spi
	post-down ip${v}tables -t mangle -D INPUT -s $ca_ex -p udp --sport $cp --dport $sp -j UDPENCAP --decap 4
	post-down ip${v}tables -t mangle -D POSTROUTING -d $ca_ex -p ipencap -j UDPENCAP --sport $sp --dport $cp
	

# client $prefix$sname /etc/network/interfaces:
iface $prefix$sname inet manual
	pre-up ip${v}tables -t mangle -A POSTROUTING -d $sa_ex -p ipencap -j UDPENCAP --sport $cp --dport $sp
	pre-up ip${v}tables -t mangle -A INPUT -s $sa_ex -p udp --sport $sp --dport $cp -j UDPENCAP --decap 4
	pre-up ip xfrm state add src $ca_ex dst $sa_ex proto esp spi $c2s_spi enc blowfish $c2s_key
	pre-up ip xfrm state add src $sa_ex dst $ca_ex proto esp spi $s2c_spi enc blowfish $s2c_key
	pre-up ip xfrm policy add dir out src $ca_ex dst $sa_ex proto udp sport $cp dport $sp tmpl proto esp
	pre-up ip xfrm policy add dir in src $sa_ex dst $ca_ex proto udp sport $sp dport $cp tmpl proto esp
	pre-up ip$ipf tunnel add \$IFACE mode ipip$v remote $sa_ex local $ca_ex dev $sdev
	pre-up sysctl -q -w net.ipv6.conf.\$IFACE.disable_ipv6=1
	pre-up ip link set \$IFACE txqueuelen 1000
	up ip link set \$IFACE up
	post-up ip addr add $ca_in peer $sa_in dev \$IFACE
	post-up ip route add 0/1 via $sa_in
	post-up ip route add 128.0/1 via $sa_in
	down ip link set \$IFACE down
	post-down ip$ipf tunnel del \$IFACE
	post-down ip xfrm policy del dir in src $sa_ex dst $ca_ex proto udp sport $sp dport $cp
	post-down ip xfrm policy del dir out src $ca_ex dst $sa_ex proto udp sport $cp dport $sp
	post-down ip xfrm state del src $sa_ex dst $ca_ex proto esp spi $s2c_spi
	post-down ip xfrm state del src $ca_ex dst $sa_ex proto esp spi $c2s_spi
	post-down ip${v}tables -t mangle -D INPUT -s $sa_ex -p udp --sport $sp --dport $cp -j UDPENCAP --decap 4
	post-down ip${v}tables -t mangle -D POSTROUTING -d $sa_ex -p ipencap -j UDPENCAP --sport $cp --dport $sp"
