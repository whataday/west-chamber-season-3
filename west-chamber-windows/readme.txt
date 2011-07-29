WestChamber alpha - Windows移植说明

简介： 
WestChamber的Windows移植版本。 
采用NDIS驱动，基于Microsoft的Passthru源代码和WestChamber Linux ver源代码完成。 
（在项目Download页可下载）。 


当前进度： 
完成了DNS抗混淆部分(gfw)和ZHANG模块。 


TODO: 
完成CUI模块。加入GUI。 


适用Windows版本： 
Wwindows XP-Windows 7理论上通用。 


注意： 
本移植版本处于刚起步的部分，稳定性还没有经过大规模验证。 
驱动程序运行失败可能导致无法正常进入系统。 
请在安装前备份自己重要的数据，并做好系统还原点的设置。 
如果无法进入系统，请进入Windows安全模式，按下文方式卸载。 


简单来说： 
如果您不确定自己在做什么，便请耐心等待我们推出更完善的版本。 

64位系统 - 驱动签名：
Windows 64位系统强制要求所有驱动程序签名。
如果希望试用64位版本，在安装之前，
请使用附带的dseo13b软件打开系统的Test Mode，
然后手动给westchamber.sys签名(sign a system file)。

安装说明： 
运行BindView。 
点Install，选择Service，选择从磁盘安装，选择driver目录的netsf.inf。 
如果弹出『未经数字签名』窗口，点『确定』/『依然安装』。 
之后在网卡的设置内（或在BindView的Service列表内）应该可以看到westchamber driver的字样，即说明安装成功。 


使用说明： 
运行DbgView，Capture - Capture Kernel（或快捷键Ctrl + K）可看到提示信息。 
如果无法看到调试信息，请将DbgView中的Capture - Enable Verbose Kernel Output选项打开。

DNS抗混淆： 
如果发现符合DNS伪数据包指纹的包，将会提示Detected GFW Poinsoned DNS data，并将包丢弃。 


IP筛选： 
驱动在C盘根目录读取iplog.bin，结构如下： 
struct 
{ 
        unsigned int32 dwNum; 
        unsigned int32 big_endian dwIpAddress[dwNum]; 
} 


【请注意目前没有支持动态加载/更改iplog，所以如果需要更改iplog请重新卸载-安装驱动或重启系统。】 
安装包内包含的iplog.bin是根据WestChamber Linux版本内含的IP列表所生成，包含Youtube和其他Google服务 
的IP地址。 

卸载说明： 
BindView -> Uninstall -> WestChamber Driver即可。 


任何的意见、建议、BugReport等等都欢迎发到作者邮箱(elysion51@gmail.com）。 

转换状态：
WestChamberWindows目前有3种支持状态：
NONE（TCP全部放行），IPLOG（根据IPLOG筛选），ALL（全部启动ZHANG）。
可以运行bin/wcwcontroller.exe在用户态转换状态。

关于Issue Report:
如果出现蓝屏，可能的话请提供崩溃转储文件。
（在Windows\minidump文件夹下，系统重新启动后的“系统已从严重错误中恢复”窗口的“详细信息”中也有）。
如果在安装后没有任何响应，请尝试重启系统。

已知兼容性问题：
与Kaspersky的NDIS Filter不兼容。

其他相关信息：
链路上存在NAT的网络有较大几率不成功（NAT大多未完全实现TCP的RFC），可能的话请尝试直连。
westchamber.sys的源代码已上传到SVN。 
如果需要重新编译，请注意使用checked模式，以便输出调试信息。 
westchamber.dll(即passthru.dll，仅重命名）和BindView.exe的源代码可在Windows DDK中找到。 
此外，希望有更多熟悉Windows驱动开发，尤其是NDIS中间层驱动开发的人士，能够积极参与到开发中来。 