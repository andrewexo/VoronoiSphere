#pragma once

#include "voronoi_generator.h"
#include "voronoi_site.h"
#include "voronoi_cell.h"
#include "task_graph.h"
#include <future>
#include <vector>

namespace VorGen {

using ::std::vector;
using ::std::promise;
using ::std::future;

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
    vector<VoronoiSite>* sites;
    unsigned int size;
};

struct TaskDataSites
{
    VoronoiCell* cells;
    unsigned int start;
    unsigned int end;
    vector<VoronoiSite>* sites;
};

struct TaskDataSitesCap
{
    VoronoiCell* cells;
    unsigned int start;
    unsigned int end;
    vector<VoronoiSite>* sites;
};

struct TaskDataDualSort
{
    vector<VoronoiSite>* sites;
    promise<VoronoiSite*>* p_temps;
    promise<bool>* p_done;
    future<VoronoiSite*> f_temps;
    future<bool> f_done;
};

struct TaskDataBucketDualSort
{
    vector<VoronoiSite>* sites;
    promise<vector<vector<VoronoiSite>>*>* p_temps;
    promise<bool>* p_done;
    future<vector<vector<VoronoiSite>>*> f_temps;
    future<bool> f_done;
};

struct TaskDataSweep
{
    vector<VoronoiSite>* sites;
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

class BucketSort1Task : public Task
{
    public:
        void process();
        TaskDataBucketDualSort td;
};

class BucketSort2Task : public Task
{
    public:
        void process();
        TaskDataBucketDualSort td;
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
