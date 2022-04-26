#include "spin_lock.h"

SpinLock::SpinLock()
{
    m_status.store(0);
}

void SpinLock::lock()
{
    while (1)
    {
        uint8_t prev = m_status.fetch_or(1);
        if (!prev)
            break;
    }
}

void SpinLock::unlock()
{
    m_status.store(0);
}
