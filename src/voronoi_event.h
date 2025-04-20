#ifndef VORONOI_EVENT_H
#define VORONOI_EVENT_H

#define _USE_MATH_DEFINES

#include <cmath>
#include <list>
#include "globals.h"
#include "../glm/glm.hpp"

template <Order O> class SkipNode;
template <Order O> class PriQueueNode;

template <Order O>
class CircleEvent
{
    public:

        CircleEvent(double polar, double polar_, const glm::dvec3 & c);

        double polar, polar_small;
        glm::dvec3 center;

        PriQueueNode<O>* pqn;
};

#endif