#include <mutex>

class heir_mutex {
private:
    static thread_local uint16_t    s_thisThreadHeirarchy;
    
    mutable std::mutex              m_mutex;
    uint16_t                        m_prevHeirarchy;
    uint16_t                        m_heirarchy;

    void throwIfInvalid();
    void updateHeirarchy() noexcept;

public:
    heir_mutex(uint16_t heirarchy);
    
    void lock();
    void unlock();
    bool try_lock();
};

int main() {
    
    return 0;
}

// ---------- Definitions

// # heir_mutex START
// # public

thread_local uint16_t heir_mutex::s_thisThreadHeirarchy = std::numeric_limits<uint16_t>::max();

heir_mutex::heir_mutex(uint16_t heirarchy)
: m_heirarchy(heirarchy)
, m_prevHeirarchy(0) {
    // no-op
}

void heir_mutex::lock() {
    throwIfInvalid();
    m_mutex.lock();
    updateHeirarchy(); // update must be called after ::lock since ::lock can throw std::system_error
}

void heir_mutex::unlock() {
    m_mutex.unlock();
    s_thisThreadHeirarchy = m_prevHeirarchy;
}

bool heir_mutex::try_lock() {
    throwIfInvalid();
    if (!m_mutex.try_lock()) return false;
    updateHeirarchy();
    return true;
}

// # private


void heir_mutex::throwIfInvalid() {
    if (m_heirarchy <= heir_mutex::s_thisThreadHeirarchy) {
        throw std::logic_error("Locking heirarchy rule violated");
    }
}

void heir_mutex::updateHeirarchy() noexcept {
    m_prevHeirarchy = s_thisThreadHeirarchy;
    s_thisThreadHeirarchy = m_heirarchy;
}


// # heir_mutex END

