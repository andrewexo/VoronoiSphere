#include "../src/priqueue.h"
#include "../src/priqueue.cpp"
#include "gtest/gtest.h"
#include <vector>

struct event {
    int value;
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
    for (int i = 0; i < count; i++) {
        event* e = new event({rand(), nullptr});
        events.push_back(e);
        pq.push(e);
    }
    EXPECT_EQ(pq.audit(), count);

    int last = pq.top()->value;
    for (int i = 0; i < count; i++) {
        int curr = pq.top()->value;
        EXPECT_GE(curr, last);
        last = curr;
        pq.pop();
    }
    EXPECT_EQ(pq.empty(), true);

    for (int i = 0; i < count; i++) {
        delete events[i];
    }
}

TEST(PriQueueTests, TestPushPop2)
{
    PriQueue<event, eventCompare, 4, 32> pq;
    size_t count = 4000;

    std::vector<event*> events;
    for (int i = 0; i < count; i++) {
        event* e = new event({rand(), nullptr});
        events.push_back(e);
        pq.push(e);
        EXPECT_EQ(pq.audit(), i+1);
    }
    EXPECT_EQ(pq.audit(), count);

    int last = pq.top()->value;
    for (int i = 0; i < count; i++) {
        int curr = pq.top()->value;
        EXPECT_GE(curr, last);
        last = curr;
        pq.pop();
        EXPECT_EQ(pq.audit(), count-i-1);
    }
    EXPECT_EQ(pq.empty(), true);

    for (int i = 0; i < count; i++) {
        delete events[i];
    }
}

TEST(PriQueueTests, TestErase)
{
    PriQueue<event, eventCompare, 4, 32> pq;   
    size_t count = 20000;

    std::vector<event*> events;
    for (int i = 0; i < count; i++) {
        event* e = new event({i, nullptr});
        events.push_back(e);
        pq.push(e);
    }
    EXPECT_EQ(pq.audit(), count);

    // erase odd elements
    for (int i = 1; i < count; i+=2) {
        pq.erase(events[i]);
    }
    EXPECT_EQ(pq.audit(), count/2);

    for (int i = 0; i < (count+1)/2; i++) {
        EXPECT_GE(pq.top()->value % 2, 0);
        pq.pop();
    }
    EXPECT_EQ(pq.empty(), true);

    for (int i = 0; i < count; i++) {
        delete events[i];
    }
}

TEST(PriQueueTests, TestErase2)
{
    PriQueue<event, eventCompare, 4, 32> pq;   
    size_t count = 20000;

    std::vector<event*> events;
    for (int i = count-1; i >= 0; i--) {
        event* e = new event({i, nullptr});
        events.push_back(e);
        pq.push(e);
    }
    EXPECT_EQ(pq.audit(), count);

    // erase from front of queue
    for (int i = 0; i < count; i++) {
        pq.erase(events[i]);
    }
    EXPECT_EQ(pq.audit(), 0);
    EXPECT_EQ(pq.empty(), true);

    for (int i = 0; i < count; i++) {
        delete events[i];
    }
}
