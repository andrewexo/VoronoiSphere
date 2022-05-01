#include "voronoi.h"
#include "globals.h"
#include "../glm/glm.hpp"

template <Order O, Axis A>
VoronoiSweeper<O, A>
::~VoronoiSweeper()
{
	free(m_memBlocks);
}

template <Order O, Axis A>
inline SkipNode<O>* VoronoiSweeper<O, A>
::initBlock()
{
	new(&(m_nextBlock->priQueueNode)) PriQueueNode<O>(block);
	new(&(m_nextBlock->skipNode)) SkipNode<O>(block);

	block++;
	return &((m_nextBlock++)->skipNode);
}

template <Order O, Axis A>
void VoronoiSweeper<O,A>::sweep()
{
	processEvents();
}

template <Order O, Axis A>
void VoronoiSweeper<O,A>
::processSiteEvent(VoronoiSite* site)
{
	SkipNode<O>* node = initBlock(); node->initSite(site, m_threadId);
	SkipNode<O>* node2 = initBlock();
		
	m_beachLine.findAndInsert(node, node2, site->m_polar, m_threadId);

	removeCircleEvent(&(NODE(node, prev)->m_beachArc));
	addCircleEventProcessSite(NODE(node, prev));
	addCircleEventProcessSite(NODE(node, next));
}

// creates a voronoi vertex
template <Order O, Axis A>
void VoronoiSweeper<O,A>
::processCircleEvent(CircleEvent<O>* circle)
{
	m_sweeplineLarge = circle->polar;
	m_sweeplineSmall = circle->polar_small;

	SkipNode<O>* sn = getSkipNodeFromCircleEvent(circle);
	SkipNode<O>* sni = NODE(sn, prev);
	SkipNode<O>* snk = NODE(sn, next);

    // add vertex to cells
	glm::dvec3 dv = glm::normalize(circle->center);
	sni->m_beachArc.m_site->m_cell->addCorner(dv, m_threadId);
	sn->m_beachArc.m_site->m_cell->addCorner(dv, m_threadId);
	snk->m_beachArc.m_site->m_cell->addCorner(dv, m_threadId);

	// remove circle events of neighbors
	removeCircleEvent(&(sni->m_beachArc));
	removeCircleEvent(&(snk->m_beachArc));

	// remove site from beachline
	m_beachLine.erase(sn, m_threadId);

	// check for new circle events
	addCircleEventProcessCircle(sni);
	addCircleEventProcessCircle(snk);

	getPriQueueNodeFromCircleEvent(circle)->clear();
}

template <Order O, Axis A>
inline void VoronoiSweeper<O,A>
::addCircleEvent(
	SkipNode<O>* node, 
	double large_polar, 
	double small_polar, 
	const glm::dvec3 & cc)
{
  new(getCircleEventFromSkipNode(node)) CircleEvent<O>(
		large_polar, small_polar, cc);

	node->m_beachArc.m_eventValid = true;
  m_circles.push(getPriQueueNodeFromSkipNode(node));
}

template <Order O, Axis A>
void VoronoiSweeper<O,A>
::removeCircleEvent(BeachArc<O>* arc)
{
	if (arc->m_eventValid)
	{
		m_circles.erase(getPriQueueNodeFromBeachArc(arc));
		getPriQueueNodeFromBeachArc(arc)->clear();
		arc->m_eventValid = false;
	}
}

#define SWEEP_AXIS X
#define COMPONENT x
#include "sweeper_templates.cpp"
#undef SWEEP_AXIS
#undef COMPONENT

#define SWEEP_AXIS Y
#define COMPONENT y
#include "sweeper_templates.cpp"
#undef SWEEP_AXIS
#undef COMPONENT

#define SWEEP_AXIS Z
#define COMPONENT z
#include "sweeper_templates.cpp"