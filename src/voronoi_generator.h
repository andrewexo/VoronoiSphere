#ifndef VORONOI_GENERATOR_H
#define VORONOI_GENERATOR_H

#include "mp_sample_generator.h"
#include "voronoi_cell.h"
#include "voronoi_event_compare.h"
#include "task_graph.h"
#include <vector>
#include <atomic>
#include <thread>

#include "gtest/gtest_prod.h"

#define CENTROID

class VoronoiGenerator
{
    public:

        VoronoiGenerator();
        VoronoiGenerator(unsigned int seed);
        ~VoronoiGenerator();

        glm::dvec3* genRandomInput(int count);
        void generate(glm::dvec3* points, int count, int gen, bool writeToFile);
        void clear();

    private:

        SampleGenerator sample_generator;
        VoronoiCell* cell_vector;
        unsigned int m_size;
		unsigned int m_gen;

        std::vector<VoronoiSite> m_sitesX;
        std::vector<VoronoiSite> m_sitesY;
        std::vector<VoronoiSite> m_sitesZ;

        void writeDataToFile();
        inline void writeCell(std::ofstream & os, int i);

        void buildTaskGraph(TaskGraph* tg, glm::dvec3* points);

        struct SyncXYZ
        {
            SyncTask* syncX;
            SyncTask* syncY;
            SyncTask* syncZ;
        };

        inline void generateInitCellsTasks(TaskGraph* tg, glm::dvec3* points, SyncTask* & syncOut);
        inline void generateInitSitesTasks(TaskGraph* tg, SyncTask* syncIn, SyncXYZ & syncOut);
        inline void generateSortPointsTasks(TaskGraph* tg, SyncXYZ & syncInOut);
        inline void generateSweepTasks(TaskGraph* tg, SyncXYZ & syncIn, SyncTask* & syncOut);
        inline void generateSortCellCornersTasks(TaskGraph* tg, SyncTask* syncIn);

        // tests
        FRIEND_TEST(VoronoiTests, TestIntersect);
        FRIEND_TEST(VoronoiTests, TestIntersectDegenerateParabola);
        FRIEND_TEST(VoronoiTests, TestVerifyResult);
        FRIEND_TEST(VoronoiTests, TestBeachLine);
        FRIEND_TEST(VoronoiTests, TestCircumcenter);
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
    std::vector<VoronoiSite>* sites;
    unsigned int size;
};

struct TaskDataSites
{
    VoronoiCell* cells;
    unsigned int start;
    unsigned int end;
    std::vector<VoronoiSite>* sites;
};

struct TaskDataDualSort
{
    std::vector<VoronoiSite>* sites;
    VoronoiSite** temps;
    bool* done;
};

struct TaskDataSweep
{
    std::vector<VoronoiSite>* sites;
	unsigned int gen;
    uint8_t taskId;
};

struct TaskDataSortCorners
{
    VoronoiCell* cell_vector;
    unsigned int start;
    unsigned int end;
};

class InitCellsTask : public Task
{
    public:
        ~InitCellsTask() {};
        void process();
        TaskDataCells td;
};

class InitCellsAndResizeSitesTask : public Task
{
    public:
        ~InitCellsAndResizeSitesTask() {};
        void process();
        TaskDataCellsResize td;
};

template <Axis A>
class InitSitesTask : public Task
{
    public:
        ~InitSitesTask() {};
        void process();
        TaskDataSites td;
};

class SortPoints1Task : public Task
{
    public:
        ~SortPoints1Task() {};
        void process();
        TaskDataDualSort td;
};

class SortPoints2Task : public Task
{
    public:
        ~SortPoints2Task() {};
        void process();
        TaskDataDualSort td;
};

template <Order O, Axis A>
class SweepTask : public Task
{
    public:
        ~SweepTask() {};
        void process();
        TaskDataSweep td;
};

class SortCellCornersTask : public Task
{
    public:
        ~SortCellCornersTask() {};
        void process();
        TaskDataSortCorners td;
};

#endif
