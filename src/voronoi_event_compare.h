#pragma once

#include "globals.h"
#include "voronoi_event.h"
#include "voronoi_site.h"

namespace VorGen {

template <Order O> struct VoronoiEventCompare;

template <> struct VoronoiEventCompare<Increasing>
{
    // returns true if lhs > rhs
    inline bool operator()(CircleEvent<Increasing>* lhs, CircleEvent<Increasing>* rhs)
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
    // returns true if rhs > lhs
    inline bool operator()(CircleEvent<Decreasing>* lhs, CircleEvent<Decreasing>* rhs)
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
    // returns true if rhs > lhs
    inline bool operator()(const VoronoiSite & lhs, const VoronoiSite & rhs)
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
    // returns true if lhs > rhs
    inline bool operator()(VoronoiSite* lhs, CircleEvent<Increasing>* rhs)
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
    // returns true if rhs > lhs
    inline bool operator()(VoronoiSite* lhs, CircleEvent<Decreasing>* rhs)
    {
        double polarDiff = lhs->m_polar - (rhs->polar - rhs->polar_small);
        if (polarDiff == 0.0)
            return false;
        else
            return (polarDiff < 0.0);
    }
};

}