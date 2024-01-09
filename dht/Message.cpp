#include "Message.h"
#include "Kademlia.h"
namespace dht
{

    int FindNodeResponseMsg::size() const
    {
        int knearSize = 0;
        for (const auto &peer : knear)
        {
            knearSize += peer.size();
        }

        return Message::size() + sizeof(ksize) + knearSize;
    };

    int FindNodeResponseMsg::serialize(byte *buf, int capacity) const
    {
        if (capacity < size())
            return -1;
        int offset = Message::serialize(buf, capacity);

        uint8_t knearSize = static_cast<uint8_t>(knear.size());
        memcpy(buf + offset, &knearSize, sizeof(knearSize));
        offset += sizeof(knearSize);

        for (const auto &peer : knear)
            offset += peer.serialize(buf + offset, capacity - offset);

        return offset;
    }

    int FindNodeResponseMsg::deserialize(const byte *buf, int capacity)
    {
        if (capacity < 1)
            return -1;
        int offset = Message::deserialize(buf, capacity);
        uint8_t knearSize;
        Peer peer;
        if (capacity < offset + sizeof(knearSize) + knearSize * peer.size())
            return -1;
        memcpy(&knearSize, buf + offset, sizeof(knearSize));
        offset += sizeof(knearSize);

        for (uint8_t i = 0; i < knearSize; ++i)
        {
            int peerSize = peer.deserialize(buf + offset, capacity - offset);
            if (peerSize == -1)
                return -1;
            knear.push_back(peer);
            offset += peerSize;
        }

        return offset;
    }

    int Kademlia::ping(const Peer &remote)
    {
        net::Address addr(remote.m_contact.m_address.m_ip, remote.m_contact.m_address.m_port);
        uint64_t connId = transport.connect(addr);
    }
}