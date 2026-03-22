#include <iostream>
#include <chrono>
#include <ctime>

int main() {
    std::chrono::system_clock clock;
    const std::time_t t_c = std::chrono::system_clock::to_time_t(clock.now());
    std::cout << std::ctime(&t_c);

    return 0;   
}
