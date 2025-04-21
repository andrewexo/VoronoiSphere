#ifndef PRIQUEUE_H
#define PRIQUEUE_H

#include "voronoi_event.h"
#include "voronoi_event_compare.h"
#include <random>

#include "gtest/gtest_prod.h"

#define SKIP_DEPTH 4
#define ROLL_LENGTH 16
static_assert(ROLL_LENGTH % 2 == 0);

template <typename T>
class PriQueueNode
{
    public:

        PriQueueNode(T* event);
        PriQueueNode();
        void clear();
        uint8_t count;
        T* event[ROLL_LENGTH];

        PriQueueNode<T>* skips[SKIP_DEPTH];
        PriQueueNode<T>* next;

        PriQueueNode<T>* prev_skips[SKIP_DEPTH];
        PriQueueNode<T>* prev;
};

template <typename T, typename Compare>
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

        PriQueueNode<T>* head;
        Compare comp;

        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution;

        void addSkips(PriQueueNode<T>* node, PriQueueNode<T>** previous);

        // tests
        FRIEND_TEST(PriQueueTests, TestPushPop);
        FRIEND_TEST(PriQueueTests, TestErase);
};

#endif