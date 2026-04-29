#include <vector>

#include "../test/util.hpp"

// TODO: implement a recursive table building algorithm

std::vector<int> pm_table(const std::string& w) {
    auto table = std::vector<int>(w.size() + 1, -1);

    int cnd = 0; // Partial match candidate
    for (int i = 1; i < w.size(); i++) {
        if (w[i] != w[cnd]) {
            table[i] = cnd;
            while (cnd >= 0 && w[i] != w[cnd]) {
                cnd = table[cnd];
            }
        } else {
            table[i] = table[cnd];
        }

        cnd = cnd + 1;
    }
    table[w.size()] = cnd;

    return table;
}

std::vector<int> match(const std::string& s, const std::string& w) {
    auto table = pm_table(w);
    std::vector<int> res;

    int i = 0;
    int j = 0;
    while (i < s.size()) {
        if (s[i] == w[j]) {
            i += 1;
            j += 1;
            if (j == w.size()) {
                res.push_back(i - j);
                j = table[j];
            }
        } 
        else {
            j = table[j];
            if (j < 0) {
                i += 1;
                j += 1;
            }
        }
    }

    return res;
}

int main() {
    TEST(cps::expect_eq_iter, match("ABCABCABCABC", "ABC"), std::vector<int>({0, 3, 6, 9}));
    TEST(cps::expect_eq_iter, match("ABC ABCDAB ABCDABCDABDE", "ABCDABD"), std::vector<int>({15}));
    TEST(cps::expect_eq_iter, match("ABCDEFG", "XYZ"), std::vector<int>{});
}
