项目目的
--------
* 不需要服务器的本地代理工具。

DNS污染
-------
本工具运行在用户态，目前没有实现反DNS污染。关于反DNS污染，参考上一级目录的内容。

IP封锁
------
由于不依赖与第三方服务器，对于IP封锁也没有优美的解决方案。目前只是通过更新配置文件的方式，尽量避免IP封锁。

使用方法
--------
* Window
    1. 下载[客户端](https://github.com/downloads/liruqi/west-chamber-season-3/gproxy.zip)，解压缩，双击 
    2. (可选)安装[西厢的Windows 移植](http://west-chamber-season-3.googlecode.com/files/west-chamber-win-0.05.zip)。
    3. 把浏览器HTTP代理设置为 127.0.0.1:1998。暂时不支持 HTTPS

可用性
------
如果网站IP被封锁，那没办法。但是很多网站还是可以上的，比如 kenengba.com。不一一列举了。

问题反馈
--------
在[这里](https://github.com/liruqi/west-chamber-season-3/issues) 直接提供不能访问的网站。

软件更新
-------
日常会有配置文件更新。如果有程序的更新，会在下载页面中给出。

TODO
----
* 实现HTTPS 代理
* 实现应用层的反DNS污染。
* Mac / Linux 平台的binary 文件发布

