#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cstring>
#include <array>
#include <string>
#include <utility>
#include <iostream>

#define GETSOCKERROR() strerror(errno)

typedef int SOCKET;

template <size_t N>
using MCSTR = std::array<char, N>;

template <size_t N>
inline void MCSTR_ZERO(std::array<char, N>& arr) { std::fill(arr.begin(), arr.end(), 0); }

namespace CPS {
    inline std::pair<std::string, std::string> getnameinfo(sockaddr* addr, socklen_t addr_len, int flags = NI_NUMERICHOST | NI_NUMERICSERV);
}

inline std::pair<std::string, std::string> CPS::getnameinfo(sockaddr* addr, socklen_t addr_len, int flags) {
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

