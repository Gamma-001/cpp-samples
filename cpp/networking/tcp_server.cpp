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
#include <cctype>

#include "common.hpp"

#define PORT "8080"

// --- main

int main() {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* serv_addr = nullptr;
    if (getaddrinfo(0, PORT, &hints, &serv_addr) != 0) {
        std::cerr << "FAIL getaddrinfo() failed, error : " << GETSOCKERROR() << '\n';
        return 1;
    }

    SOCKET sock_serv = socket(serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol);
    if (sock_serv < 0) {
        std::cerr << "FAIL socket() failed, error : " << GETSOCKERROR() << '\n';
        return 1;
    }

    if (bind(sock_serv, serv_addr->ai_addr, serv_addr->ai_addrlen) != 0) {
        std::cerr << "FAIL bind() failed, error : " << GETSOCKERROR() << '\n';
        return 1;
    }
    freeaddrinfo(serv_addr);

    if (listen(sock_serv, 10) < 0) {
        std::cerr << "FAIL listen() failed, error : " << GETSOCKERROR() << '\n';
        return 1;
    }
    std::cout << "STAT Listening on 0.0.0.0:" << PORT << "...\n";

    fd_set master;
    FD_ZERO(&master);
    FD_SET(sock_serv, &master);
    SOCKET max_socket = sock_serv;
    while (true) {
        fd_set reads;
        reads = master;
        if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
            std::cerr << "FAIL select() failed, error : " << GETSOCKERROR() << '\n';
            return 1;
        }

        for (SOCKET sock = 1; sock <= max_socket; sock++) {
            if (FD_ISSET(sock, &reads)) {
                // New incoming connect, accept it
                if (sock == sock_serv) {
                    sockaddr_storage client_addr;
                    socklen_t client_addrlen = sizeof(client_addr);
                    SOCKET sock_client = accept(sock_serv, reinterpret_cast<sockaddr*>(&client_addr), &client_addrlen);
                    if (sock_client < 0) {
                        std::cerr << "FAIL accept() failed, error: " << GETSOCKERROR() << '\n';
                        return 1;
                    }

                    FD_SET(sock_client, &master);
                    max_socket = std::max(max_socket, sock_client);

                    auto prstraddr_client = CPS::getnameinfo(reinterpret_cast<sockaddr*>(&client_addr), client_addrlen, NI_NUMERICHOST);
                    if (prstraddr_client.first.empty()) return 1;
                    std::cout << "STAT Established connection with " << prstraddr_client.first << '\n';
                }
                // client sockets are available with data to be read
                else {
                    MCSTR<1024> buff;
                    MCSTR_ZERO(buff);
                    int bytes_recv = recv(sock, buff.data(), buff.size(), 0);
                    if (bytes_recv < 1) {
                        std::cout << "STAT Connection closed\n";
                        FD_CLR(sock, &master);
                        close(sock);
                        continue;
                    }

                    for (int i = 0; i < bytes_recv; i++) buff[i] = std::toupper(buff[i]);
                    send(sock, buff.data(), bytes_recv, 0);
                }
            }
        }
    }
    
    close(sock_serv);

    return 1;

}
