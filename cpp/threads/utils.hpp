#pragma once

#include <thread>
#include <random>

// ---------- thread utils

class ScopedThread {
public:
    enum class ThreadScope {
        detach,
        join
    };

    inline ScopedThread(std::thread&& thread, ThreadScope scope);
    inline ~ScopedThread();

private:
    std::thread m_thread;
    ThreadScope m_scope;
};

ScopedThread::ScopedThread(std::thread&& thread, ThreadScope scope)
    : m_thread(std::move(thread))
    , m_scope(scope) {}

ScopedThread::~ScopedThread() {
    switch (m_scope) {
        case ThreadScope::join:
            m_thread.join();
            break;

        case ThreadScope::detach:
            m_thread.detach();
            break;

        default:
            break;
    }
}

// ---------- general utils

inline int randInt(int start, int end) {
    static std::random_device rd;
    static std::default_random_engine gen(rd());
    std::uniform_int_distribution<int> dist(start, end);

    return dist(gen);
}
