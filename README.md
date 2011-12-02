项目目的
--------
* 反污染的DNS
* 尽可能实现TCP连接混淆

DNS污染
-------
有两个办法可以实现反DNS污染

1. 自建DNS 服务器。

    系统要求：Linux 或 Mac。

    可以用dnsmasq 做本地的DNS服务器。只要把DNS服务器设置到国外，dnsmasq 可以很神奇地规避DNS 污染的问题。如果在国内有Linux服务器，建议做一个DNS服务小范围共享。我自己有维护一份 dnsmasq 的[配置文件](https://github.com/liruqi/kernet/blob/stable/kerdns/dnsmasq.conf)。

2. 设置国外的DNS 服务器，并本机丢弃GFW的 DNS伪包。

    a) 国外DNS服务器。大家比较熟悉的可能是[Google Public DNS](http://code.google.com/speed/public-dns/)。但是最近测试发现针对 Google DNS 的丢包很严重。先推荐两个，[V2EX DNS](http://dns.v2ex.com/) 和 [OpenDNS](http://www.opendns.com/)。

    b) 丢弃DNS伪包。
    * Windows: [west-chmber的windows 移植](http://code.google.com/p/west-chamber-season-3/downloads/detail?name=west-chamber-win-0.05.zip)
    * Mac OS X: [kernet](https://github.com/liruqi/kernet/downloads)。下最新的吧。运气好的话还能上blogspot。
    * Linux: (Linux平台上比较麻烦，先推荐使用dnsmasq，要喜欢折腾继续看。)需要有iptables。如果 iptables 有 u32模块(或者你能自己搞定安装一个)，可以直接用本项目中的 client.sh；否则，只能自己编译原始的[西厢项目](http://code.google.com/p/scholarzhang)，具体操作看西厢的文档吧。

TCP连接混淆
-----------
首先说明一下，[这东西很不靠谱](http://gfwrev.blogspot.com/2010/03/gfw.html)，容易受GFW 的更新而影响。感觉是，目前就kernet 项目效果还行。西厢的原始项目和Windows 移植现在都不好用。

我这个项目本来尝试在 Linux 下做一些改进。现在天冷也懒得动了。
