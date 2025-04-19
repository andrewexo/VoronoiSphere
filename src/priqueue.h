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

        PriQueueNode(CircleEvent<O>* event);
        CircleEvent<O>* event;

        PriQueueNode<O>* skips[SKIP_DEPTH];
        PriQueueNode<O>* next;

        PriQueueNode<O>* prev_skips[SKIP_DEPTH];
        PriQueueNode<O>* prev;

        void clear();
};

template <Order O>
class PriQueue
{
    public:

        PriQueue();
        ~PriQueue();

        void push(CircleEvent<O>* event);
        CircleEvent<O>* top();
        void pop();
        bool empty();

        void erase(CircleEvent<O>* event);

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