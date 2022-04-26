#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#include <atomic>

class SpinLock
{
    public:

        SpinLock();

        void lock();
        void unlock();

        std::atomic<uint8_t> m_status;
};

#endif