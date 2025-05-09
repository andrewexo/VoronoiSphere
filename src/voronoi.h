#pragma once

#include <vector>
#include "../glm/glm.hpp"
#include "voronoi_event.h"
#include "voronoi_site.h"
#include "beachline.h"
#include "voronoi_event_compare.h"
#include "priqueue.h"
#include "memblock.h"
#include "globals.h"

namespace VorGen {

template <Order O>
const double sweeplineStart;

template <Order O>
class OrderedIterator 
{
  size_t index;
  const size_t maxSize;


  public:
    OrderedIterator(size_t maxSize);
    size_t operator++(int);
    inline bool isInRange();
    inline bool isAtEnd();
    inline size_t getIndex() { return index; };
};

template <Order O, Axis A>
class VoronoiSweeper
{
  public:

    VoronoiSweeper(
      ::std::vector<VoronoiSite>* sites, 
      size_t gen, 
      uint8_t threadId);
    ~VoronoiSweeper();

    void sweep();

  private:
      
    double m_sweeplineLarge;
    double m_sweeplineSmall;

    BeachLine<O> m_beachLine;
    PriQueue<CircleEvent<O>, VoronoiEventCompare<O>, 8, 64> m_circles;

    ::std::vector<VoronoiSite>* m_sites;
    OrderedIterator<O> m_next;

    size_t m_gen;
    uint8_t m_threadId;

    VoronoiSiteEventCompare<O> voronoi_site_event_comp;

    void processEvents();

    void processSiteEvent(VoronoiSite* site);
    void processCircleEvent(CircleEvent<O>* circle);

    bool onOtherSide(const glm::dvec3 & cc);

    bool eventIsUpcoming(double small_polar, double large_polar);

    void addCircleEventProcessSite(SkipNode<O>* node);
    void addCircleEventProcessCircle(SkipNode<O>* node);
    inline void addCircleEvent(
      SkipNode<O>* node, 
      double lp, 
      double sp, 
      const glm::dvec3 & cc);

    void removeCircleEvent(SkipNode<O>* node);

    // Memory buffer
    int block;
    MemBlock<O>* m_memBlocks;
    MemBlock<O>* m_nextBlock;

    SkipNode<O>* initBlock();

  public:
    static glm::dvec3 circumcenter(
      const glm::dvec3 & i, 
      const glm::dvec3 & j, 
      const glm::dvec3 & k);
};

}