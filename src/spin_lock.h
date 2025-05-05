#pragma once

#include "platform.h"
#include "globals.h"
#include <atomic>

namespace VorGen {

class SpinLock
{
    public:

        SpinLock();

        void lock();
        void unlock();

        ::std::atomic<uint8_t> m_status;
};

}