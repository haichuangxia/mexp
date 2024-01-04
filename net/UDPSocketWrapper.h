#include <stdexcept>

#ifdef _WIN32

#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#endif

namespace net {
    class UDPSocket {
    public:
        int socketfd;

    public:
        UDPSocket();

        ~UDPSocket();

        UDPSocket(const UDPSocket &) = delete;

        UDPSocket &operator=(const UDPSocket &) = delete;

        UDPSocket(UDPSocket &&another) noexcept;

        UDPSocket &operator=(UDPSocket &&another) noexcept;

    private:
        void initSocket();
    };

    UDPSocket::UDPSocket() : socketfd(-1) {
        initSocket();
    }

    UDPSocket::~UDPSocket() {
        if (socketfd != -1) {
#ifdef _WIN32
            closesocket(socketfd);
#else
            close(socketfd);
#endif
        }
    }

    UDPSocket::UDPSocket(UDPSocket &&another) noexcept: socketfd(another.socketfd) {
        another.socketfd = -1;
    }

    UDPSocket &UDPSocket::operator=(UDPSocket &&another) noexcept {
        if (this != &another) {
            socketfd = another.socketfd;
            another.socketfd = -1;
        }
        return *this;
    }

    void UDPSocket::initSocket() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock");
        }
#endif

        if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            throw std::runtime_error("Failed to create UDP socket");
        }
    }
}
