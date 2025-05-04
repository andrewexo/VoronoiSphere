#define GLM_SWIZZLE
#include "../glm/glm.hpp"
#include "voronoi_tasks.h"
#include "voronoi_event_compare.h"
#include "voronoi.h"
#include <algorithm>
#include <cstring>

namespace VorGen {

void RotatePointsTask::process()
{
    for (unsigned int i = td.start; i <= td.end; i++)
        td.points[i] = (td.rotation * glm::dvec4(td.points[i], 1.0)).xyz();
}

void InitCellsTask::process()
{
    for (unsigned int i = td.start; i <= td.end; i++)
        new(td.cells + i) VoronoiCell(td.points[i]);
}

void InitCellsAndResizeSitesTask::process()
{
    for (unsigned int i = td.start; i <= td.end; i++)
        new(td.cells + i) VoronoiCell(td.points[i]);

    td.sites->resize(td.size);
}

template<Axis A>
void InitSitesTask<A>::process()
{
    for (unsigned int i = td.start; i <= td.end; i++)
    {
        VoronoiSite site{(td.cells)[i].position, td.cells + i};
        computePolarAndAzimuth<A>(site);
        (*(td.sites))[i] = site;
    }
}

template class InitSitesTask<X>;
template class InitSitesTask<Y>;
template class InitSitesTask<Z>;

void SortPoints1Task::process()
{
    // sort array half
    unsigned int size = (unsigned int)td.sites->size() / 2;
    VoronoiSiteCompare voronoiSiteCompare;
    ::std::sort(td.sites->begin(), td.sites->begin() + size, voronoiSiteCompare);

    // copy into scratch array
    VoronoiSite* scratch = (VoronoiSite*)new char[size * sizeof(VoronoiSite)];
    memcpy(scratch, td.sites->data(), size * sizeof(VoronoiSite));

    // send data to other thread
    td.p_temps->set_value(scratch);
    VoronoiSite* scratch2 = td.f_temps.get();

    // merge into original array
    int a = 0; int b = 0;
    for (unsigned int i = 0; i < size; i++)
    {
        if (voronoiSiteCompare(scratch[a], scratch2[b]))
            (*td.sites)[i] = scratch[a++];
        else
            (*td.sites)[i] = scratch2[b++];
    }

    // wait for other thread
    td.p_done->set_value(true);
    bool ready = td.f_done.get();

    // cleanup temp memory
    delete[] scratch;
    delete[] td.p_temps;
    delete[] td.p_done;
}

void SortPoints2Task::process()
{
    // sort array half
    unsigned int size1 = (unsigned int)td.sites->size() / 2;
    unsigned int size = (unsigned int)td.sites->size() - size1;
    VoronoiSiteCompare voronoiSiteCompare;
    ::std::sort(td.sites->begin() + size1, td.sites->end(), voronoiSiteCompare);

    // copy into scratch array
    VoronoiSite* scratch = (VoronoiSite*)new char[size * sizeof(VoronoiSite)];
    memcpy(scratch, td.sites->data() + size1, size * sizeof(VoronoiSite));

    // send data to other thread
    td.p_temps->set_value(scratch);
    VoronoiSite* scratch1 = td.f_temps.get();

    // merge into original array
    int a = size1 - 1; int b = size - 1;
    for (unsigned int i = (unsigned int)td.sites->size() - 1; i >= size1; i--)
    {
        if (a < 0 || voronoiSiteCompare(scratch1[a], scratch[b]))
            (*td.sites)[i] = scratch[b--];
        else
            (*td.sites)[i] = scratch1[a--];
    }

    // wait for other thread
    td.p_done->set_value(true);
    bool ready = td.f_done.get();

    // delete scratch
    delete[] scratch;
}

template<Order O, Axis A>
inline void SweepTask<O, A>::process()
{
#ifdef ENABLE_SWEEP_TIMERS
    boost::timer::cpu_timer timer;
#endif
    
    VoronoiSweeper<O, A> voronoiSweeper(td.sites, td.gen, td.taskId);
    voronoiSweeper.sweep();
    
#ifdef ENABLE_SWEEP_TIMERS
    ::std::string orderStr = (O == Increasing) ? "Increasing" : "Decreasing";
    ::std::string axisStr;
    if (A == X) axisStr = "X";
    else if (A == Y) axisStr = "Y";
    else axisStr = "Z";
    
    ::std::lock_guard<::std::mutex> lock(cout_mutex);
    ::std::cout << "SweepTask<" << orderStr << ", " << axisStr << "> process time: " << timer.format() << ::std::endl;
#endif
}

template class SweepTask<Increasing, X>;
template class SweepTask<Increasing, Y>;
template class SweepTask<Increasing, Z>;
template class SweepTask<Decreasing, X>;
template class SweepTask<Decreasing, Y>;
template class SweepTask<Decreasing, Z>;

void SortCellCornersTask::process()
{
    for (unsigned int i = td.start; i <= td.end; i++)
    {
        if (td.cell_vector[i].corners.size() == 0)
            continue;
        (td.cell_vector[i]).sortCorners();
#ifdef CENTROID
        (td.cell_vector[i]).computeCentroid();
#endif
    }
}

void SortCornersRotateTask::process()
{
    for (unsigned int i = td.start; i <= td.end; i++)
    {
        (td.cell_vector[i]).sortCorners();
        for (unsigned int j = 0; j < td.cell_vector[i].corners.size(); j++)
        {
            td.cell_vector[i].corners[j] = (td.rotation * glm::dvec4(td.cell_vector[i].corners[j], 1.0)).xyz();
        }
#ifdef CENTROID
        (td.cell_vector[i]).computeCentroid();
#endif
        td.cell_vector[i].position = (td.rotation * glm::dvec4(td.cell_vector[i].position, 1.0)).xyz();
    }
}

}