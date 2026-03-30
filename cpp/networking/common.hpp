#include <errno.h>

#include <cstring>
#include <array>

#define GETSOCKERROR() strerror(errno)

typedef int SOCKET;

template <size_t N>
using MCSTR = std::array<char, N>;

template <size_t N>
inline void MCSTR_ZERO(std::array<char, N>& arr) { std::fill(arr.begin(), arr.end(), 0); }
