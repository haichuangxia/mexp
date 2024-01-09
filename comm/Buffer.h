#pragma once
#include "../type.h"
namespace comm
{
    struct Buffer
    {
        byte *m_buffer;
        int m_capacity;
        int m_size;
        Buffer() : m_buffer(nullptr), m_capacity(0), m_size(0){};
        Buffer(uint32_t capacity) : m_size(0)
        {
            m_buffer = new byte[capacity];
            m_capacity = capacity;
        };
        Buffer(const Buffer &other) = delete;
        Buffer &operator=(const Buffer &) = delete;
        Buffer(Buffer &&other) : m_buffer(other.m_buffer), m_capacity(other.m_capacity), m_size(other.m_size)
        {
            other.m_buffer = nullptr;
            other.m_capacity = 0;
            other.m_size = 0;
        }
        Buffer &operator=(Buffer &&other)
        {
            m_buffer = other.m_buffer;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            other.m_buffer = nullptr;
            other.m_capacity = 0;
            other.m_size;
        };
        ~Buffer()
        {
            delete[] m_buffer;
        };
    };
}
