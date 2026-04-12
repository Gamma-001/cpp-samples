#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <cstring>

#include "common.hpp"

#define PORT "8080"

int main() {
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* srv_addr;
    if (getaddrinfo(NULL, PORT, &hints, &srv_addr) != 0) {
        std::cout << "getaddrinfo() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }

    SOCKET srv_sock = socket(srv_addr->ai_family, srv_addr->ai_socktype, srv_addr->ai_protocol);
    if (srv_sock < 0) {
        std::cout << "socket() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }

    if (bind(srv_sock, srv_addr->ai_addr, srv_addr->ai_addrlen) != 0) {
        std::cout << "bind() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }
    freeaddrinfo(srv_addr);

    std::cout << "Listening for incoming data on 0.0.0.0:" << PORT << "...\n";

    sockaddr_storage client_addr;
    socklen_t client_len = sizeof(sockaddr_storage);
    MCSTR<1025> read;
    MCSTR_ZERO(read);
    int bytes_recv = recvfrom(srv_sock, read.data(), read.size() - 1, 0, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (bytes_recv < 0) {
        std::cout << "FAIL recvfrom() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }
    
    auto prstraddr_client = CPS::getnameinfo(reinterpret_cast<sockaddr*>(&client_addr), client_len);
    if (prstraddr_client.first.empty()) return 1;

    std::cout << "Incoming data from: " << prstraddr_client.first << ":" << prstraddr_client.second << '\n';
    std::cout << std::string(30, '=') << '\n' << std::string(read.data(), bytes_recv) << '\n' << std::string(30, '=') << '\n';

    close(srv_sock);

    return 0;
}
