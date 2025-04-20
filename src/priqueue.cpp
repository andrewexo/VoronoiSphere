#include "priqueue.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include "memblock.h"

constexpr int DIST_MAX = 1 << (SKIP_DEPTH + 1);
constexpr int SKIP_DEPTH_sub1 = SKIP_DEPTH - 1;

template <Order O>
PriQueueNode<O>::PriQueueNode(CircleEvent<O>* event)
{
    count = 1;
    this->event[0] = event;
    event->pqn = this;
    clear();
}

template <Order O>
PriQueueNode<O>::PriQueueNode()
{
    count = 0;
    clear();
}

template <Order O>
void PriQueueNode<O>::clear()
{
    for (int i = count; i < ROLL_LENGTH; i++)
        event[i] = nullptr;
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
    if (head == nullptr)
    {
        PriQueueNode<O>* node = new PriQueueNode<O>(event);
        head = node;
        return;
    }

    if (comp(head->event[0], event)) // insert at front
    {
        if (head->count < ROLL_LENGTH) {
            memmove(head->event + 1, head->event, head->count * sizeof(CircleEvent<O>*));
            head->event[0] = event;
            head->count++;
            event->pqn = head;
            return;
        }

        PriQueueNode<O>* node = new PriQueueNode<O>(event);
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
                memmove(curr->event+i+1, curr->event+i, (curr->count-i) * sizeof(CircleEvent<O>*));
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
    PriQueueNode<O>* node = new PriQueueNode<O>();
    //  Code below assumes ROLL_LENGTH is 4
    size_t i = 1;
    for (; i < ROLL_LENGTH; i++) { 
        if (comp(curr->event[i], event)) {
            break;
        }
    }
    if (i == 1) {
        node->event[0] = curr->event[1];
        node->event[1] = curr->event[2];
        node->event[2] = curr->event[3];
        curr->event[1] = event;
        event->pqn = curr;
    } else if (i == 2) {
        node->event[0] = event;
        node->event[1] = curr->event[2];
        node->event[2] = curr->event[3];
    } else if (i == 3) {
        node->event[0] = curr->event[2];
        node->event[1] = event;
        node->event[2] = curr->event[3];
    } else { // i == 4
        node->event[0] = curr->event[2];
        node->event[1] = curr->event[3];
        node->event[2] = event;
    }
    node->event[0]->pqn = node;
    node->event[1]->pqn = node;
    node->event[2]->pqn = node;
    curr->event[2] = nullptr;
    curr->event[3] = nullptr;
    curr->count = 2;
    node->count = 3;
    
    // bool inserted = false;
    // CircleEvent<O>* displaced = nullptr;
    // for (size_t i = 1; i < ROLL_LENGTH/2; i++) {
    //     if (comp(curr->event[i], event)) {
    //         displaced = curr->event[ROLL_LENGTH/2-1];
    //         memmove(curr->event+i+1, curr->event+i, (ROLL_LENGTH/2-1-i) * sizeof(CircleEvent<O>*));
    //         curr->event[i] = event;
    //         event->pqn = curr;
    //         inserted = true;
    //         break;
    //     }
    // }
    // if (inserted) {
    //     node->event[0] = displaced;
    //     displaced->pqn = node;
    //     size_t j = 1;
    //     for (size_t i = ROLL_LENGTH/2; i < ROLL_LENGTH; i++) {
    //         node->event[j++] = curr->event[i];
    //         curr->event[i]->pqn = node;
    //         curr->event[i] = nullptr;
    //     }
    // } else {
    //     size_t j = 0;
    //     for (size_t i = ROLL_LENGTH/2; i < ROLL_LENGTH; i++) {
    //         if (comp(curr->event[i], event)) {
    //             node->event[j++] = event;
    //             event->pqn = node;
    //         }
    //         node->event[j++] = curr->event[i];
    //         curr->event[i]->pqn = node;
    //     }
    // }
    // curr->count = (ROLL_LENGTH+1)/2;
    // node->count = ROLL_LENGTH/2+1;
    
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
        return head->event[0];
}

template <Order O>
void PriQueue<O>::pop()
{
    if (head == nullptr) return;

    head->event[0]->pqn = nullptr;

    if (head->count > 1)
    {
        memmove(head->event, head->event+1, (head->count-1) * sizeof(CircleEvent<O>*));
        head->count--;
        return;
    }

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
    PriQueueNode<O>* node = event->pqn;
    event->pqn = nullptr;

    if (node == nullptr) return;

    assert(node->count <= ROLL_LENGTH);

    if (node->prev == nullptr)
    {
        pop();
    }
    else if (node->next == nullptr)
    {
        if (node->count > 1)
        {
            for (size_t i = 0; i < node->count; i++) {
                if (node->event[i] == event) {
                    memmove(node->event+i, node->event+i+1, (node->count-i-1) * sizeof(CircleEvent<O>*));
                    node->count--;
                    return;
                }
            }
        }
        assert(node->count == 1);
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
        if (node->count > 1)
        {
            for (size_t i = 0; i < node->count; i++) {
                if (node->event[i] == event) {
                    memmove(node->event+i, node->event+i+1, (node->count-i-1) * sizeof(CircleEvent<O>*));
                    node->count--;
                    return;
                }
            }
        }
        assert(node->count == 1);
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