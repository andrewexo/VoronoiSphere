#ifndef BEACHLINE_H
#define BEACHLINE_H

#include "globals.h"
#include "voronoi_event.h"
#include "voronoi_site.h"
#include <random>

#define SKIP_DEPTH_B 8

template <Order O>
class BeachArc
{
    public:
        VoronoiSite* m_site;
};

struct SweepLine
{
    double m_polar;
    double m_polCos;
    double m_polSin;
};

template <Order O>
class SkipNode
{
    public:

        void init(int i);
        void initSite(VoronoiSite* site, uint8_t threadId);
        SkipNode(int i);
        ~SkipNode();
        double getRangeEnd(const SweepLine & sl, double shift, SkipNode<O>* other);
        double intersect(VoronoiSite* siteA, VoronoiSite* siteB, const SweepLine & sl, double shift);

        // SIMD intersect: computes intersection between a,b and c,d. Stores the results in out[1], out[0]
        void intersect2(VoronoiSite* siteA, VoronoiSite* siteB, VoronoiSite* siteC, VoronoiSite* siteD, const SweepLine & sl, double shift, double* out);

        int index;

        int skips[SKIP_DEPTH_B];
        int p_skips[SKIP_DEPTH_B];
        int prev;
        int next;

        double sweepline_pos;
        double range_end;

        BeachArc<O> m_beachArc;
};

/* 
    This class manages the beachline for voronoi tessellation.
*/

template <Order O>
class BeachLine
{
    public:

        BeachLine();
        ~BeachLine();

        int getSize();

        void findAndInsert(SkipNode<O>* node, SkipNode<O>* node2, double sweepline, uint8_t threadId);
        void insert1(SkipNode<O>* node);
        void insert2(SkipNode<O>* node);
        void erase(SkipNode<O>* node, uint8_t threadId);

    private:

        SkipNode<O>* linked_list;

        int size;

        bool isRangeEndGreater(SkipNode<O>* next, SkipNode<O>* curr, SweepLine & sl, double shift, int skipLevel);
        
        void insertAfter(SkipNode<O>* node, SkipNode<O>* at);

        void addSkips(SkipNode<O>* node, SkipNode<O>** previous, bool repeat_first);
        void removeSkips(SkipNode<O>* node);

        // for randomly determining the number of skip levels to add
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution;
};

#endif