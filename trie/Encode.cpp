#include "Encode.h"
#include <stdexcept>
namespace trie
{
    void trie::encode::hex_encode(const byte *buffer_in, int buffer_in_len, byte *buffer_out, int buffer_out_len)
    {
        if (buffer_in == nullptr || buffer_out == nullptr)
            throw std::invalid_argument("Pointer can`t be nullptr");
        if (buffer_out_len < buffer_in_len << 1)
            throw std::invalid_argument("The buffer_out size is less than twice the size of buffer_in");
        for (int i = 0; i < buffer_in_len; ++i)
        {
            *(buffer_out + 2 * i) = *(buffer_in + i) >> 4;
            *(buffer_out + 2 * i + 1) = *(buffer_in + i) & 0x0f;
        }
    }
    int trie::encode::compare(const byte *one, int start_one, const byte *two, int start_two, int len)
    {
        if (one == nullptr && two == nullptr)
            return -1;
        if (one == nullptr || two == nullptr)
            throw std::invalid_argument("The pointer can`t be nullptr");
        for (int i = 0; i < len; ++i)
        {
            if (*(one + start_one + i) != *(two + start_two + i))
                return i;
        }
        return -1;
    }
}