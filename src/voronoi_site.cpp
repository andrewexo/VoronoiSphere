#include "voronoi_site.h"

#define _USE_MATH_DEFINES
#include <math.h>

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

VoronoiSite::VoronoiSite(
    const glm::dvec3 & p, 
    VoronoiCell* cell, 
    const glm::dvec3 & origin,
    const glm::dvec3 & originY,
    const glm::dvec3 & originZ)
{
    m_position = p;
    m_cell = cell;

    m_polCos = -glm::dot(p, origin);
    m_polar = acos(m_polCos);
    m_azimuth = atan2(glm::dot(originZ,p), glm::dot(originY,p));

    m_azimuth /= PI2;
    m_azimuth -= floor(m_azimuth);
    m_azimuth *= PI2;

    m_polSin = sin(m_polar);
    m_aziCosPS = cos(m_azimuth) * m_polSin;
    m_aziSinPS = sin(m_azimuth) * m_polSin;
}