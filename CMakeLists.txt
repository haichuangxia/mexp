cmake_minimum_required(VERSION 3.27)
project(mexp)

set(CMAKE_CXX_STANDARD 17)

add_executable(mexp main.cpp
        dht/Config.h
        dht/Peer.h
        type.h
        dht/Peer.cpp
        dht/RouteTable.h
        trie/trie.h
        trie/trie.cpp
        dht/Kademlia.h
        net/UDPSocketWrapper.h
        dht/Message.cpp
        dht/Message.h
        net/Address.h
)
