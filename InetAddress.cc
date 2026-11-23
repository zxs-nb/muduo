#include "InetAddress.h"
#include <cstring>
// int inet_pton(int af,
//               const char* src,
//               void* dst);

InetAddress::InetAddress(uint16_t port, const std::string& ip) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}

// const char* inet_ntop(
//     int af,
//     const void* src,
//     char* dst,
//     socklen_t size
// );

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

// int snprintf(
//     char* str,
//     size_t size,
//     const char* format,
//     ...
// );
#include <iostream>
std::string InetAddress::toIpPort() const {
    char buf[128] = {0};
    uint16_t port = ntohs(addr_.sin_port);
    if (inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)) == nullptr) {
        std::cout << "somthing wrong";
        return "";
    }
    size_t len = strlen(buf);
    snprintf(buf + len, sizeof(buf) - len, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const { return ntohs(addr_.sin_port); }


int main()
{
    InetAddress addr(8080);
    std::cout << "目标地址：" << addr.toIpPort() << std::endl;
    std::cout << "目标ip：" << addr.toIp() << std::endl;
    std::cout << "目标端口：" << addr.toPort() << std::endl;
    return 0;
}