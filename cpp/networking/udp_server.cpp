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

    cps::scoped_socket srv_sock;
    {
        cps::scoped_addrinfo srv_addr;
        if (getaddrinfo(NULL, PORT, &hints, srv_addr.ref()) != 0) {
            std::cerr << "FAIL getaddrinfo() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }

        srv_sock = socket(srv_addr->ai_family, srv_addr->ai_socktype, srv_addr->ai_protocol);
        if (srv_sock.get() < 0) {
            std::cerr << "FAIL socket() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }

        if (bind(srv_sock.get(), srv_addr->ai_addr, srv_addr->ai_addrlen) != 0) {
            std::cerr << "FAIL bind() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }
    }
    std::cout << "STAT Listening for incoming data on 0.0.0.0:" << PORT << "...\n";
    
    fd_set fds_master;
    FD_ZERO(&fds_master);
    FD_SET(srv_sock.get(), &fds_master);
    while (true) {
        fd_set fds_reads = fds_master;
        int sret = select(srv_sock.get() + 1, &fds_reads, 0, 0, 0);        
        if (sret == 0) continue;
        else if (sret == -1) {
            std::cerr << "FAIL select() failed, error: " << GETSOCKERROR() << '\n';
            break;
        }

        if (FD_ISSET(srv_sock.get(), &fds_reads)) {
            sockaddr_storage client_addr;
            socklen_t client_len = sizeof(sockaddr_storage);
            MCSTR<1025> buff;
            MCSTR_ZERO(buff);
            int bytes_recv = recvfrom(srv_sock.get(), buff.data(), buff.size() - 1, 0, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
            if (bytes_recv < 0) {
                std::cout << "FAIL recvfrom() failed, error: " << GETSOCKERROR() << '\n';
                return 1;
            }
            
            auto prstraddr_client = cps::getnameinfo(reinterpret_cast<sockaddr*>(&client_addr), client_len);
            if (prstraddr_client.first.empty()) return 1;

            std::string body = std::string(buff.data(), bytes_recv);
            std::cout << "STAT Incoming data from: " << prstraddr_client.first << ":" << prstraddr_client.second << '\n';
            std::cout << std::string(30, '=') << '\n' << body << '\n' << std::string(30, '=') << '\n';
        
            std::string response = body;
            for (auto &x: response) x = std::toupper(static_cast<unsigned char>(x));
            int bytes_sent = sendto(srv_sock.get(), response.data(), response.size(), 0, reinterpret_cast<const sockaddr*>(&client_addr), client_len);
            if (bytes_sent < 0) {
                std::cerr << "FAIL sendto failed, error: " << GETSOCKERROR() << '\n';
                break;
            }
            else std::cout << "STAT sent " << bytes_sent << " bytes of " << response << '\n';
        } 
    }

    return 0;
}
