# 服务架构介绍

## 特点

* 通过mq来解决服务器之间的通讯问题
* 独立出redis、mysql 代理服务
* 使用协程等待消息。包括mq、redis、mysql等网络消息等待，都是基于异步回调的协程。底层回调发生后唤醒对应等待的协程。
* 服务唯一标识自动生成。组成部分为 type_ip_pwd。
* 使用zookeeper做服务发现。

## todo

##### 对外的接口服务

* 接入httpsvr处理http请求
* 接入grpc提供rpc服务
* 接入mysql

