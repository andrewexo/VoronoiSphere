#include "voronoi_site.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace VorGen {

constexpr double PI2 = 2.0 * M_PI;

VoronoiSite::VoronoiSite() {}

VoronoiSite::VoronoiSite(
    const glm::dvec3 & p, 
    VoronoiCell* cell, 
    Axis a)
{
    m_position = p;
    m_cell = cell;

    if (a == X)
    {
        m_polar = acos(p.x);
        m_azimuth = atan2(p.z, p.y);
        m_polCos = p.x;
    }
    else if (a == Y)
    {
        m_polar = acos(p.y);
        m_azimuth = atan2(p.x, p.z);
        m_polCos = p.y;
    }
    else // a == Z
    {
        m_polar = acos(p.z);
        m_azimuth = atan2(p.y, p.x);
        m_polCos = p.z;
    }

    m_azimuth /= PI2;
    m_azimuth -= floor(m_azimuth);
    m_azimuth *= PI2;

    m_polSin = sin(m_polar);
    m_aziCosPS = cos(m_azimuth) * m_polSin;
    m_aziSinPS = sin(m_azimuth) * m_polSin;
}

}