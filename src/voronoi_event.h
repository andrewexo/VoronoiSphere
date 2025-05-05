#pragma once

#define _USE_MATH_DEFINES

#include "globals.h"
#include "../glm/glm.hpp"
#include "voronoi_site.h"
#include "beachline.h"

namespace VorGen {

template <Order O>
class CircleEvent
{
    public:

        CircleEvent();
        CircleEvent(double polar, double polar_, const glm::dvec3 & c);

        double polar, polar_small;
        glm::dvec3 center;

        void* pqn; // pointer to node in priority queue
};

}