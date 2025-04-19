#ifndef PRIQUEUE_H
#define PRIQUEUE_H

#include "voronoi_event.h"
#include "voronoi_event_compare.h"
#include <random>

#include "gtest/gtest_prod.h"

#define SKIP_DEPTH 8

template <Order O>
class PriQueueNode
{
    public:

        PriQueueNode(size_t i);

        size_t index;

        size_t skips[SKIP_DEPTH];
        size_t next;

        size_t prev_skips[SKIP_DEPTH];
        size_t prev;

        void clear();
};

template <Order O>
class PriQueue
{
    public:

        PriQueue();
        ~PriQueue();

        void push(PriQueueNode<O>* node);
        CircleEvent<O>* top();
        void pop();
        bool empty();

        void erase(PriQueueNode<O>* node);

    private:

        PriQueueNode<O>* head;
        VoronoiEventCompare<O> comp;

        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution;

        void addSkips(PriQueueNode<O>* node, PriQueueNode<O>** previous);

        // tests
        FRIEND_TEST(PriQueueTests, TestInsertLinkedList);
        FRIEND_TEST(PriQueueTests, TestInsertOutOfOrderLinkedList);
        FRIEND_TEST(PriQueueTests, TestInsertReverseOrderLinkedList);
        FRIEND_TEST(PriQueueTests, TestInsertSkips);
        FRIEND_TEST(PriQueueTests, TestEraseSkips);
        FRIEND_TEST(PriQueueTests, TestEraseHeadSkips);
        FRIEND_TEST(PriQueueTests, TestEraseTailSkips);
        FRIEND_TEST(PriQueueTests, TestErasePopPushSkips);
};

#endif