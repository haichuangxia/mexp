#pragma once
#include "Address.h"
#include <arpa/inet.h>
#include "Config.h"
#include "../comm/Buffer.h"
namespace net
{
    struct Message
    {
        Address m_addr;
        comm::Buffer m_buf;
        Message() = default;

        Message(const sockaddr_in &from, comm::Buffer &&buf) : m_addr(from.sin_addr.s_addr, from.sin_port), m_buf(buf.m_size)
        {
            memcpy(m_buf.m_buffer, buf.m_buffer, buf.m_size);
        };

        Message(const sockaddr_in &from, comm::Buffer &buf) : m_addr(from.sin_addr.s_addr, from.sin_port), m_buf(buf.m_size)
        {
            memcpy(m_buf.m_buffer, buf.m_buffer, buf.m_size);
        };
    };
}