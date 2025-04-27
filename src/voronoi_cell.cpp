#include "voronoi_cell.h"
#include "../glm/gtc/matrix_transform.hpp"
#include "globals.h"
#include <algorithm>


void VoronoiCell::addCorner(const glm::dvec3 & c, uint8_t thread)
{
    uint8_t prev = m_owner.fetch_or(thread);
    if (prev & thread || prev == 0)
        corners.push_back(c); // already owned by thread or previously not owned
    else
        m_owner.fetch_and(~thread); // revoke ownership
}

void VoronoiCell::increment(uint8_t thread)
{
    uint8_t prev = m_owner.fetch_or(thread);
    if (prev & thread || prev == 0)
        m_arcs++; // already owned by thread or previously not owned
    else
        m_owner.fetch_and(~thread); // revoke ownership
}

void VoronoiCell::decrement(uint8_t thread)
{
    uint8_t prev = m_owner.fetch_or(thread);
    if (prev & thread || prev == 0)
    {
        m_arcs--; // already owned by thread or previously not owned
        if (m_arcs == 0)
        {
            completedCells++;
        }
    }
    else
        m_owner.fetch_and(~thread); // revoke ownership
}

// VoronoiCell implementations
VoronoiCell::VoronoiCell() {}

VoronoiCell::VoronoiCell(const glm::dvec3 & p)
{
    m_arcs = 0;
    position = p;
    corners.reserve(8);
    m_owner.store(0);
}

void VoronoiCell::sortCorners()
{
    struct VecAngle
    {
        glm::dvec3 vec;
        double angle;
    };

    std::vector<VecAngle> pairs(corners.size());

    glm::dvec3 pivnorm = glm::normalize(corners[0] - position);

    for (unsigned int i = 0; i < pairs.size(); i++)
    {
        glm::dvec3 pnormA = glm::normalize(corners[i] - position);
        double x = (double)glm::dot(glm::cross(pivnorm, pnormA), position);
        double y = (double)glm::dot(pivnorm, pnormA);
        pairs[i].vec = corners[i];
        pairs[i].angle = atan2(y, x);
    }

    std::sort(pairs.begin(), pairs.end(), 
        [](const VecAngle & a, const VecAngle & b) -> bool
        {
            return a.angle > b.angle;
        }
    );

    for (unsigned int i = 0; i < corners.size(); i++)
    {
        corners[i] = pairs[i].vec;
    }
}

void VoronoiCell::computeCentroid()
{
    // construct transformation and inverse
    glm::dmat4x4 transform = glm::lookAt(glm::dvec3(0.0),
                                         position,
                                         glm::dvec3(0.0,1.0,0.0));
    glm::dmat4x4 inverse = glm::inverse(transform);

    double cx, cy, cz, area;
    cx = cy = area = 0.0;

    glm::dvec4 pc4 = glm::dvec4(corners[corners.size()-1], 1.0);

    glm::dvec4 prev_t_corner = transform * pc4;
    cz = prev_t_corner.z;

    for (auto it = corners.begin(); it != corners.end(); it++)
    {
        glm::dvec3 corner = *it;
        glm::dvec4 c4 = glm::dvec4(corner, 1.0);
        glm::dvec4 t_corner = transform * c4;

        double a = (prev_t_corner.x * t_corner.y) 
                    - (t_corner.x * prev_t_corner.y);
        cx += (prev_t_corner.x + t_corner.x) * a;
        cy += (prev_t_corner.y + t_corner.y) * a;
        area += a;

        prev_t_corner = t_corner;
    }

    double f = 1.0/(3.0 * area);
    position = glm::vec3(inverse * glm::dvec4(f*cx, f*cy, cz, 1.0));
}
