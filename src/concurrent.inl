#pragma once

#include <vector>
#include <thread>

// Concurrently calls 'func' with input values from [start,end)
inline void concurrent(auto func, size_t start, size_t end, size_t n_threads) {
    auto wrapper = [&](size_t n) {
        double s = start + ((double)n*(end-start))/n_threads;
        double e = start + ((double)(n+1)*(end-start))/n_threads;
        for (size_t i = (size_t)s; i < (size_t)e; i++)
            func(i);
    };
    
    std::vector<std::thread> ts(n_threads);
    for (size_t i = 0; i < n_threads; i++)
        ts[i] = std::thread{wrapper, i};
    for (size_t i = 0; i < n_threads; i++)
        ts[i].join();
} 