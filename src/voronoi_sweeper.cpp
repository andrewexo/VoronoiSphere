#include "voronoi.h"
#include "globals.h"
#include "../glm/glm.hpp"

template<> double sweeplineStart<Increasing> = 0.0;
template<> double sweeplineStart<Decreasing> = M_PI;

template <>
inline glm::highp_float OrientedDvec3<X>::getComponent() const
{
	return vec.x;
}

template <>
inline glm::highp_float OrientedDvec3<Y>::getComponent() const
{
	return vec.y;
}

template <>
inline glm::highp_float OrientedDvec3<Z>::getComponent() const
{
	return vec.z;
}

template <>
OrderedIterator<Increasing>
::OrderedIterator(unsigned int maxSize) : maxSize(maxSize)
{
	index = 0;
}

template <>
OrderedIterator<Decreasing>
::OrderedIterator(unsigned int maxSize) : maxSize(maxSize)
{
	index = maxSize-1;
}

template <>
unsigned int OrderedIterator<Increasing>
::operator++(int)
{
	return index++;
}

template <>
unsigned int OrderedIterator<Decreasing>
::operator++(int)
{
	return index--;
}

template <Order O>
inline bool OrderedIterator<O>
::isInRange()
{
	return index >= 0 && index < maxSize;
}

template <>
inline bool OrderedIterator<Increasing>
::isAtEnd()
{
	return index == maxSize;
}

template <>
inline bool OrderedIterator<Decreasing>
::isAtEnd()
{
	return index < 0;
}

template <Order O, Axis A>
VoronoiSweeper<O, A>
::VoronoiSweeper(
	std::vector<VoronoiSite>* sites, 
	unsigned int gen, 
	uint8_t threadId
	) : m_sites(sites), 
	m_next(m_sites->size()),
	m_gen(gen), 
	m_threadId(threadId)
{
	m_sweeplineLarge = sweeplineStart<O>;
	unsigned int count = std::min((int)sites->size(), (int)(m_gen * 2));
	m_sweeplineSmall = 0.0;
	auto size = (2 * count - 2) * sizeof(MemBlock<O>);
	m_nextBlock = m_memBlocks = (MemBlock<O>*)malloc( size );
	block = 0;
}

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

template <Order O, Axis A>
void VoronoiSweeper<O,A>
::addCircleEventProcessSite(SkipNode<O>* node)
{
	OrientedDvec3<A> cc; cc = circumcenter(
		NODE(node, prev)->m_beachArc.m_site->m_position,
		node->m_beachArc.m_site->m_position,
		NODE(node, next)->m_beachArc.m_site->m_position);

	double small_polar = acos(
		glm::dot(cc.vec, node->m_beachArc.m_site->m_position));
	double large_polar = acos(cc.getComponent());

	addCircleEvent(node, large_polar, small_polar, cc.vec);
}

template <Order O, Axis A>
void VoronoiSweeper<O,A>
::addCircleEventProcessCircle(SkipNode<O>* node)
{
	OrientedDvec3<A> cc; cc = circumcenter(
		NODE(node, prev)->m_beachArc.m_site->m_position,
		node->m_beachArc.m_site->m_position,
		NODE(node, next)->m_beachArc.m_site->m_position);

	double small_polar = acos(
		glm::dot(cc.vec, node->m_beachArc.m_site->m_position));
	double large_polar = acos(cc.getComponent());

	if (eventIsUpcoming(small_polar, large_polar))
		addCircleEvent(node, large_polar, small_polar, cc.vec);
}

template <Order O, Axis A>
void VoronoiSweeper<O,A>
::processEvents()
{
	// process first two sites
	VoronoiSite* site = &(*m_sites)[m_next++];
	SkipNode<O>* node = initBlock();
	node->initSite(site, m_threadId);
	m_beachLine.insert1(node);

	site = &(*m_sites)[m_next++];
	node = initBlock();
	node->initSite(site, m_threadId);
	m_beachLine.insert2(node);

	// pop events from sites and circles in order of O(template Order) polar angle
	while ( completedCells < m_gen && 
					(m_next.isInRange() || !m_circles.empty()) )
	{
		if (m_circles.empty()) // No circle events so we process next site event
		{
			VoronoiSite* next_site = &(*m_sites)[m_next++];
			processSiteEvent(next_site);
		}
		else if (m_next.isAtEnd()) // No site events so we process next circle event
		{
			CircleEvent<O>* next_circle = m_circles.top();
			m_circles.pop();
			processCircleEvent(next_circle);
		}
		else // Get next site and circle events, then process whichever is closer
		{
			VoronoiSite* next_site = &(*m_sites)[m_next.getIndex()];
			CircleEvent<O>* next_circle = m_circles.top();

			if (voronoi_site_event_comp(next_site, next_circle))
			{
				m_circles.pop();
				processCircleEvent(next_circle);
			}
			else
			{
				m_next++;
				processSiteEvent(next_site);
			}
		}
	}
}

#define SWEEP_AXIS X
#include "sweeper_templates.cpp"
#undef SWEEP_AXIS

#define SWEEP_AXIS Y
#include "sweeper_templates.cpp"
#undef SWEEP_AXIS

#define SWEEP_AXIS Z
#include "sweeper_templates.cpp"