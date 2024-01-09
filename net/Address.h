#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include "../comm/ISerilize.h"
namespace net
{

  struct Address : comm::ISerilize
  {
    uint32_t m_ip;
    uint16_t m_port;

    int size() const override;

    bool isZero() const;

    int serialize(byte *buf, int buf_size) const override;

    int deserialize(const byte *buf, int capacity) override;

    std::string toString() const;

    Address() = default;

    Address(sockaddr_in addr) : m_ip(addr.sin_addr.s_addr), m_port(addr.sin_port){};

    Address(uint32_t ip, uint16_t port) : m_ip(ip), m_port(port){};

    operator uint64_t() const
    {
      return m_ip << 16 + m_port;
    };

    friend std::ostream &operator<<(std::ostream &os, const Address &addr)
    {
      return os << addr.toString();
    };
  };

  bool Address::isZero() const
  {
    return !m_ip && !m_port;
  };

  int Address::size() const
  {
    return sizeof(m_ip) + sizeof(m_port);
  }

  int Address::serialize(byte *buf, int buf_size) const
  {
    if (buf_size < size())
      return 0;
    memcpy(buf, &m_ip, sizeof(m_ip));
    memcpy(buf + sizeof(m_ip), &m_port, sizeof(m_port));
    return size();
  };

  int Address::deserialize(const byte *buf, int capacity)
  {
    if (capacity < size())
      return 0;

    memcpy(&m_ip, buf, sizeof(m_ip));
    memcpy(&m_port, buf + sizeof(m_ip), sizeof(m_port));

    return size();
  }

  std::string Address::toString() const
  {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = m_ip;
    addr.sin_port = m_port;

    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), buffer, INET_ADDRSTRLEN);

    return std::string(buffer) + ":" + std::to_string(ntohs(addr.sin_port));
  }

}
