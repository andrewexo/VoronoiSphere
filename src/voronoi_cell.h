#ifndef VORONOI_CELL_H
#define VORONOI_CELL_H

#define _USE_MATH_DEFINES

#include "../glm/glm.hpp"
#include <vector>
#include <atomic>
#include <cmath>

class VoronoiCell
{
    public:

        VoronoiCell();
        VoronoiCell(const glm::dvec3 & p);

        glm::dvec3 position;

        std::vector<glm::dvec3> corners;

        std::atomic<uint8_t> m_owner;
        uint8_t m_arcs;	// probably enough bits!

        void addCorner(const glm::dvec3 & c, uint8_t thread);
        void increment(uint8_t thread);
        void decrement(uint8_t thread);
        void sortCorners();

        void computeCentroid();
};

#endif