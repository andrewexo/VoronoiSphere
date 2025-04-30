#ifndef VORONOI_SITE_H
#define VORONOI_SITE_H

#include "../glm/glm.hpp"
#include "voronoi_cell.h"
#include "globals.h"

namespace VorGen {

enum Axis {X,Y,Z};

class VoronoiSite
{
public:

  VoronoiSite();
  VoronoiSite(
    const glm::dvec3 & p, 
    VoronoiCell* cell, 
    Axis a);

  glm::dvec3 m_position;
  double m_azimuth, m_polar;

  double m_polSin, m_polCos;
  double m_aziSinPS, m_aziCosPS;

  VoronoiCell* m_cell;
};

}

#endif