// 定义DHT需要使用到的一些参数
#pragma once
#include <chrono>
namespace dht
{
    // 参数K：K桶的大小
    extern std::size_t const ROUTING_TABLE_BUCKET_SIZE{20};

    // 并行查找量α:同时执行多少个并发查找
    extern std::size_t const CONCURRENT_FIND_PEER_REQUESTS_COUNT{3};

    // 设置超时时间
    extern std::chrono::milliseconds const PEER_LOOKUP_TIMEOUT{200};

    // ping的超时时间
    extern std::chrono::milliseconds const PING_WAIT_TIMEOUT{200};

    // 每次间隔多久再次重试建立连接
    extern std::chrono::milliseconds const CONNECTION_RETRY_INTERVAL{200};

    // 建立连接的最大重试次数
    extern std::size_t const MAX_CONNECTION_RETRY_TIMES{6};
}