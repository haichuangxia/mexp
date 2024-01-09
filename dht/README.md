
# 项目结构
需要考虑的主要结构
- config：设置DHT需要用的参数
  - 如k桶的大小
  - 并发查询量
  - 初始种子节点
- RouteTable：使用MPT树实现路由表
  - find(NodeID):根据一个节点的ID查询该节点的地址
  - insert(NodeID,Pub,Address):插入节点，包括公钥，NodeID，地址
  - update(NodeID,Address): 更新节点的地址，因为ID根据公钥产生，所以公钥不变
  - remove(NodeID):考虑是否需要移除节点
- peer:DHT网络中节点的信息
  - Pub：保存节点的公钥，需要能够解析公钥
  - ID：定义ID的结构，根据公钥计算ID，计算两个ID之间的距离
  - Address：节点的地址，使用字符串格式存储？
- RPC：
  - ping(address):查看一个节点是否在线
  - find_node(address，nodeId):向一个远程节点查询节点
  - connect(address):与一个节点交换地址和公钥信息
- dht：
  - init():构建路由表
  - find_node(NodeID):根据ID，查询节点的地址，这一个远程过程调用需要使用异步操作


# 主要数据结构
## KBucket

主要用来在FIND_NODE过程调用中，保存收到的节点信息。主要需要有插入删除操作。因为都要在多线程下操作，因此需要考虑线程安全问题。
- 插入：在没有满之前直接插入，在满了之后，如果小于最大值则替换最大值。
- 删除：从队列中将元素删除掉

## RouteTable

使用MPT作为路由表，因为以后有可能同时插入多个新节点。因此需要在MPT的基础上增加一些线程安全机制。

## Peer
保存网络中每一个节点的信息，包括节点的ID，地址等信息。



# 并发设计
UdpTransport需要一个监听进程，监听接收到的UDP数据报并交给对应的线程处理

Kademlia所需进程：
- 其他节点请求处理线程：来自其他的ping或者find_node请求，需要用一个线程来处理
- 本地路由表线程：主要用来处理本地其他程序给出的请求，如果没有找到还需要调用UdpTransport来发送消息

启动顺序：
- 初始化UdpTransport
- 初始化Kademlia
- 启动主通信线程