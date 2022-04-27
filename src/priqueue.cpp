#include "priqueue.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include "memblock.h"

constexpr int DIST_MAX = 1 << (SKIP_DEPTH + 1);
constexpr int SKIP_DEPTH_sub1 = SKIP_DEPTH - 1;

template <Order O>
PriQueueNode<O>::PriQueueNode(int i)
{
    index = i;
    clear();
}

template <Order O>
inline void PriQueueNode<O>::clear()
{
    memset(skips, -1, sizeof(int) * SKIP_DEPTH);
    next = -1;
    memset(prev_skips, -1, sizeof(int) * SKIP_DEPTH);
    prev = -1;
}

template <Order O>
PriQueue<O>::PriQueue()
{
    head = NULL;
    distribution = std::uniform_int_distribution<int>(0, DIST_MAX);
}

template <Order O>
PriQueue<O>::~PriQueue()
{
}

// 25.01% - 32.13%
template <Order O>
void PriQueue<O>::push(PriQueueNode<O>* node)
{
    if (head == NULL)
    {
        head = node;
    }
    else
    {
        if (comp(getCircleEventFromPriQueueNode(head), getCircleEventFromPriQueueNode(node))) // insert at front
        {
            node->next = head->index;
            head->prev = node->index;
            for (int i = 0; i < SKIP_DEPTH; i++)
            {
                if (head->skips[i] == -1) break;
                node->skips[i] = head->skips[i];
                NODE(head, skips[i])->prev_skips[i] = node->index;
                head->skips[i] = -1;
            }
            head = node;
            return;
        }

        int skip_level = SKIP_DEPTH_sub1;
        PriQueueNode<O>* nodes[SKIP_DEPTH];
        PriQueueNode<O>* curr = head;

        while (true)
        {
            PriQueueNode<O>* next = NODE(curr, skips[skip_level]);
            if (curr->skips[skip_level] != -1 && comp(getCircleEventFromPriQueueNode(node), getCircleEventFromPriQueueNode(next)))
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
        while (curr->next != -1 && comp(getCircleEventFromPriQueueNode(node), getCircleEventFromPriQueueNode(NODE(curr, next))))
        {
            curr = NODE(curr, next);
        }

        // insert after curr
        node->next = curr->next;
        node->prev = curr->index;
        if (curr->next != -1) NODE(curr, next)->prev = node->index;
        curr->next = node->index;

        addSkips(node, nodes);
    }
}

inline int log2_5(int n)
{
    const unsigned int b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
    const unsigned int S[] = { 1, 2, 4, 8, 16 };

    register unsigned int r = 0; // result of log2(v) will go here
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
        if (node->skips[i] != -1) NODE(node, skips[i])->prev_skips[i] = node->index;

        node->prev_skips[i] = (*previous)->index;
        (*previous)->skips[i] = node->index;

        previous++;
    }
}

template <Order O>
CircleEvent<O>* PriQueue<O>::top()
{
    if (head == NULL)
        return NULL;
    else
        return getCircleEventFromPriQueueNode(head);
}

template <Order O>
void PriQueue<O>::pop()
{
    if (head == NULL) return;

    if (head->next != -1)
    {
        for (int i = 0; i < SKIP_DEPTH; i++)
        {
            if (head->skips[i] == -1) break;

            if (head->skips[i] != head->next)
            {
                NODE(head, next)->skips[i] = head->skips[i];
                NODE(head, skips[i])->prev_skips[i] = head->next;
            }
            NODE(head, next)->prev_skips[i] = -1;
        }

        NODE(head, next)->prev = -1;

        head = NODE(head, next);
    }
    else
    {
        head = NULL;
    }
}

template <Order O>
bool PriQueue<O>::empty()
{
    return head == NULL;
}

template <Order O>
void PriQueue<O>::erase(PriQueueNode<O>* node)
{
    if (node->prev == -1)
    {
        pop();
    }
    else if (node->next == -1)
    {
        NODE(node, prev)->next = -1;

        for (int i = 0; i < SKIP_DEPTH; i++)
        {
            if (node->prev_skips[i] == -1) break;
            NODE(node, prev_skips[i])->skips[i] = -1;
        }
    }
    else
    {
        NODE(node, prev)->next = node->next;
        NODE(node, next)->prev = node->prev;

        for (int i = 0; i < SKIP_DEPTH; i++)
        {
            bool c = false;

            if (node->skips[i] != -1)
            {
                NODE(node, skips[i])->prev_skips[i] = node->prev_skips[i];
                c = true;
            }

            if (node->prev_skips[i] != -1)
            {
                NODE(node, prev_skips[i])->skips[i] = node->skips[i];
                c = true;
            }

            if (!c) break;
        }
    }
}

// Forward declare template types so compiler generates code to link against
template class PriQueue<Increasing>;
template class PriQueue<Decreasing>;

template class PriQueueNode<Increasing>;
template class PriQueueNode<Decreasing>;