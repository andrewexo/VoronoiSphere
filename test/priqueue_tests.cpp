#include "../src/priqueue.h"
#include "../src/priqueue.cpp"
#include "gtest/gtest.h"
#include <vector>
#include <chrono>
#include <iostream>

struct event {
    size_t value;
    void* pqn;
};
struct eventCompare
{
    inline bool operator()(event* lhs, event* rhs) {
        return lhs->value > rhs->value;
    }
};
template class PriQueue<event, eventCompare, 4, 32>;

TEST(PriQueueTests, TestPushPop)
{
    PriQueue<event, eventCompare, 4, 32> pq;
    size_t count = 20000;

    std::vector<event*> events;
    for (size_t i = 0; i < count; i++) {
        event* e = new event({(size_t)rand(), nullptr});
        events.push_back(e);
        pq.push(e);
    }
    EXPECT_EQ(pq.audit(), count);

    int last = pq.top()->value;
    for (size_t i = 0; i < count; i++) {
        int curr = pq.top()->value;
        EXPECT_GE(curr, last);
        last = curr;
        pq.pop();
    }
    EXPECT_EQ(pq.empty(), true);

    for (size_t i = 0; i < count; i++) {
        delete events[i];
    }
}

TEST(PriQueueTests, TestPushPop2)
{
    PriQueue<event, eventCompare, 4, 32> pq;
    size_t count = 4000;

    std::vector<event*> events;
    for (size_t i = 0; i < count; i++) {
        event* e = new event({(size_t)rand(), nullptr});
        events.push_back(e);
        pq.push(e);
        EXPECT_EQ(pq.audit(), i+1);
    }
    EXPECT_EQ(pq.audit(), count);

    int last = pq.top()->value;
    for (size_t i = 0; i < count; i++) {
        int curr = pq.top()->value;
        EXPECT_GE(curr, last);
        last = curr;
        pq.pop();
        EXPECT_EQ(pq.audit(), count-i-1);
    }
    EXPECT_EQ(pq.empty(), true);

    for (size_t i = 0; i < count; i++) {
        delete events[i];
    }
}

TEST(PriQueueTests, TestErase)
{
    PriQueue<event, eventCompare, 4, 32> pq;   
    size_t count = 20000;

    std::vector<event*> events;
    for (size_t i = 0; i < count; i++) {
        event* e = new event({i, nullptr});
        events.push_back(e);
        pq.push(e);
    }
    EXPECT_EQ(pq.audit(), count);

    // erase odd elements
    for (size_t i = 1; i < count; i+=2) {
        pq.erase(events[i]);
    }
    EXPECT_EQ(pq.audit(), count/2);

    for (size_t i = 0; i < (count+1)/2; i++) {
        EXPECT_GE(pq.top()->value % 2, (size_t)0);
        pq.pop();
    }
    EXPECT_EQ(pq.empty(), true);

    for (size_t i = 0; i < count; i++) {
        delete events[i];
    }
}

TEST(PriQueueTests, TestErase2)
{
    PriQueue<event, eventCompare, 4, 32> pq;   
    size_t count = 20000;

    std::vector<event*> events;
    for (int i = count-1; i >= 0; i--) {
        event* e = new event({(size_t)i, nullptr});
        events.push_back(e);
        pq.push(e);
    }
    EXPECT_EQ(pq.audit(), count);

    // erase from front of queue
    for (size_t i = 0; i < count; i++) {
        pq.erase(events[i]);
    }
    EXPECT_EQ(pq.audit(), (size_t)0);
    EXPECT_EQ(pq.empty(), true);

    for (size_t i = 0; i < count; i++) {
        delete events[i];
    }
}

// Template function to test a specific roll length
template <size_t RollLength>
std::pair<double, double> testRollLengthPerformance(size_t count) {
    PriQueue<event, eventCompare, 8, RollLength> pq;
    std::vector<event*> events;
    
    // Create events
    for (size_t i = 0; i < count; i++) {
        event* e = new event({(size_t)rand(), nullptr});
        events.push_back(e);
    }
    
    // Measure push performance
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        pq.push(events[i]);
    }
    auto pushEnd = std::chrono::high_resolution_clock::now();
    double pushTime = std::chrono::duration<double, std::milli>(pushEnd - start).count();
    
    // Measure pop performance
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        pq.pop();
    }
    auto popEnd = std::chrono::high_resolution_clock::now();
    double popTime = std::chrono::duration<double, std::milli>(popEnd - start).count();
    
    // Cleanup
    for (size_t i = 0; i < count; i++) {
        delete events[i];
    }
    
    return std::make_pair(pushTime, popTime);
}

TEST(PriQueueTests, TestRollLengthPerformance)
{
    const size_t count = 10000;
    const std::vector<int> rollLengths = {2, 4, 8, 16, 32};
    
    std::cout << "\nRoll Length Performance Test Results:" << std::endl;
    
    // Run tests for each roll length
    for (auto rollLength : rollLengths) {
        std::pair<double, double> result;
        
        switch (rollLength) {
            case 2:
                result = testRollLengthPerformance<2>(count);
                break;
            case 4:
                result = testRollLengthPerformance<4>(count);
                break;
            case 8:
                result = testRollLengthPerformance<8>(count);
                break;
            case 16:
                result = testRollLengthPerformance<16>(count);
                break;
            case 32:
                result = testRollLengthPerformance<32>(count);
                break;
            default:
                continue;
        }
        
        std::cout << "Roll Length " << rollLength << ": Push time: " << result.first 
                  << "ms, Pop time: " << result.second << "ms" << std::endl;
    }
    
    // Make sure test passes - we're just measuring performance
    EXPECT_TRUE(true);
}
