#pragma once
#include "../trie/trie.h"
#include "Peer.h"
namespace dht{
    class RouteTable{
    private:
        trie::trie<Contact> m_table;
    public:
        bool is_in_table(const Peer &peer);

        bool find(Peer &peer);

        bool insert(const Peer& peer);

        bool update(const Peer& peer);

        bool remove(Peer &peer);
    };
}