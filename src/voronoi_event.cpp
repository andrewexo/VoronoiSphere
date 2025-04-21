#include "voronoi_event.h"

#include <math.h>

template <Order O>
CircleEvent<O>::CircleEvent()
{
    pqn = nullptr;
}

template <Order O>
CircleEvent<O>::CircleEvent(double polar, double polar_small, const glm::dvec3 & c)
{
    this->polar = polar;
    this->polar_small = polar_small;
    center = c;
    pqn = nullptr;
}

template class CircleEvent<Increasing>;
template class CircleEvent<Decreasing>;