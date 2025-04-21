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
template class PriQueue<event, eventCompare>;

TEST(PriQueueTests, TestPushPop)
{
    PriQueue<event, eventCompare> pq;
    int count = 100000;

    std::vector<event*> events;
    for (int i = 0; i < count; i++) {
        event* e = new event({rand(), nullptr});
        events.push_back(e);
        pq.push(e);
    }

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

TEST(PriQueueTests, TestErase)
{
    PriQueue<event, eventCompare> pq;
    int count = 100000;

    std::vector<event*> events;
    for (int i = 0; i < count; i++) {
        event* e = new event({i, nullptr});
        events.push_back(e);
        pq.push(e);
    }

    // erase odd elements
    for (int i = 1; i < count; i+=2) {
        pq.erase(events[i]);
    }

    for (int i = 0; i < count/2; i++) {
        EXPECT_GE(pq.top()->value % 2, 0);
        pq.pop();
    }
    EXPECT_EQ(pq.empty(), true);

    for (int i = 0; i < count; i++) {
        delete events[i];
    }
}
