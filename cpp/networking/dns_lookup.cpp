#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <iostream>
#include <string>
#include <vector>

#include "common.hpp"

std::vector<std::string> lookup(const std::string& host, bool reverse = false);

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "incorrect usage, expected: dns_lookup [option] <hostname>\n";
        return 1;
    }

    bool dir = false;
    if (argc == 3 && std::string(argv[1]) == "-r") dir = true;

    auto fw_lookup = lookup(argv[1], dir);
    for (auto&x: fw_lookup) {
        std::cout << '\''<< x << "\'\n";
        for (auto &y: lookup(x, !dir)) {
            std::cout << "\t | " << y << '\n';
        }
        std::cout << '\n';
    }

    return 0;
}

std::vector<std::string> lookup(const std::string& host, bool reverse) {
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    cps::scoped_addrinfo peer_addr;
    int gai_ret = getaddrinfo(host.c_str(), 0, &hints, peer_addr.ref());
    if (gai_ret != 0) {
        std::cerr << "FAIL getaddrinfo() failed, error: " << gai_strerror(gai_ret) << '\n';
        return {};
    }

    std::vector<std::string> res;
    for (addrinfo* addr = peer_addr.get(); addr != nullptr; addr = addr->ai_next) {
        MCSTR<101> buff_host;
        MCSTR_ZERO(buff_host);
        int flags = reverse ? NI_NAMEREQD : NI_NUMERICHOST;
        int gni_ret = getnameinfo(
            addr->ai_addr, addr->ai_addrlen, 
            buff_host.data(), buff_host.size(), 
            nullptr, 0, 
            flags
        );
        if (gni_ret != 0) {
            std::cerr << "FAIL getnameinfo() failed, error: " << gai_strerror(gni_ret) << '\n';
        } else {
            res.emplace_back(buff_host.data());
        }
    }

    return res;
}
