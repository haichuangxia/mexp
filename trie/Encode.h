#pragma once
#include "../type.h"
namespace trie
{
    class encode
    {
    private:
        encode() = default;

    public:
        encode(const encode &) = delete;
        encode &operator=(const encode &) = delete;
        static encode &getInstance()
        {
            static encode coder;
            return coder;
        };
        // 将原始编码转换成十六进制编码
        static void hex_encode(const byte *buffer_in, int buffer_in_len, byte *buffer_out, int buffer_out_len);

        /**
         * @brief 比较两个十六进制编码是否相同，不同则返回不同的位置:-1表示全部匹配
         *
         * @param one 编码1指针
         * @param start_one 编码1起始位置
         * @param two 编码2指针
         * @param start_two 编码2起始位置
         * @param len 要比较的长度
         * @return int 1：两个十六进制编码值相同
         */
        static int compare(const byte *one, int start_one, const byte *two, int start_two, int len);
    };
}
