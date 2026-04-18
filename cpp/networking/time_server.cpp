#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>
#include <array>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "common.hpp"

constexpr const char* PORT = "8080";

std::string current_time();
std::string get_ipaddr(sockaddr* addr, socklen_t addrlen);

void send_response(weak_socket sock, const std::string& response, int flags = 0);
void run_loop(weak_socket sock);

int main() {
    cps::scoped_socket sock_server;
    {
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        // Poppulate addrinfo list with address matching requested hints
        cps::scoped_addrinfo bind_address;
        if (getaddrinfo(0, PORT, &hints, bind_address.ref()) == -1) {
            std::cerr << "FAIL getaddrinfo() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }

        // create a socket to listen for incoming connections
        sock_server = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
        if (sock_server.invalid()) {
            std::cerr << "FAIL socket() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }

        // unset IPV6_V6ONLY to support dual stack
        int option = 0;
        if (setsockopt(sock_server.get(), IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<void*>(&option), sizeof(option))) {  
            std::cerr << "FAIL setsockopt() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }

        // Associate the socket with the host and port
        if (bind(sock_server.get (), bind_address->ai_addr, bind_address->ai_addrlen) != 0) {
            std::cerr << "FAIL bind() failed, error: " << GETSOCKERROR() << '\n';
            return 1;
        }
    }

    // Listen for new connections
    if (listen(sock_server.get(), 10) < 0) {
        std::cerr << "FAIL listen() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }
    std::cout << "STAT Listening on port:" << PORT << " (all interfaces) ...\n";

    // Accept a client connection and create a new associated client socket
    sockaddr_storage c_addr;
    socklen_t c_addr_len = sizeof(c_addr);
    cps::scoped_socket sock_client;
    sock_client = accept(sock_server.get(), reinterpret_cast<sockaddr*>(&c_addr), &c_addr_len);
    if (sock_client.invalid()) {
        std::cerr << "FAIL accept() failed, error: " << GETSOCKERROR() << '\n';
        return 1;
    }

    std::string ipaddr = get_ipaddr(reinterpret_cast<sockaddr*>(&c_addr), c_addr_len);
    if (ipaddr.empty()) 
        return -1;
    else 
        std::cout << "STAT Client connected with address: " << ipaddr << '\n';
    run_loop(sock_client.get());

    std::cout << "STAT Closing connection...\n";

    return 0;
}

std::string get_ipaddr(sockaddr* addr, socklen_t addrlen) {
    std::array<char, 100> ipaddr;
    if (getnameinfo(addr, addrlen, ipaddr.data(), ipaddr.size(), nullptr, 0, NI_NUMERICHOST) != 0) {
        std::cerr << "FAIL getnameinfo() failed, error: " << GETSOCKERROR() << '\n';
        return "";
    }

    return std::string(ipaddr.data());
}

std::string current_time() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    
    return std::string(std::ctime(&t_c));
}

void send_response(weak_socket sock, const std::string& response, int flags) {
    int bytes_sent = send(sock, response.c_str(), response.size(), flags);
    std::cout << "STAT Sent " << bytes_sent << " of " << response.size() << " bytes.\n";
}

void run_loop(weak_socket sock) {
    std::array<char, 1025> buffer;
    while (true) {
        int recv_bytes = recv(sock, buffer.data(), buffer.size() - 1, 0);
        if (recv_bytes == 0) {
            std::cerr << "STAT Connection closed for socket: " << sock << '\n';
            break;
        }
        else if (recv_bytes < 0) {
            std::cerr << "FAIL recv() failed for socket: " << sock << " with error: " << GETSOCKERROR() << '\n';
            continue;
        }

        std::cout << "STAT received " << recv_bytes << " bytes\n";
        {
            const std::string request = std::string(buffer.data(), recv_bytes);
            std::cout << "VERB Request received\n";
            std::cout << std::string(35, '=') << '\n' << request << std::string(35, '=') << '\n';

            const std::string body = "Local time is: " + std::string(current_time());
            const std::string response = 
                "HTTP/1.1 200 OK\r\n"
                "Connection: keep-alive\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: " + std::to_string(body.size()) + "\r\n"
                "\r\n" +
                body;
            send_response(sock, response, 0);
        }
    }
}

