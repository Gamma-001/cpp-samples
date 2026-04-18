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
    cps::scoped_addrinfo peer_addr;
    if (getaddrinfo(serv_addr.data(), serv_port.data(), &hints, peer_addr.ref()) != 0) {
        std::cerr << "FAIL getaddrinfo() failed, error << " << GETSOCKERROR() << '\n';
        return 1;
    }

    auto prstraddr = cps::getnameinfo(peer_addr->ai_addr, peer_addr->ai_addrlen);
    if (prstraddr.first.empty()) return 1;
    std::cout << "STAT Connected to peer: " << prstraddr.first << ':' << prstraddr.second << '\n';

    cps::scoped_socket sock_peer;
    sock_peer = socket(peer_addr->ai_family, peer_addr->ai_socktype, peer_addr->ai_protocol);
    if (sock_peer.get() < 0) {
        std::cerr << "FAIL socket() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }

    fd_set fds_master;
    FD_ZERO(&fds_master);
    FD_SET(0, &fds_master);
    FD_SET(sock_peer.get(), &fds_master);
    while (true) {
        fd_set fds_reads = fds_master;
        int sret = select(sock_peer.get() + 1, &fds_reads, nullptr, nullptr, 0);
        if (sret == -1) {
            std::cerr << "FAIL select() failed, error: " << GETSOCKERROR() << '\n';
            break;
        }

        if (FD_ISSET(sock_peer.get(), &fds_reads)) {
            MCSTR<1025> buff;
            MCSTR_ZERO(buff);
            int bytes_recv = recv(sock_peer.get(), buff.data(), buff.size() - 1, 0);
            if (bytes_recv < 0) {
                std::cerr << "FAIL recv() failed, error: " << GETSOCKERROR() << '\n';
                break;
            }

            std::cout << "STAT read " << bytes_recv << " bytes.\n";
            std::cout << std::string(15, '=') << '\n' << std::string(buff.data(), bytes_recv) << '\n' << std::string(15, '=') << '\n';
        }
        else if (FD_ISSET(0, &fds_reads)) {
            MCSTR<1025> buff;
            MCSTR_ZERO(buff);
            if (fgets(buff.data(), buff.size() - 1, stdin) == nullptr) continue;
            
            int bytes_sent = sendto(sock_peer.get(), buff.data(), strlen(buff.data()), 0, peer_addr->ai_addr, peer_addr->ai_addrlen);
            if (bytes_sent < 0) {
                std::cerr << "FAIL sendto() failed, error: " << GETSOCKERROR() << '\n';
                break;
            }

            std::cout << "STAT sent " << bytes_sent << " bytes of " << std::string(buff.data(), buff.size()) << '\n';
        }
    }

    return 0;
}
