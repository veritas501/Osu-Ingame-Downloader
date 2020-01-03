# Osu Ingame Downloader

一个使用Ingame overlay方式实现的Osu游戏内铺面下载器。

铺面镜像使用[Sayobot]( https://osu.sayobot.cn/)，感谢Sayobot提供如此优质的服务。

## 使用说明

本下载器的实现方式和steam中`shift+tab`的overlay以及OsuSync中的[IngameOverlay](https://github.com/OsuSync/IngameOverlay) 是一样的。

只要不涉及游戏数据作弊，Osu!还是比较宽容的，因此虽然下载器需要注入DLL到Osu!中并修改Osu!的Render以及劫持某些API，但正常使用是不会被Ban的。如果用户使用后因本插件被Ban，作者不承担任何责任。

PS：作为参考，使用OsuSync的IngameOverlay不会被Ban， https://osu.ppy.sh/community/forums/topics/697214?n=78

## DLL下载

 https://github.com/veritas501/Osu-Ingame-Downloader/releases 

## 食用方法

1. 从[下载页](https://github.com/veritas501/Osu-Ingame-Downloader/releases )下载`IngameDL.dll`和`loader.zip`。
2. 将`IngameDL.dll`放入Osu!的游戏目录下，即和`osu!.exe`在同一目录。
3. 从`loader.zip`中选择一个加载器放入Osu!的游戏目录下。启动Osu!，在游戏内按`Alt+M`测试下载器是否启动成功。
4. 游戏内按`Alt+M`可以唤出设置窗口，此时可以调整偏好选项或停用下载器，且可以自由移动`Status`窗口的位置
5. `Status`窗口平时是隐藏的，只有在有下载任务的时候显示，下载完成后会再次隐藏。
6. 插件会判断铺面是否已经存在，如果存在则不再下载且打开浏览器。

## 演示视频

![](Assets/demo.gif)



