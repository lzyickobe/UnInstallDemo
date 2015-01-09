## Android应用监听自己是否被卸载，卸载后弹出反馈网页

### 在前人的基础上有这些解决方案
1. 监听卸载广播，只能监听到别人卸载。自己被卸载的时候，早就收不到广播了。
2. 监听log。这样听起来很靠谱，能稳定监听到，但是发送操作不靠谱。
3. 监听/data/data/<package name>。当Android卸载应用的时候，会先删除这里的文件。可以轮询监听，可以优化成unix文件监听方式，，这样只用等待文件监听服务的回调。

### 采用了第3种解决办法，并对其进行了优化：
#### 问题：
如果监听/data/data/<package name>这个目录，会有两个问题：

1. 如果版本更新的话，也会弹出反馈网页
2. 如果对这个目录下任何文件进行了删除操作，都会弹出反馈网页

#### 原因：
由于inotify_add_watch(fileDescriptor, path, IN_DELETE)这个函数会监听path目录下所有文件的删除操作导致。

#### 解决方法：
创建一个特殊的文件用于卸载监听，不要监听整个app目录。具体做法是：APP启动的时候，在/data/data/<package name>/files/下创建一个专用于卸载监听的文件，然后监听这个文件有没有被删除。

详细方案可参考我的博文：[Android监听自己是否被卸载](lzyblog.com)

参考自:

https://github.com/sevenler/Uninstall_Statics

http://www.cnblogs.com/zealotrouge/p/3157126.html

http://www.cnblogs.com/zealotrouge/p/3159772.html

http://www.cnblogs.com/zealotrouge/p/3182617.html