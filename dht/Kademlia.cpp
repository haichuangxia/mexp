#include "Kademlia.h"
#include "Message.h"
#include <chrono>
#include "Config.h"
#include "../net/UDPSocketWrapper.h"
namespace dht
{
    int KBucket::insert(const Peer &other)
    {
        NodeID distance = other.m_id.countDistance(base);
        std::lock_guard<std::mutex> peerMapMtxLock(peerMapMtx);
        // 已存在则直接返回
        if (m_bucket.find(distance) != m_bucket.end())
            return -1;
        // 未满则直接插入
        if (size < capacity)
        {
            m_bucket.emplace(distance, other);
            ++size;
            return 1;
        }
        // 已满且更小则替换
        else if (distance < m_bucket.rbegin()->first)
        {
            m_bucket.erase(distance);
            m_bucket.emplace(distance, other);
            return 1;
        }
        else
            return 0;
    }

    int Kademlia::ping(const Peer &peer)
    {
        int online;
        uint64_t connId;
        std::unique_lock<std::mutex> connectionsLock(transport.connMutex);
        if ((connId = transport.connect(peer.m_contact.m_address)) == 0)
            return -1; // 表示连接已经被占用，可以过一段时间重试

        connectionsLock.unlock();

        PingMsg msg(peer.m_id);
        comm::Buffer buf(2 * msg.size());
        buf.m_size = msg.deserialize(buf.m_buffer, buf.m_capacity);
        transport.sendto(peer.m_contact.m_address, buf);

        std::unique_lock<std::mutex> lock(transport.connections[connId].mtx);
        bool result = transport.connections[connId].cv.wait_for(lock, PING_WAIT_TIMEOUT, [&]
                                                                { return transport.connections[connId].buf.m_size > 0; });
        if (result == true)
        {
            PongMsg pong;
            pong.deserialize(transport.connections[connId].buf.m_buffer, transport.connections[connId].buf.m_size);
            online = pong.local == peer.m_id ? 1 : 0;
        }
        else
            online = 0;
        lock.unlock();
        transport.close(connId); // 只有一个线程会关闭连接，因此不用申请锁？
        return online;
    };

    int Kademlia::onPing(net::Message &msg)
    {
        // 先序列化对方发出的ping消息
        PingMsg pingmsg;
        if (pingmsg.deserialize(msg.m_buf.m_buffer, msg.m_buf.m_size))
        {
            std::cerr << "Cant`t deserialize PingMsg form buffer" << std::endl;
            return -1;
        }
        // 生成响应的pong消息
        PongMsg pongmsg(pingmsg.remote);
        comm::Buffer buf(pongmsg.size());
        if ((buf.m_size = pongmsg.serialize(buf.m_buffer, buf.m_capacity)) < 1)
        {
            std::cerr << "Can`t serialize pong message to buffer" << std::endl;
            return -2;
        }
        // 调用socket发送消息
        int hasSend = transport.sendto(msg.m_addr, buf);
        if (hasSend < 1)
        {
            std::cerr << "Failed to call function UdpTransport::sendto to send pong message" << std::endl;
            return hasSend;
        }
        return hasSend;
    };

    int Kademlia::find_node(Peer &peer, const NodeID &key, KBucket &bucket)
    {
        int hasNewNearestNode = false;
        if (peer.m_contact.m_address == 0)
            return false;

        uint64_t connId = 0, hasRetried = 1;
        while (connId == 0 && hasRetried < MAX_CONNECTION_RETRY_TIMES)
        {
            std::unique_lock<std::mutex> connMapLock(transport.connMutex);
            if (connId = transport.connect(peer.m_contact.m_address) == 0)
            {
                connMapLock.unlock();
                std::cout << "Connection already exists. Wait for " << hasRetried * CONNECTION_RETRY_INTERVAL.count() << "ms" << std::endl;
                std::this_thread::sleep_for(hasRetried++ * CONNECTION_RETRY_INTERVAL);
            }
            else
                connMapLock.unlock();
        }
        if (connId == 0)
        {
            std::cout << "Faile to create connection for it already exists and will still work" << std::endl;
            return false;
        }

        FindNodeRequestMsg requestMsg(key);
        comm::Buffer buf(requestMsg.size());

        int hasSend = transport.sendto(peer.m_contact.m_address, buf);
        if (hasSend < requestMsg.size())
        {
            std::cout << "Failed to send find_node request to address:" << peer.m_contact.m_address << std::endl;
            transport.close(connId);
            return -1; // 表示异常
        }

        // 开始等待
        std::unique_lock<std::mutex> connLock(transport.connections[connId].mtx);
        auto status = transport.connections[connId].cv.wait_for(connLock, PEER_LOOKUP_TIMEOUT);
        if (status == std::cv_status::timeout)
        {
            connLock.unlock();
            std::cout << "Request at " << peer.m_contact.m_address << " timeout" << std::endl;
            transport.close(connId);
            return -2; // 对方规定时间内没有返回消息，TODO:考虑是否有必要将其从路由表中删除
        }

        FindNodeResponseMsg response;
        if (response.deserialize(transport.connections[connId].buf.m_buffer, transport.connections[connId].buf.m_size) < 1)
        {
            std::cout << "Failed to deserialize FindNodeResponseMsg object" << std::endl;
            connLock.unlock();
            transport.close(connId);
            return -3;
        }
        // 更新k桶
        std::lock_guard<std::mutex> rtLock(routeTableLock);
        for (auto &nearPeer : response.knear)
            hasNewNearestNode = bucket.insert(nearPeer) ? 1 : hasNewNearestNode;

        connLock.unlock();
        transport.close(connId);

        return hasNewNearestNode; // 为true表示更新了K通，false默认为0表示k桶没有更新
    }

    int Kademlia::findThead(Peer &peer, KBucket &bucket)
    {
        do
        {
            std::unique_lock<std::mutex> lock(bucket.peerMapMtx);

            // 等待有数据，如果已经有数据则直接跳过此步骤
            bucket.peerMaxCv.wait(lock, [&]
                                  { return !bucket.m_bucket.empty(); });

            // 有数据，则工作线程加一
            ++bucket.isWorking;

            auto item = bucket.m_bucket.begin();
            Peer askfor = item->second;
            // 已经找到了目标节点，终止查找过程
            if (item->first.isZero())
            {
                peer = item->second;
                bucket.shouldTerminate = true;
                return 1;
            }
            bucket.remove(item);
            lock.unlock();

            // 通过远程调用更新节点，这是一个耗时长的工作，因此需要释放锁让其他线程操作
            find_node(askfor, peer.m_id, bucket);

            // 工作结束，工作线程减1
            lock.lock();
            --bucket.isWorking;
            lock.unlock();
        } while (bucket.isWorking != 0 || !bucket.shouldTerminate);
        return 0;
    };

    int Kademlia::find(Peer &peer)
    {
        // 本地路由表查找
        if (routeTable.find(peer))
            return 1;

        // 初始化K桶
        KBucket bucket(peer.m_id);
        int num = routeTable.findKnearestPeer(ROUTING_TABLE_BUCKET_SIZE, bucket);

        // 非空则循环从队列中取数据
        std::thread threads[CONCURRENT_FIND_PEER_REQUESTS_COUNT];
        // Launch worker threads
        for (int i = 0; i < CONCURRENT_FIND_PEER_REQUESTS_COUNT; ++i)
        {
            threads[i] = std::thread(findThead, peer, bucket);
        }

        // Wait for threads to finish
        for (int i = 0; i < CONCURRENT_FIND_PEER_REQUESTS_COUNT; ++i)
        {
            threads[i].join();
        }

        // 检查是否查询到
        return peer.m_contact.m_address == 0 ? 0 : 1; // 0:查找失败，1：查找成功
    };

    void Kademlia::onFindNodeReqeust(net::Message &msg)
    {
        // 反序列化消息
        FindNodeRequestMsg request;
        int result = request.deserialize(msg.m_buf.m_buffer, msg.m_buf.m_capacity);
        if (result < 0)
        {
            std::cerr << "Failed to deserialize FindNode Request" << std::endl;
            return;
        }

        // TODO:如果路由表中没有该节点则插入

        // 生成响应的消息
        FindNodeResponseMsg response;
        response.ksize = routeTable.findKnearestPeer(request.k, request.key, response.knear);

        // 消息序列化
        comm::Buffer buf(response.size());
        if ((result = response.serialize(buf.m_buffer, buf.m_capacity)) < 1)
        {
            std::cerr << "Failed to serialize find node response message" << std::endl;
            return;
        }

        // 发送消息
        if ((result = transport.sendto(msg.m_addr, buf)) < 0)
        {
            std::cerr << "Failed to send FindNode response to " << msg.m_addr << std::endl;
            return;
        }
        return;
    };

    void Kademlia::onReceiveRequest()
    {
        std::unique_lock<std::mutex> recvBufMtxLock(transport.recvBufMtx);
        transport.receiveRequestCv.wait(recvBufMtxLock);
        switch (*(transport.msg.m_buf.m_buffer))
        {
        case (int)MessageType::KPing:
        {
            onPing(transport.msg);
            break;
        }
        case (int)MessageType::KFindNodeRequest:
        {
            onFindNodeReqeust(transport.msg);
            break;
        }

        default:
        {
            // 应用程序发送来的消息
            break;
        }
        }
    };

    void Kademlia::init()
    {
        std::thread requestProcesser(onReceiveRequest);
        requestProcesser.detach();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        // 执行一次自我查找
        find(self);
    };
}