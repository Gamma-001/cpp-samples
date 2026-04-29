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
    TEST_FUNC(cps::expect_eq_iter, match("ABCABCABCABC", "ABC"), std::vector<int>({0, 3, 6, 9}));
    TEST_FUNC(cps::expect_eq_iter, match("ABC ABCDAB ABCDABCDABDE", "ABCDABD"), std::vector<int>({15}));
    TEST_FUNC(cps::expect_eq_iter, match("ABCDEFG", "XYZ"), std::vector<int>{});
    TEST_FUNC(cps::expect_eq_iter, match("AAAAAAA", "AAA"), std::vector<int>({0, 1, 2, 3, 4}));
    TEST_FUNC(cps::expect_eq_iter, match("ABABABAB", "ABAB"), std::vector<int>({0, 2, 4}));
    TEST_FUNC(cps::expect_eq_iter, match("AAABAAABAAAB", "AAAB"), std::vector<int>({0, 4, 8}));
    TEST_FUNC(cps::expect_eq_iter, match("MISSISSIPPI", "ISS"), std::vector<int>({1, 4}));
    TEST_FUNC(cps::expect_eq_iter, match("HELLO WORLD", "WORLD"), std::vector<int>({6}));
    TEST_FUNC(cps::expect_eq_iter, match("ABCDE", "A"), std::vector<int>({0}));
    TEST_FUNC(cps::expect_eq_iter, match("ABCDE", "E"), std::vector<int>({4}));
    TEST_FUNC(cps::expect_eq_iter, match("ZZZZZ", "ZZ"), std::vector<int>({0, 1, 2, 3}));
    TEST_FUNC(cps::expect_eq_iter, match("ABCABCABC", "ABCABC"), std::vector<int>({0, 3}));
    TEST_FUNC(cps::expect_eq_iter, match("MISSISSIPPI", "XYZ"), std::vector<int>{}); 

    return 0;
}
