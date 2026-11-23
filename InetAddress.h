#pragma once
#include <netinet/in.h> //网络地址结构体
#include <arpa/inet.h>  //网络处理函数
#include <string>

//封装socket地址类
class InetAddress 
{
public:
    explicit InetAddress(uint16_t port, const std::string& ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr) 
    : addr_(addr)
    {}

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in* getSockAddr() const { return &addr_; }

private:
    sockaddr_in addr_{};
};