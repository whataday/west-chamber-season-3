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

使用方法
--------
* Window

    1. 下载[客户端](https://github.com/downloads/liruqi/west-chamber-season-3/west-chamber-proxy-20111224.zip)，解压缩，双击 exe
    2. 把浏览器HTTP代理设置为 127.0.0.1:1998。
* Mac OS X (目前仅支持 64位系统)

    1. 下载[客户端](https://github.com/downloads/liruqi/west-chamber-season-3/west-chamber-proxy-mac-x64-20111217.zip)，解压缩，双击 gproxy
    2. 把浏览器HTTP代理设置为 127.0.0.1:1998。
* Android

    基于[GAE Proxy](http://code.google.com/p/gaeproxy/)修改的。下载 [apk文件](https://github.com/liruqi/west-chamber-season-3/west-chamber-proxy-20111223.apk/qr_code)
    
HTTPS 目前似乎有点问题。

可用性
------
如果网站IP被封锁，那没办法。但是很多网站可以上的，比如 kenengba.com。不一一列举了。

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
* Linux, iOS 平台的文件发布

UPDATE LOG
---
2011-11-23 解决android 客户端的远程 dns 解析的问题。
2011-11-24 对于IP被封锁的站点，走网页代理。

