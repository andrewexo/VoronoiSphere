#include "priqueue.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include "memblock.h"



template <typename T, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>::PriQueueNode(T* event)
{
    count = 1;
    this->event[0] = event;
    event->pqn = this;
    clear();
}

template <typename T, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>::PriQueueNode()
{
    count = 0;
    clear();
}

template <typename T, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
void PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>::clear()
{
    for (size_t i = count; i < ROLL_LENGTH; i++)
        event[i] = nullptr;
    for (size_t i = 0; i < SKIP_DEPTH; i++)
    {
        skips[i] = nullptr;
        prev_skips[i] = nullptr;
    }
    next = nullptr;
    prev = nullptr;
}

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::PriQueue()
{
    head = nullptr;
    distribution = std::uniform_int_distribution<int>(0, DIST_MAX);
}

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::~PriQueue()
{
}

// 25.01% - 32.13%
template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
void PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::push(T* event)
{
    if (head == nullptr)
    {
        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* node = new PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>(event);
        head = node;
        return;
    }

    if (comp(head->event[0], event)) // insert at front
    {
        if (head->count < ROLL_LENGTH) {
            memmove(head->event + 1, head->event, head->count * sizeof(T*));
            head->event[0] = event;
            head->count++;
            event->pqn = head;
            return;
        }

        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* node = new PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>(event);
        node->next = head;
        head->prev = node;
        for (size_t i = 0; i < SKIP_DEPTH; i++)
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
    PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* nodes[SKIP_DEPTH];
    PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* curr = head;

    while (true)
    {
        PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* next = curr->skips[skip_level];
        if (curr->skips[skip_level] != nullptr && 
            comp(event, next->event[0]))
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
    while (curr->next != nullptr && comp(event, curr->next->event[0]))
    {
        curr = curr->next;
    }

    // insert into curr
    if (curr->count < ROLL_LENGTH) {
        for (size_t i = 1; i < curr->count; i++ ) {
            if (comp(curr->event[i], event)) {
                memmove(curr->event+i+1, curr->event+i, (curr->count-i) * sizeof(T*));
                curr->event[i] = event;
                curr->count++;
                event->pqn = curr;
                return;
            }   
        }
        curr->event[curr->count++] = event;
        event->pqn = curr;
        return;
    }

    // split curr into two nodes
    PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* node = new PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>();
    size_t i = 1;
    for (; i < ROLL_LENGTH; i++) { 
        if (comp(curr->event[i], event)) {
            break;
        }
    }
    if (i < ROLL_LENGTH/2) {
        memmove(node->event, curr->event+(ROLL_LENGTH/2-1), (ROLL_LENGTH/2+1) * sizeof(T*));
        memmove(curr->event+i+1, curr->event+i, (ROLL_LENGTH/2-i-1) * sizeof(T*));
        curr->event[i] = event;
        event->pqn = curr;
    }
    else {
        memmove(node->event, curr->event+(ROLL_LENGTH/2), (i - ROLL_LENGTH/2) * sizeof(T*));
        node->event[i-ROLL_LENGTH/2] = event;
        memmove(node->event+(i-ROLL_LENGTH/2+1), curr->event+i, (ROLL_LENGTH-i) * sizeof(T*));
    }
    for (size_t j = 0; j < ROLL_LENGTH/2+1; j++) {
        node->event[j]->pqn = node;
    }
    node->count = ROLL_LENGTH/2+1;
    for (size_t j = ROLL_LENGTH/2; j < ROLL_LENGTH; j++) {
        curr->event[j] = nullptr;
    }
    curr->count = ROLL_LENGTH/2;
    
    // insert node into list
    node->next = curr->next;
    node->prev = curr;
    if (curr->next != nullptr)
        curr->next->prev = node;
    curr->next = node;

    addSkips(node, nodes);
}

inline int log2_5(int n)
{
    const unsigned int b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
    const unsigned int S[] = { 1, 2, 4, 8, 16 };

    unsigned int r = 0; // result of log2(v) will go here
    for (int i = 4; i >= 0; i--) // unroll for speed...   ...actually the compiler will unroll
    {
        if (n & b[i])
        {
            n >>= S[i];
            r |= S[i];
        }
    }

    return r;
}

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
void PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::addSkips(PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* node, PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>** previous)
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

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
T* PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::top()
{
    if (head == nullptr)
        return nullptr;
    else
        return head->event[0];
}

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
void PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::pop()
{
    if (head == nullptr) return;

    head->event[0]->pqn = nullptr;

    if (head->count > 1)
    {
        memmove(head->event, head->event+1, (head->count-1) * sizeof(T*));
        head->event[--(head->count)] = nullptr;
        return;
    }

    auto oldHead = head;
    if (head->next != nullptr)
    {
        for (size_t i = 0; i < SKIP_DEPTH; i++)
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

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
bool PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::empty()
{
    return head == nullptr;
}

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
void PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::erase(T* event)
{
    PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* node = (PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>*)event->pqn;
    event->pqn = nullptr;

    if (node == nullptr) return;

    if (node->count > 1)
    {
        for (size_t i = 0; i < node->count; i++) {
            if (node->event[i] == event) {
                memmove(node->event+i, node->event+i+1, (node->count-i-1) * sizeof(T*));
                node->event[--(node->count)] = nullptr;
                return;
            }
        }
    }

    if (node->prev == nullptr)
    {
        pop();
    }
    else if (node->next == nullptr)
    {
        node->prev->next = nullptr;
        for (size_t i = 0; i < SKIP_DEPTH; i++)
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

        for (size_t i = 0; i < SKIP_DEPTH; i++)
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

template <typename T, typename Compare, size_t SKIP_DEPTH, size_t ROLL_LENGTH>
size_t PriQueue<T, Compare, SKIP_DEPTH, ROLL_LENGTH>::audit()
{
    size_t count = 0;
    PriQueueNode<T, SKIP_DEPTH, ROLL_LENGTH>* curr = head;
    while (curr != nullptr) {
        count += curr->count;
        for (size_t i = 0; i < curr->count; i++) {
            assert(curr->event[i] != nullptr);
        }
        for (size_t i = curr->count; i < ROLL_LENGTH; i++) {
            assert(curr->event[i] == nullptr);
        }
        curr = curr->next;
    }
    return count;
}

// Forward declare template types so compiler generates code to link against
template class PriQueue<CircleEvent<Increasing>, VoronoiEventCompare<Increasing>, 8, 32>;
template class PriQueue<CircleEvent<Decreasing>, VoronoiEventCompare<Decreasing>, 8, 32>;

template class PriQueueNode<CircleEvent<Increasing>, 8, 32>;
template class PriQueueNode<CircleEvent<Decreasing>, 8, 32>;