#include "voronoi.h"
#include "../glm/glm.hpp"

namespace VorGen {

template <>
glm::dvec3 VoronoiSweeper<Increasing,SWEEP_AXIS>
::circumcenter(
	const glm::dvec3 & i, 
	const glm::dvec3 & j, 
	const glm::dvec3 & k)
{
    return glm::normalize( glm::cross((i-j),(k-j)) );
}

template <>
glm::dvec3 VoronoiSweeper<Decreasing,SWEEP_AXIS>
::circumcenter(
	const glm::dvec3 & i, 
	const glm::dvec3 & j, 
	const glm::dvec3 & k)
{
	return glm::normalize( glm::cross((k-j),(i-j)) );
}

template <>
bool VoronoiSweeper<Increasing,SWEEP_AXIS>
::eventIsUpcoming(double small_polar, double large_polar)
{
	return (large_polar - m_sweeplineLarge) + 
		   (small_polar - m_sweeplineSmall) >= 0;
}

template <>
bool VoronoiSweeper<Decreasing,SWEEP_AXIS>
::eventIsUpcoming(double small_polar, double large_polar)
{
	return (large_polar - m_sweeplineLarge) - 
	  	   (small_polar - m_sweeplineSmall) <= 0;
}

template <>
inline bool VoronoiSweeper<Decreasing,SWEEP_AXIS>
::onOtherSide(const glm::dvec3 & cc)
{
	return cc[SWEEP_AXIS] > 0.0;
}

template <>
inline bool VoronoiSweeper<Increasing,SWEEP_AXIS>
::onOtherSide(const glm::dvec3 & cc)
{
	return cc[SWEEP_AXIS] < -0.0;
}

// Forward declare template types so compiler generates 
// code to link against
template class VoronoiSweeper<Increasing,SWEEP_AXIS>;
template class VoronoiSweeper<Decreasing,SWEEP_AXIS>;

}
