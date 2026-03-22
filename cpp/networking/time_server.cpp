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
#include <errno.h>

typedef int SOCKET;
#define GETSOCKERROR() strerror(errno)

constexpr const char* PORT = "8080";

std::string current_time();
std::string get_ipaddr(sockaddr* addr, socklen_t addrlen);

void send_response(SOCKET sock, const std::string& response, int flags = 0);


int main() {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Poppulate addrinfo list with address matching requested hints
    addrinfo* bind_address;
    if (getaddrinfo(0, PORT, &hints, &bind_address) == -1) {
        std::cout << "getaddrinfo() failed, error: " << GETSOCKERROR() << '\n';
        return -1;
    }

    // create a socket to listen for incoming connections
    SOCKET sock_server;
    sock_server = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if (sock_server < 0) {
        std::cout << "Failed to created socket, error: " << GETSOCKERROR() << '\n';
        return -1;
    }

    // unset IPV6_V6ONLY to support dual stack
    int option = 0;
    if (setsockopt(sock_server, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<void*>(&option), sizeof(option))) {  
        std::cout << "setsockopt() failed, error: " << GETSOCKERROR() << '\n';
        return -1;
    }

    // Associate the socket with the host and port
    if (bind(sock_server, bind_address->ai_addr, bind_address->ai_addrlen) != 0) {
        std::cout << "Failed to bind the socket, error: " << GETSOCKERROR() << '\n';
        return -1;
    }
    freeaddrinfo(bind_address);

    // Listen for new connections
    std::cout << "Listening on port:" << PORT << " (all interfaces) ...\n";
    if (listen(sock_server, 10) < 0) {
        std::cout << "listen() failed, error: " << GETSOCKERROR() << '\n';
        return -1;
    }

    // Accept a client connection and create a new associated client socket
    sockaddr_storage c_addr;
    socklen_t c_addr_len = sizeof(c_addr);
    SOCKET sock_client = accept(sock_server, reinterpret_cast<sockaddr*>(&c_addr), &c_addr_len);
    if (sock_client < 0) {
        std::cout << "accept(), error: " << GETSOCKERROR() << '\n';
        return -1;
    }
    std::string ipaddr = get_ipaddr(reinterpret_cast<sockaddr*>(&c_addr), c_addr_len);
    if (ipaddr.empty()) return -1;
    else std::cout << "Client connected with address: " << ipaddr << '\n';

    // Wait unti a request is received
    char request[1025];
    int bytes_recv = recv(sock_client, request, sizeof(request) - 1, 0);
    if (bytes_recv < 0) {
        std::cout << "recv() failed, error: " << GETSOCKERROR() << '\n';
    }
    else if (bytes_recv == 0) {
        std::cout << "Connection closed\n";
    }
    else {
        // Send a response back with current local time
        request[bytes_recv] = '\0';
        std::cout << std::string(30, '=') << '\n';
        std::cout << "Received " << bytes_recv <<" bytes\n";
        std::cout << "Request:\n" << request << '\n';
        std::cout << std::string(30, '=') << '\n';
        
        const std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "Local time is: ";
        send_response(sock_client, response, 0);
        send_response(sock_client, current_time(), 0);
    }

    // Close the client and serer socket
    std::cout << "Closing connection...\n";
    close(sock_client);

    std::cout << "Closing the listening socket...\n";
    close(sock_server);

    return 0;
}

std::string get_ipaddr(sockaddr* addr, socklen_t addrlen) {
    std::array<char, 100> ipaddr;
    if (getnameinfo(addr, addrlen, ipaddr.data(), ipaddr.size(), nullptr, 0, NI_NUMERICHOST) != 0) {
        std::cout << "getnameinfo() failed, error: " << GETSOCKERROR() << '\n';
        return "";
    }

    return std::string(ipaddr.data());
}

std::string current_time() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    
    return std::string(std::ctime(&t_c));
}

void send_response(SOCKET sock, const std::string& response, int flags) {
    int bytes_sent = send(sock, response.c_str(), response.size(), flags);
    std::cout << "Sent " << bytes_sent << " of " << response.size() << " bytes.\n";
}

