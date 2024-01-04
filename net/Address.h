#pragma once
#include <cstdint>
#include <string>
#include <iostream>
namespace net {

    struct Address {
        uint32_t ip;
        uint16_t port;
        std::string toString() const;

        friend std::ostream &operator <<(std::ostream &os,const Address& addr) {
          return os<<addr.toString();
        };
    };

}
