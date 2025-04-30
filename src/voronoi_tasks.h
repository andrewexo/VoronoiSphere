#ifndef VORONOI_TASKS_H
#define VORONOI_TASKS_H

#include "voronoi_generator.h"
#include "voronoi_site.h"
#include "voronoi_cell.h"
#include "task_graph.h"
#include <future>
#include <vector>
namespace VorGen {

struct TaskDataRotatePoints
{
    glm::dvec3* points;
    unsigned int start;
    unsigned int end;
    glm::dmat4 rotation;
};

struct TaskDataCells
{
    VoronoiCell* cells;
    glm::dvec3* points;
    unsigned int start;
    unsigned int end;
};

struct TaskDataCellsResize
{
    VoronoiCell* cells;
    glm::dvec3* points;
    unsigned int start;
    unsigned int end;
    ::std::vector<VoronoiSite>* sites;
    unsigned int size;
};

struct TaskDataSites
{
    VoronoiCell* cells;
    unsigned int start;
    unsigned int end;
    ::std::vector<VoronoiSite>* sites;
};

struct TaskDataSitesCap
{
    VoronoiCell* cells;
    unsigned int start;
    unsigned int end;
    ::std::vector<VoronoiSite>* sites;
};

struct TaskDataDualSort
{
    ::std::vector<VoronoiSite>* sites;
    ::std::promise<VoronoiSite*>* p_temps;
    ::std::promise<bool>* p_done;
    ::std::future<VoronoiSite*> f_temps;
    ::std::future<bool> f_done;
};

struct TaskDataSweep
{
    ::std::vector<VoronoiSite>* sites;
    unsigned int gen;
    uint8_t taskId;
};

struct TaskDataSortCorners
{
    VoronoiCell* cell_vector;
    unsigned int start;
    unsigned int end;
};

struct TaskDataRotateCorners
{
    VoronoiCell* cell_vector;
    unsigned int start;
    unsigned int end;
    glm::dmat4 rotation;
};

class RotatePointsTask : public Task
{
    public:
        void process();
        TaskDataRotatePoints td;
};

class InitCellsTask : public Task
{
    public:
        void process();
        TaskDataCells td;
};

class InitCellsAndResizeSitesTask : public Task
{
    public:
        void process();
        TaskDataCellsResize td;
};

template <Axis A>
class InitSitesTask : public Task
{
    public:
        void process();
        TaskDataSites td;
};

class InitSitesCapTask : public Task
{
    public:
        void process();
        TaskDataSites td;
};

class SortPoints1Task : public Task
{
    public:
        void process();
        TaskDataDualSort td;
};

class SortPoints2Task : public Task
{
    public:
        void process();
        TaskDataDualSort td;
};

template <Order O, Axis A>
class SweepTask : public Task
{
    public:
        void process();
        TaskDataSweep td;
};

class SortCellCornersTask : public Task
{
    public:
        void process();
        TaskDataSortCorners td;
};

class SortCornersRotateTask : public Task
{
    public:
        void process();
        TaskDataRotateCorners td;
};

}

#endif
