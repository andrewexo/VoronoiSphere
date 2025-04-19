#pragma once

#include "voronoi_event.h"
#include "priqueue.h"

#define BUCKETS 4


template <Order O>
class Buckets
{
  public:

    Buckets();

    void push(PriQueueNode<O>* node);
    CircleEvent<O>* top();
    void pop();
    bool empty();

    void erase(PriQueueNode<O>* node);

  private:

    PriQueue<O> m_queues[BUCKETS];
   
};
