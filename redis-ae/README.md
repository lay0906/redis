从redis中抽出的网络库-ae, 使用epoll的水平触发模式,IO为非阻塞模式</br>
1. 去掉了kqueue,select只保留了epoll </br>
2. 去掉了时间处理事件,只保留文件事件 </br>
3. 去掉了一些不影响流程的socket函数 </br>
