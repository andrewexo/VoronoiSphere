#include "voronoi_site.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace VorGen {

constexpr double PI2 = 2.0 * M_PI;

VoronoiSite::VoronoiSite() {}

VoronoiSite::VoronoiSite(
    const glm::dvec3 & p, 
    VoronoiCell* cell) : m_position(p), m_cell(cell)
{
}

inline void computePolarAndAzimuthHelper(VoronoiSite& site)
{
    site.m_azimuth /= PI2;
    site.m_azimuth -= floor(site.m_azimuth);
    site.m_azimuth *= PI2;

    site.m_polSin = sin(site.m_polar);
    site.m_aziCosPS = cos(site.m_azimuth) * site.m_polSin;
    site.m_aziSinPS = sin(site.m_azimuth) * site.m_polSin;
}

template<>
void computePolarAndAzimuth<X>(VoronoiSite& site)
{
    site.m_polar = acos(site.m_position.x);
    site.m_azimuth = atan2(site.m_position.z, site.m_position.y);
    site.m_polCos = site.m_position.x;
    computePolarAndAzimuthHelper(site);
}

template<>
void computePolarAndAzimuth<Y>(VoronoiSite& site)
{
    site.m_polar = acos(site.m_position.y);
    site.m_azimuth = atan2(site.m_position.x, site.m_position.z);
    site.m_polCos = site.m_position.y;
    computePolarAndAzimuthHelper(site);
}

template<>
void computePolarAndAzimuth<Z>(VoronoiSite& site)
{
    site.m_polar = acos(site.m_position.z);
    site.m_azimuth = atan2(site.m_position.y, site.m_position.x);
    site.m_polCos = site.m_position.z;
    computePolarAndAzimuthHelper(site);
}

}