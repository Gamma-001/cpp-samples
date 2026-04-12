#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <cstring>

#include "common.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Incorrect usage, expected udp_client <hostname> <port>\n";
        return 1;
    }

    MCSTR<100> serv_addr;
    MCSTR<100> serv_port;
    MCSTR_ZERO(serv_addr);
    MCSTR_ZERO(serv_port);

    strncpy(serv_addr.data(), argv[1], serv_addr.size());
    strncpy(serv_port.data(), argv[2], serv_port.size());

    std::cout << "STAT Connecting to UDP server: " << std::string(serv_addr.data()) << ':' << std::string(serv_port.data()) << "...\n"; 

    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* peer_addr = nullptr;
    if (getaddrinfo(serv_addr.data(), serv_port.data(), &hints, &peer_addr) != 0) {
        std::cerr << "FAIL getaddrinfo() failed, error << " << GETSOCKERROR() << '\n';
        return 1;
    }

    auto prstraddr = CPS::getnameinfo(peer_addr->ai_addr, peer_addr->ai_addrlen);
    if (prstraddr.first.empty()) return 1;
    std::cout << "STAT Connected to peer: " << prstraddr.first << ':' << prstraddr.second << '\n';

    SOCKET sock_peer = socket(peer_addr->ai_family, peer_addr->ai_socktype, peer_addr->ai_protocol);
    if (sock_peer < 0) {
        std::cerr << "FAIL socket() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }

    std::string message = "Hello World!";
    int bytes_sent = sendto(sock_peer, message.c_str(), message.size(), 0, peer_addr->ai_addr, peer_addr->ai_addrlen);
    if (bytes_sent < 0) {
        std::cerr << "FAIL sendto() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }
    std::cout << "STAT successfully sent " << bytes_sent << " bytes of " << message << '\n';

    freeaddrinfo(peer_addr);
    close(sock_peer);

    return 0;
}
