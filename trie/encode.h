#pragma once
#include "../type.h"
namespace trie{
    class encode {
    private:
        encode()=default;


    public:
        encode(const encode&)=delete;
        encode& operator=(const encode&)=delete;
        static encode& getInstance(){
            static encode coder;
            return coder;
        };
        // 将原始编码转换成十六进制编码
        static void hex_encode(const byte* buffer_in,int  buffer_in_len,byte * buffer_out,int buffer_out_len);
        // 比较两个十六进制编码是否相同，不同则返回不同的位置:-1表示全部匹配
        static int compare(const byte *one, int start_one, const byte *two, int start_two, int len);
    };
}

