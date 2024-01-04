
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
