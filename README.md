## Android应用监听自己是否被卸载，卸载后弹出反馈网页

### 在前人的基础上有这些解决方案
1. 监听卸载广播，只能监听到别人卸载。自己被卸载的时候，早就收不到广播了。
2. 监听log。这样听起来很靠谱，能稳定监听到，但是发送操作不靠谱。
3. 监听/data/data/<package name>。当Android卸载应用的时候，会先删除这里的文件。可以轮询监听，可以优化成unix文件监听方式，，这样只用等待文件监听服务的回调。

### 采用了第3种解决办法，并对其进行了优化：
#### 问题：
监听/data/data/<package name>这个目录，还存在以下几个问题：

1. 清除数据、插拔USB线、覆盖安装等操作引起程序误判卸载。
2. 重复监听的问题。
3. 用户将已在Internal SD卡安装好的应用移动到external SD卡，导致监听不正常。

#### 原因：
1. 由于inotify_add_watch(fileDescriptor, path, IN_DELETE)这个函数会监听path目录下所有文件的删除操作导致。
2. 重复调用JNI的init方法
3. 暂时未修复

#### 解决方法：
1. 监听不应该针对整个文件夹，而是某个文件。
2. 重复监听的问题，都可以通过加文件锁来防止

详细方案可参考我的博文：[Android监听自己是否被卸载](http://lzyblog.com/2015/01/09/Android%E5%BA%94%E7%94%A8%E7%9B%91%E5%90%AC%E8%87%AA%E5%B7%B1%E6%98%AF%E5%90%A6%E8%A2%AB%E5%8D%B8%E8%BD%BD%EF%BC%8C%E5%81%9A%E5%8F%8D%E9%A6%88%E7%BB%9F%E8%AE%A1/)

参考自:

https://github.com/sevenler/Uninstall_Statics

http://www.cnblogs.com/zealotrouge/p/3157126.html

http://www.cnblogs.com/zealotrouge/p/3159772.html

http://www.cnblogs.com/zealotrouge/p/3182617.html