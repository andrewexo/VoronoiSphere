#pragma once

#define _USE_MATH_DEFINES

#include "../glm/glm.hpp"
#include "globals.h"
#include <vector>
#include <atomic>

namespace VorGen {

class VoronoiCell
{
    public:
        VoronoiCell();
        VoronoiCell(const glm::dvec3 & p);

        glm::dvec3 position;
        ::std::vector<glm::dvec3> corners;
        uint8_t m_arcs;	// probably enough bits!
        ::std::atomic<uint8_t> m_owner;

        void addCorner(const glm::dvec3 & c, uint8_t thread);
        void increment(uint8_t thread);
        void decrement(uint8_t thread);

        void sortCorners();
        void computeCentroid();
};

}