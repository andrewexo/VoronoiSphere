#ifndef PRIQUEUE_H
#define PRIQUEUE_H

#include "voronoi_event.h"
#include "voronoi_event_compare.h"
#include <random>

#include "gtest/gtest_prod.h"


template <typename T, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
class PriQueueNode
{
    static_assert(ROLL_LENGTH % 2 == 0);

    public:

        PriQueueNode(T* event);
        PriQueueNode();
        void clear();
        uint8_t count;
        T* event[ROLL_LENGTH];

        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* skips[SKIP_DEPTH];
        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* next;

        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* prev_skips[SKIP_DEPTH];
        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* prev;
};

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
class PriQueue
{
    public:

        PriQueue();
        ~PriQueue();

        void push(T* event);
        T* top();
        void pop();
        bool empty();

        void erase(T* event);

    private:

        static constexpr int DIST_MAX = 1 << (SKIP_DEPTH + 1);
        static constexpr int SKIP_DEPTH_sub1 = SKIP_DEPTH - 1;

        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* head;
        Compare comp;

        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution;

        void addSkips(PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* node, PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>** previous);

        size_t audit();

        // tests
        FRIEND_TEST(PriQueueTests, TestPushPop);
        FRIEND_TEST(PriQueueTests, TestPushPop2);
        FRIEND_TEST(PriQueueTests, TestErase);  
        FRIEND_TEST(PriQueueTests, TestErase2);
};

#endif