#pragma once
#include "../type.h"
namespace comm
{
    struct ISerilize
    {
        virtual int size() const = 0;
        virtual int serialize(byte *buf, int capacity) const = 0;
        virtual int deserialize(const byte *buf, int capacity) = 0;
    };
}