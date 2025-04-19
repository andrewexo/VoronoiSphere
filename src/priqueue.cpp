#include "priqueue.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include "memblock.h"

constexpr int DIST_MAX = 1 << (SKIP_DEPTH + 1);
constexpr int SKIP_DEPTH_sub1 = SKIP_DEPTH - 1;

template <Order O>
PriQueueNode<O>::PriQueueNode(CircleEvent<O>* event)
: event(event) {
    clear();
}

template <Order O>
inline void PriQueueNode<O>::clear()
{
    for (int i = 0; i < SKIP_DEPTH; i++)
    {
        skips[i] = nullptr;
        prev_skips[i] = nullptr;
    }
    next = nullptr;
    prev = nullptr;
}

template <Order O>
PriQueue<O>::PriQueue()
{
    head = nullptr;
    distribution = std::uniform_int_distribution<int>(0, DIST_MAX);
}

template <Order O>
PriQueue<O>::~PriQueue()
{
}

// 25.01% - 32.13%
template <Order O>
void PriQueue<O>::push(CircleEvent<O>* event)
{
    PriQueueNode<O>* node = new PriQueueNode<O>(event);
    *getPriQueueNodePtrFromCircleEvent(event) = node;

    if (head == nullptr)
    {
        head = node;
    }
    else
    {
        if (comp(head->event, node->event)) // insert at front
        {
            node->next = head;
            head->prev = node;
            for (int i = 0; i < SKIP_DEPTH; i++)
            {
                if (head->skips[i] == nullptr) break;
                node->skips[i] = head->skips[i];
                head->skips[i]->prev_skips[i] = node;
                head->skips[i] = nullptr;
            }
            head = node;
            return;
        }

        int skip_level = SKIP_DEPTH_sub1;
        PriQueueNode<O>* nodes[SKIP_DEPTH];
        PriQueueNode<O>* curr = head;

        while (true)
        {
            PriQueueNode<O>* next = curr->skips[skip_level];
            if (curr->skips[skip_level] != nullptr && 
                comp(node->event, next->event))
            {
                curr = next;
            }
            else
            {
                nodes[skip_level--] = curr;
                if (skip_level < 0) break;
            }
        }

        // continue search on the linked list level
        while (curr->next != nullptr && 
               comp(node->event, curr->next->event))
        {
            curr = curr->next;
        }

        // insert after curr
        node->next = curr->next;
        node->prev = curr;
        if (curr->next != nullptr)
            curr->next->prev = node;
        curr->next = node;

        addSkips(node, nodes);
    }
}

inline int log2_5(int n)
{
    const unsigned int b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
    const unsigned int S[] = { 1, 2, 4, 8, 16 };

    unsigned int r = 0; // result of log2(v) will go here
    for (int i = 4; i >= 0; i--) // unroll for speed...
    {
        if (n & b[i])
        {
            n >>= S[i];
            r |= S[i];
        }
    }

    return r;
}

template <Order O>
void PriQueue<O>::addSkips(PriQueueNode<O>* node, PriQueueNode<O>** previous)
{
    int skip_count = (int) (SKIP_DEPTH - log2_5(std::max((int)(distribution(generator)), 1)));

    for (int i = 0; i < skip_count; i++)
    {
        node->skips[i] = (*previous)->skips[i];
        if (node->skips[i] != nullptr) node->skips[i]->prev_skips[i] = node;

        node->prev_skips[i] = (*previous);
        (*previous)->skips[i] = node;

        previous++;
    }
}

template <Order O>
CircleEvent<O>* PriQueue<O>::top()
{
    if (head == nullptr)
        return nullptr;
    else
        return head->event;
}

template <Order O>
void PriQueue<O>::pop()
{
    if (head == nullptr) return;
    auto oldHead = head;

    if (head->next != nullptr)
    {
        for (int i = 0; i < SKIP_DEPTH; i++)
        {
            if (head->skips[i] == nullptr) break;

            if (head->skips[i] != head->next)
            {
                head->next->skips[i] = head->skips[i];
                head->skips[i]->prev_skips[i] = head->next;
            }
            head->next->prev_skips[i] = nullptr;
        }

        head->next->prev = nullptr;

        head = head->next;
    }
    else
    {
        head = nullptr;
    }
    delete oldHead;
}

template <Order O>
bool PriQueue<O>::empty()
{
    return head == nullptr;
}

template <Order O>
void PriQueue<O>::erase(CircleEvent<O>* event)
{
    PriQueueNode<O>* node = *getPriQueueNodePtrFromCircleEvent(event);
    if (node == nullptr) return;

    if (node->prev == nullptr)
    {
        pop();
    }
    else if (node->next == nullptr)
    {
        node->prev->next = nullptr;

        for (int i = 0; i < SKIP_DEPTH; i++)
        {
            if (node->prev_skips[i] == nullptr) break;
            node->prev_skips[i]->skips[i] = nullptr;
        }
        delete node;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;

        for (int i = 0; i < SKIP_DEPTH; i++)
        {
            bool c = false;

            if (node->skips[i] != nullptr)
            {
                node->skips[i]->prev_skips[i] = node->prev_skips[i];
                c = true;
            }

            if (node->prev_skips[i] != nullptr)
            {
                node->prev_skips[i]->skips[i] = node->skips[i];
                c = true;
            }

            if (!c) break;
        }
        delete node;
    }
}

// Forward declare template types so compiler generates code to link against
template class PriQueue<Increasing>;
template class PriQueue<Decreasing>;

template class PriQueueNode<Increasing>;
template class PriQueueNode<Decreasing>;