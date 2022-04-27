#ifndef VORONOI_H
#define VORONOI_H

#include <vector>
#include <cmath>
#include "../glm/glm.hpp"
#include "voronoi_event.h"
#include "voronoi_site.h"
#include "beachline.h"
#include "voronoi_event_compare.h"
#include "priqueue.h"
#include "memblock.h"
#include "globals.h"

template <Order O, Axis A>
class VoronoiSweeper
{
  public:

    VoronoiSweeper(
      std::vector<VoronoiSite>* sites, 
      unsigned int gen, 
      uint8_t threadId);
    ~VoronoiSweeper();

    void sweep();

  private:
      
    double m_sweeplineLarge;
    double m_sweeplineSmall;

    BeachLine<O> m_beachLine;
    PriQueue<O> m_circles;

    std::vector<VoronoiSite>* m_sites;
    unsigned int m_next;

    unsigned int m_gen;
    uint8_t m_threadId;

    VoronoiSiteEventCompare<O> voronoi_site_event_comp;

    void processEvents();

    void processSiteEvent(VoronoiSite* site);
    void processCircleEvent(CircleEvent<O>* circle);

    bool onOtherSide(const glm::dvec3 & cc);

    void addCircleEventProcessSite(SkipNode<O>* node);
    void addCircleEventProcessCircle(SkipNode<O>* node);
    inline void addCircleEvent(
      SkipNode<O>* node, 
      double lp, 
      double sp, 
      const glm::dvec3 & cc);

    void removeCircleEvent(BeachArc<O>* arc);

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

#endif