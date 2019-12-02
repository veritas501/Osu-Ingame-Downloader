# Osu Ingame Downloader

一个使用Ingame overlay方式实现的Osu游戏内铺面下载器。

铺面镜像使用[Sayobot]( https://osu.sayobot.cn/)，感谢Sayobot提供如此优质的服务。

## 使用说明

本下载器的实现方式和steam中`shift+tab`的overlay以及OsuSync中的[IngameOverlay](https://github.com/OsuSync/IngameOverlay) 是一样的。

但由于需要注入DLL到Osu!中并修改Osu!的Render以及劫持某些API，如果用户使用后因本插件被Ban，作者不承担任何责任。

PS：作为参考，使用OsuSync的IngameOverlay不会被ban， https://osu.ppy.sh/community/forums/topics/697214?n=78

## DLL下载

 https://github.com/veritas501/Osu-Ingame-Downloader/releases 

## 食用方法

1. DLL的加载有三种方法，选择一种适合你的：
   1. 将`version.dll`复制到Osu!游戏文件夹下（不一定起效，游戏启动后按`Alt+M`测试是否成功）
   2. 将`xinput1_4.dll`复制到Osu!游戏文件夹下（不一定起效，游戏启动后按`Alt+M`测试是否成功）
   3. 利用注入器（还没写，请使用第三方注入器）将`IngameDL.dll`注入到游戏中
2. 游戏内按`Alt+M`可以唤出设置窗口，此时可以调整偏好选项或停用下载器，且可以自由移动`Status`窗口的位置
3. `Status`窗口平时是隐藏的，只有在有下载任务的时候显示，下载完成后会再次隐藏。
4. 插件会判断铺面是否已经存在，如果存在则不再下载且打开浏览器。

## 演示视频

![](Assets/demo.gif)



