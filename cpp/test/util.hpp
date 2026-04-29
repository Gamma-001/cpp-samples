#include <iterator>
#include <iostream>
#include <vector>

template <typename T>
std::string expand(T) {
    return "";
}

template <typename T>
std::string expand(std::vector<T> vec) {
    std::string res = "<";
    for (int i = 0; i < vec.size(); i++) {
        res += std::to_string(vec[i]);
        if (i != vec.size() - 1) res += ", ";
    }
    res += ">";

    return res;
}

#define TEST_FUNC(func, p1, p2) \
{\
    bool res = func(p1, p2); \
    std::string fsig = std::string(#func) + "(" + std::string(#p1) +", " + expand(p2) + ")"; \
    \
    if (res) std::cout << "TEST PASS \n - " << fsig << '\n'; \
    else std::cout << "TEST FAIL " << fsig << " Found: " << expand(p1) << "\n"; \
}

namespace cps {
    template<typename T>
    bool expect_eq_iter(const T& a, const T& b) {
        if (a.size() != b.size()) return false;
        auto it1 = a.begin();
        auto it2 = b.begin();
        for (size_t i = 0; i < a.size(); i++) {
            if (*it1 != *it2) {
                return false;
            }
            std::advance(it1, 1);
            std::advance(it2, 1);
        }

        return true;
    }
}


