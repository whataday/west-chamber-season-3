项目目的
--------
* 不需要服务器的本地翻墙代理工具。
* [项目维护地址](https://github.com/liruqi/west-chamber-season-3/tree/master/west-chamber-proxy)

开发者
------
* [XIAOXIA](http://xiaoxia.org), 原始版本作者
* [LIRUQI](http://liruqi.info), 各平台的打包、发布

DNS污染
-------
有实现用户态反DNS污染。而且独立于系统的DNS配置。

IP封锁
------
由于不依赖与第三方服务器，对于IP封锁也没有优美的解决方案。目前只是通过更新配置文件的方式，尽量避免IP封锁。

可用性
------
本工具在联通的移动网络下不可用。
如果国外网站IP被封锁，使用本工具也无法访问。
另外，Android 客户端似乎设置了 https 代理，访问需要登陆的网站也有问题。

使用方法
--------
* Window

    1. 下载[客户端](https://github.com/downloads/liruqi/west-chamber-season-3/west-chamber-proxy-20111224.zip)，解压缩，双击 exe
    2. 把浏览器HTTP代理设置为 127.0.0.1:1998。

* Mac / Linux

    1. 提供了[pyc文件下载](https://github.com/downloads/liruqi/west-chamber-season-3/west-chamber-proxy-linux-20111124.zip)，解压缩，终端运行 python xxx.pyc
    2. 如果加一个额外的数字参数，可以换本地端口。
    3. 把浏览器HTTP代理设置为 127.0.0.1:1998。

* Android

    基于[GAE Proxy](http://code.google.com/p/gaeproxy/)修改的。下载 [apk文件](https://github.com/liruqi/west-chamber-season-3/west-chamber-proxy-20111223.apk/qr_code)。Google Market 也上线了，[地址](https://market.android.com/details?id=org.westchamberproxy)。

* iOS
    
    目前不打算自己做一个iOS 应用放在 appstore上。因为这需要做成浏览器，我不喜欢做自己不擅长而且重复的事情。iOS 上要使用代理有两个办法。(下载中提供了python27的 pyc，有兴趣的同学完全可以自己尝试。)

    1. 局域网内的其它设备(PC, Android 设备)上安装本代理，然后把 iOS 设备的 HTTP 代理设置到该设备上。（或者在国内有服务器的同学，自己搭建HTTP 代理）
    2. 类似GoAgent 那种iOS客户端的办法。需要越狱。单我本人不喜欢刷机越狱，所以，暂不研究了。
    
HTTPS 目前似乎有点问题。

问题反馈
--------
在[这里](https://github.com/liruqi/west-chamber-season-3/issues) 直接提供不能访问的网站。

软件更新
-------
日常会有配置文件更新。如果有程序的更新，会在下载页面中给出。

源代码
------
实际上我已经发布。需要的同学大概自己就能找到。但是找到的同学就别公开帮方老师做源码分析了。

TODO
----
* HTTPS 支持。至少做一个HTTPS下的DNS解析吧。
* 实现系统HTTP 代理的设置，这样系统自带的浏览器也可以用。
* 用 Java 重写代理逻辑，用户就不用下载依赖的 python 软件包。

UPDATE LOG
---
2011-11-23 解决android 客户端的远程 dns 解析的问题。
2011-11-24 对于IP被封锁的站点，走网页代理。

