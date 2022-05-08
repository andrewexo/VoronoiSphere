#include "buckets.h"
#include "memblock.h"
#include <cassert>

template <Order O>
Buckets<O>::Buckets()
{

}

template <>
void Buckets<Increasing>::push(PriQueueNode<Increasing>* node)
{
  auto ce = getCircleEventFromPriQueueNode(node);
  auto polar = ce->polar + ce->polar_small;
  size_t idx = (size_t)(BUCKETS * (polar / (2*M_PI)));

  m_queues[idx].push(node);
}

template <>
void Buckets<Decreasing>::push(PriQueueNode<Decreasing>* node)
{
  auto ce = getCircleEventFromPriQueueNode(node);
  auto polar = ce->polar - ce->polar_small + M_PI;
  size_t idx = (size_t)(BUCKETS * (polar / (2*M_PI)));

  m_queues[idx].push(node);
}

template <>
CircleEvent<Increasing>* Buckets<Increasing>::top()
{
  for (size_t i = 0; i < BUCKETS; ++i)
  {
    if (!(m_queues[i].empty()))
    {
      return m_queues[i].top();
    }
  }

  return nullptr;
}

template <>
CircleEvent<Decreasing>* Buckets<Decreasing>::top()
{
  for (size_t i = BUCKETS-1; i < BUCKETS; --i)
  {
    if (!(m_queues[i].empty()))
    {
      return m_queues[i].top();
    }
  }

  return nullptr;
}

template <>
void Buckets<Increasing>::pop()
{
  for (size_t i = 0; i < BUCKETS; ++i)
  {
    if (!(m_queues[i].empty()))
    {
      m_queues[i].pop();
      return;
    }
  }
}

template <>
void Buckets<Decreasing>::pop()
{
  for (size_t i = BUCKETS-1; i < BUCKETS; --i)
  {
    if (!(m_queues[i].empty()))
    {
      m_queues[i].pop();
      return;
    }
  }
}

template <>
bool Buckets<Increasing>::empty()
{
  for (size_t i = 0; i < BUCKETS; ++i)
  {
    if (!(m_queues[i].empty()))
    {
      return false;
    }
  }
  return true;
}

template <>
bool Buckets<Decreasing>::empty()
{
  for (size_t i = BUCKETS-1; i < BUCKETS; --i)
  {
    if (!(m_queues[i].empty()))
    {
      return false;
    }
  }
  return true;
}

template <>
void Buckets<Increasing>::erase(PriQueueNode<Increasing>* node)
{
  auto ce = getCircleEventFromPriQueueNode(node);
  auto polar = ce->polar + ce->polar_small;
  size_t idx = (size_t)(BUCKETS * (polar / (2*M_PI)));

  m_queues[idx].erase(node);
}

template <>
void Buckets<Decreasing>::erase(PriQueueNode<Decreasing>* node)
{
  auto ce = getCircleEventFromPriQueueNode(node);
  auto polar = ce->polar - ce->polar_small + M_PI;
  size_t idx = (size_t)(BUCKETS * (polar / (2*M_PI)));

  m_queues[idx].erase(node);
}

template class Buckets<Increasing>;
template class Buckets<Decreasing>;
