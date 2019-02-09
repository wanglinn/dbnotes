该文件夹pgxl下的文件及子文件夹内容说明如下：

enable_debug_mvcc.patch
添加了GUC参数enable_debug_mvcc，对函数 HeapTupleSatisfiesMVCC 进行了修改，若enable_debug_mvcc=on则该函数总是返回true，
由于是标记删除，这样删除的tuple在vacuum 未清空时，可以查看。


autoinstall_xl.sh
安装pgxl集群工具，同安装pgxc的区别是需要在datanode节点上create 所有coordinator和datanode节点
