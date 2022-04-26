#ifndef VORONOI_EVENT_COMPARE_H
#define VORONOI_EVENT_COMPARE_H

#include "globals.h"
#include "voronoi_event.h"
#include "voronoi_site.h"

template <Order O> struct VoronoiEventCompare;

template <> struct VoronoiEventCompare<Increasing>
{
    inline bool operator()(CircleEvent<Increasing>* lhs, CircleEvent<Increasing>* rhs)	// returns true if lhs > rhs
    {
        double polarDiff = (lhs->polar - rhs->polar) + (lhs->polar_small - rhs->polar_small);
        if (polarDiff == 0.0)
            return false;
        else
            return (polarDiff > 0.0);
    }
};

template <> struct VoronoiEventCompare<Decreasing>
{
    inline bool operator()(CircleEvent<Decreasing>* lhs, CircleEvent<Decreasing>* rhs)	// returns true if rhs > lhs
    {
        double polarDiff = (rhs->polar - lhs->polar) - (rhs->polar_small - lhs->polar_small);
        if (polarDiff == 0.0)
            return false;
        else
            return (polarDiff > 0.0);
    }
};

struct VoronoiSiteCompare
{
    inline bool operator()(const VoronoiSite & lhs, const VoronoiSite & rhs)	// returns true if rhs > lhs
    {
        double polarDiff = rhs.m_polar - lhs.m_polar;
        if (polarDiff == 0.0)
            return false;
        else
            return (polarDiff > 0.0);
    }
};

template <Order O> struct VoronoiSiteEventCompare;

template <> struct VoronoiSiteEventCompare<Increasing>
{
    inline bool operator()(VoronoiSite* lhs, CircleEvent<Increasing>* rhs)	// returns true if lhs > rhs
    {
        double polarDiff = lhs->m_polar - (rhs->polar + rhs->polar_small);
        if (polarDiff == 0.0)
            return false;
        else
            return (polarDiff > 0.0);
    }
};

template <> struct VoronoiSiteEventCompare<Decreasing>
{
    inline bool operator()(VoronoiSite* lhs, CircleEvent<Decreasing>* rhs)	// returns true if lhs > rhs
    {
        double polarDiff = lhs->m_polar - (rhs->polar - rhs->polar_small);
        if (polarDiff == 0.0)
            return false;
        else
            return (polarDiff > 0.0);
    }
};

#endif