// --- system libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// --- standard libraries
#include <iostream>
#include <cstring>

#include "common.hpp"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: tcp_client <hostname> <port>\n";
        return 1;
    }

    cps::scoped_socket sock_peer;
    {
        addrinfo hints;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_socktype = SOCK_STREAM;

        cps::scoped_addrinfo peer_address;
        if (getaddrinfo(argv[1], argv[2], &hints, peer_address.ref()) != 0) {
            std::cerr << "FAIL getaddrinfo() failed with error: " << GETSOCKERROR();
            return 1;
        }

        auto prstraddr_peer = cps::getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, NI_NUMERICHOST);
        if (prstraddr_peer.first.empty()) return 1;

        sock_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
        if (sock_peer.invalid()) {
            std::cerr << "FAIL socket() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }

        if (connect(sock_peer.get(), peer_address->ai_addr, peer_address->ai_addrlen) != 0) {
            std::cerr << "FAIL connect() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }
        std::cout << "STAT Connected to remote address: " << prstraddr_peer.first << " " << prstraddr_peer.second << '\n';
        std::cout << "STAT Reading data in line mode.\n";
    }

    while(true) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(sock_peer.get(), &reads);
        FD_SET(0, &reads);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        
        if (select(sock_peer.get() + 1, &reads, 0, 0, &timeout) < 0) {
            std::cerr << "FAIL select() failed, error: " << GETSOCKERROR() << '\n';
            break;
        }

        // Check for any TCP data from remote adddress
        if (FD_ISSET(sock_peer.get(), &reads)) {
            MCSTR<4096> read;
            int bytes_recv = recv(sock_peer.get(), read.data(), read.size(), 0);
            if (bytes_recv < 1) {
                std::cout << "STAT Connection closed by peer.\n";
                break;
            }
            std::cout << "STAT received " << bytes_recv << " bytes: " << std::string(read.data(), bytes_recv) << '\n';
        }
        // Check for any data from stdin
        else {
            MCSTR<4096> read;
            MCSTR_ZERO(read);
            if (!fgets(read.data(), read.size(), stdin)) break;
            std::cout << "Sending: " << std::string(read.data()) << '\n';
            int bytes_sent = send(sock_peer.get(), read.data(), strlen(read.data()), 0);
            std::cout << "Sent " << bytes_sent << " bytes\n";
        }
    }

    return 0;
}
