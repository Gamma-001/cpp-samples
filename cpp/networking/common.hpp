#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <cstring>
#include <array>
#include <string>
#include <utility>
#include <iostream>

#define GETSOCKERROR() strerror(errno)

typedef int SOCKET;
typedef SOCKET weak_socket;

constexpr SOCKET SOCKINVL = -1;

template <size_t N>
using MCSTR = std::array<char, N>;

template <size_t N>
inline void MCSTR_ZERO(std::array<char, N>& arr) { std::fill(arr.begin(), arr.end(), 0); }

namespace cps {
    inline std::pair<std::string, std::string> getnameinfo(sockaddr* addr, socklen_t addr_len, int flags = NI_NUMERICHOST | NI_NUMERICSERV);
    class scoped_addrinfo;
    class scoped_socket;
}

inline std::pair<std::string, std::string> cps::getnameinfo(sockaddr* addr, socklen_t addr_len, int flags) {
    MCSTR<101> buff_addr;
    MCSTR<101> buff_serv;
    MCSTR_ZERO(buff_addr);
    MCSTR_ZERO(buff_serv);
    if (getnameinfo(addr, addr_len, buff_addr.data(), buff_addr.size() - 1, buff_serv.data(), buff_serv.size() - 1, flags) != 0) {
        std::cerr << "FAIL, getnameinfo() failed, error: " << GETSOCKERROR() << '\n';
        return std::make_pair("", "");
    }

    return std::make_pair(std::string(buff_addr.data()), std::string(buff_serv.data()));
}

class cps::scoped_addrinfo {
public:
    explicit scoped_addrinfo(addrinfo* addr = nullptr)
    : m_addr(addr) {}
    scoped_addrinfo(const scoped_addrinfo&) = delete;
    scoped_addrinfo& operator=(const scoped_addrinfo&) = delete;


    ~scoped_addrinfo() {
        this->reset();
    }

    addrinfo* get() const noexcept { return m_addr;}
    addrinfo* operator->() const noexcept { return m_addr; }
    addrinfo** ref() noexcept { return &m_addr; }
    void reset() noexcept {
        if (m_addr != nullptr) {
            freeaddrinfo(m_addr);
            m_addr = nullptr;
        }
    }

private:
    addrinfo* m_addr;
};

class cps::scoped_socket {
public:
    explicit scoped_socket(SOCKET sock = SOCKINVL) noexcept
    : m_sock(sock) { }
    scoped_socket(const scoped_socket&) = delete;
    scoped_socket(scoped_socket&& other) noexcept
    : m_sock(other.m_sock) { 
        other.m_sock = SOCKINVL; 
    }
    
    scoped_socket& operator=(const scoped_socket&) = delete;
    scoped_socket& operator=(scoped_socket&& other) noexcept {
        if (this != &other) {
            this->close();
            this->m_sock = other.m_sock;
            other.m_sock = SOCKINVL;
        }

        return *this;
    }
    scoped_socket& operator=(SOCKET sock) noexcept {
        if (sock != m_sock) {
            this->close();
            m_sock = sock;
        }

        return *this;
    }

    ~scoped_socket() noexcept { 
        close(); 
    }

    weak_socket get() const noexcept { return m_sock; }
    bool invalid() const noexcept { return m_sock == SOCKINVL; }
    void close() noexcept {
        if (m_sock != SOCKINVL) {
            ::close(m_sock);
            m_sock = SOCKINVL;
        }
    }

private:
    SOCKET m_sock;
};
