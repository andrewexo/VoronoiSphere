#ifndef VORONOI_GENERATOR_H
#define VORONOI_GENERATOR_H

#include "mp_sample_generator.h"
#include "voronoi_cell.h"
#include "voronoi_event_compare.h"
#include "task_graph.h"
#include <vector>
#include "gtest/gtest_prod.h"

namespace VorGen {

//#define CENTROID

class VoronoiGenerator
{
    public:

        VoronoiGenerator();
        VoronoiGenerator(unsigned int seed);
        ~VoronoiGenerator();

        glm::dvec3* genRandomInput(int count);
        void generate(glm::dvec3* points, int count, int gen, bool writeToFile);
        VoronoiCell* generateCap(const glm::dvec3& origin, glm::dvec3* points, int count);
        void clear();

    private:

        SampleGenerator sample_generator;
        VoronoiCell* cell_vector;

        // max number of cells to generate
        unsigned int m_size;

        // number of cells to generate
        // use if you want part of the sphere 
        // to be generated
		unsigned int m_gen;

        ::std::vector<VoronoiSite> m_sitesX;
        ::std::vector<VoronoiSite> m_sitesY;
        ::std::vector<VoronoiSite> m_sitesZ;

        void writeDataToFile();
        void writeDataToOBJ();
        inline void writeCell(::std::ofstream & os, int i);
        inline void writeCellOBJ(::std::ofstream & os, int i);

        void buildTaskGraph(TaskGraph* tg, glm::dvec3* points);
        void buildCapTaskGraph(TaskGraph* tg, const glm::dvec3& origin, glm::dvec3* points);
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
        inline void generateSortCellCornersTasks(TaskGraph* tg, SyncTask* syncIn, size_t threads);

        inline void generateCapInitCellsTasks(TaskGraph* tg, glm::dvec3* points, SyncTask* & syncOut);
        inline void generateCapInitSitesTasks(TaskGraph* tg, SyncTask* & syncInOut, const glm::dvec3 & origin);
        inline void generateCapSortPointsTasks(TaskGraph* tg, SyncTask* & syncInOut);
        inline void generateCapSweepTasks(TaskGraph* tg, SyncTask* & syncInOut);

        // tests
        FRIEND_TEST(VoronoiTests, TestIntersect);
        FRIEND_TEST(VoronoiTests, TestIntersectDegenerateParabola);
        FRIEND_TEST(VoronoiTests, TestVerifyResult);
        FRIEND_TEST(VoronoiTests, TestBeachLine);
        FRIEND_TEST(VoronoiTests, TestCircumcenter);
};

}

#endif
