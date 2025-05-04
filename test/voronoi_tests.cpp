#include "../src/voronoi.h"
#include "../src/voronoi_generator.h"
#include "../src/beachline.h"
#include "../src/voronoi_event.h"
#include "../src/voronoi_cell.h"
#include "../src/voronoi_event_compare.h"
#include "../glm/glm.hpp"
#include "../src/platform.h"
#include <iostream>
#include <vector>
#include <future>
#include "../src/task_graph.h"
#include "../src/voronoi_tasks.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "gtest/gtest.h"
#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>

namespace VorGen {

TEST(VoronoiTests, TestIntersect)
{
    glm::dvec3 p1 = glm::normalize(glm::dvec3(1.0, 1.0, 0.0));
    glm::dvec3 p2 = glm::normalize(glm::dvec3(-1.0, 1.0, 0.0));
    glm::dvec3 p3 = glm::normalize(glm::dvec3(-1.0, -1.0, 0.0));
    glm::dvec3 p4 = glm::normalize(glm::dvec3(1.0, -1.0, 0.0));

    SkipNode<Increasing> sn = SkipNode<Increasing>{0};
    
    SweepLine sl;
    sl.m_polar = 3.0;
    sl.m_polCos = cos(sl.m_polar);
    sl.m_polSin = sin(sl.m_polar);

    VoronoiSite s1 = VoronoiSite(p1, NULL);
    computePolarAndAzimuth<Z>(s1);
    VoronoiSite s2 = VoronoiSite(p2, NULL);
    computePolarAndAzimuth<Z>(s2);
    VoronoiSite s3 = VoronoiSite(p3, NULL);
    computePolarAndAzimuth<Z>(s3);
    VoronoiSite s4 = VoronoiSite(p4, NULL);
    computePolarAndAzimuth<Z>(s4);

    ALIGN(16) double results[2];
    sn.intersect2(&s1, &s2, &s3, &s4, sl, 0.0, results);

    EXPECT_DOUBLE_EQ(results[1], atan2(1.0, 0.0));
    EXPECT_DOUBLE_EQ(results[0], atan2(-1.0, 0.0) + 2*M_PI);
}

TEST(VoronoiTests, TestIntersectDegenerateParabola)
{
    glm::dvec3 p1 = glm::normalize(glm::dvec3(1.0,1.0,0.5f));
    glm::dvec3 p2 = glm::normalize(glm::dvec3(1.0,1.0,0.0));

    SkipNode<Increasing> sn = SkipNode<Increasing>{0};

    SweepLine sl;
    sl.m_polar = acos(p2.z);
    sl.m_polCos = p2.z;
    sl.m_polSin = sin(sl.m_polar);

    VoronoiSite s1 = VoronoiSite(p1, NULL);
    computePolarAndAzimuth<Z>(s1);
    VoronoiSite s2 = VoronoiSite(p2, NULL);
    computePolarAndAzimuth<Z>(s2);

    ALIGN(16) double results[2];
    sn.intersect2(&s1, &s2, &s2, &s1, sl, 0.0, results);

    EXPECT_DOUBLE_EQ(results[0], results[1]);
    EXPECT_DOUBLE_EQ(results[1], s2.m_azimuth);
}

TEST(VoronoiTests, TestVerifyResult)
{
    for (int w = 0; w < 12; w++)
    {
        VoronoiGenerator vg;
        size_t count = (int)pow(10, ((w / 3) + 1));
        glm::dvec3* points = vg.genRandomInput(count);
        VoronoiCell* cells = vg.generate(points, count, count, false);
        delete[] points;

        // verify that each corner is closest to its origin point
        //unsigned int corner_sum = 0;
        unsigned int incorrect = 0;
        unsigned int corner_count_incorrect = 0;
        for (unsigned int i = 0; i < vg.m_size; ++i)
        {
            VoronoiCell* b = cells + i;
            //corner_sum += (unsigned int)b->corners.size();

            for (auto ct = b->corners.begin(); ct != b->corners.end(); ++ct)
            {
                glm::dvec3 c = *ct;
                c += (b->position - c) * 0.01;

                long double iclose = glm::dot(b->position, c);

                bool correct = true;
                for (unsigned int j = 0; j < vg.m_size; ++j)
                {
                    if (j != i)
                    {
                        VoronoiCell* b2 = cells + j;
                        long double jclose = glm::dot(b2->position, c);
                        if (jclose > iclose)
                            correct = false;
                    }
                }
                if (!correct)
                    incorrect++;
            }

            if (b->corners.size() < 3)
                corner_count_incorrect++;
        }
        delete[] cells;

        // assert correctness == 100%
        EXPECT_EQ((unsigned int)0, incorrect);
        EXPECT_EQ((unsigned int)0, corner_count_incorrect);
        EXPECT_GE(completedCells+2, count); // there may be 2 arcs on the beachline, but the vertex they converge to has been added
    }
}

TEST(VoronoiTests, TestCircumcenter)
{
    ::std::vector<VoronoiSite> sites;
    VoronoiSweeper<Increasing,X> vorI(&sites, 0, 0);
    glm::dvec3 p1 = glm::normalize(glm::dvec3( 0.0, 1.0, 0.5f));
    glm::dvec3 p2 = glm::normalize(glm::dvec3( 1.0, 0.0, 0.5f));
    glm::dvec3 p3 = glm::normalize(glm::dvec3( 0.0,-1.0, 0.5f));

    glm::dvec3 cc = vorI.circumcenter(p1,p2,p3);

    EXPECT_DOUBLE_EQ( 0.0, cc.x );
    EXPECT_DOUBLE_EQ( 0.0, cc.y );
    EXPECT_DOUBLE_EQ( 1.0, cc.z );

    VoronoiSweeper<Decreasing, X> vorD(&sites, 0, 0);
    cc = vorD.circumcenter(p1, p2, p3);

    EXPECT_DOUBLE_EQ( 0.0, cc.x );
    EXPECT_DOUBLE_EQ( 0.0, cc.y );
    EXPECT_DOUBLE_EQ(-1.0, cc.z );
}

TEST(VoronoiTests, TestCompare)
{
    CircleEvent<Increasing> ce3 = CircleEvent<Increasing>(3.0, 0.25, glm::dvec3(0.0, 0.0, 0.0));
    CircleEvent<Increasing> ce2 = CircleEvent<Increasing>(2.0, 0.25, glm::dvec3(0.0, 0.0, 0.0));

    CircleEvent<Decreasing> ce1 = CircleEvent<Decreasing>(1.0, 0.25, glm::dvec3(0.0, 0.0, 0.0));
    CircleEvent<Decreasing> ce4 = CircleEvent<Decreasing>(4.0, 1.70, glm::dvec3(0.0, 0.0, 0.0));

    CircleEvent<Increasing> ceX = CircleEvent<Increasing>(2.0, 0.5, glm::dvec3(0.0, 0.0, 0.0));
    CircleEvent<Decreasing> ceY = CircleEvent<Decreasing>(3.0, 0.5, glm::dvec3(0.0, 0.0, 0.0));

    VoronoiSite v1; v1.m_polar = 1.0;
    VoronoiSite v2; v2.m_polar = 2.5;

    VoronoiEventCompare<Increasing> vecI;
    VoronoiEventCompare<Decreasing> vecD;
    VoronoiSiteCompare vsc;
    VoronoiSiteEventCompare<Increasing> vsecI;
    VoronoiSiteEventCompare<Decreasing> vsecD;

    EXPECT_TRUE( vecI(&ce3,&ce2) );
    EXPECT_TRUE( vecD(&ce1,&ce4) );
    EXPECT_TRUE( vsc(v1,v2) );
    EXPECT_TRUE( vsecI(&v2,&ce2) );
    EXPECT_FALSE( vsecD(&v2,&ce1) );

    EXPECT_FALSE( vecI(&ce3, &ce3) );
    EXPECT_FALSE( vecD(&ce1, &ce1) );
    EXPECT_FALSE( vsc(v1, v1) );
    EXPECT_FALSE( vsecI(&v2, &ceX) );
    EXPECT_FALSE( vsecD(&v2, &ceY) );
}

TEST(VoronoiTests, TestPerformance)
{
    ::boost::timer::cpu_timer total;
    int runs = 16;
    for (int w = 0; w < runs; w++)
    {
        VoronoiGenerator vg;
        int count = 100000;
        glm::dvec3* points = vg.genRandomInput(count);

        total.resume();
        VoronoiCell* cells = vg.generate(points, count, count, false);
        total.stop();
        delete[] cells;

        delete[] points;
    }
    ::std::cout << (total.elapsed().wall / (runs * 1000000.f)) << "ms\n";
}

TEST(VoronoiTests, TestCapPerformance)
{
    ::boost::timer::cpu_timer total;
    int runs = 16;
    for (int w = 0; w < runs; w++)
    {
        VoronoiGenerator vg;
        size_t count = 100000;
        glm::dvec3* points = vg.genRandomInput(count);
        glm::dvec3* points_in_radius = new glm::dvec3[count];
        glm::dvec3 origin = glm::normalize(glm::dvec3(1.0, 1.0, 1.0));

        // copy points within radius of origin
        size_t j = 0;
        for (size_t i = 0; i < count; i++)
        {
            if (glm::dot(points[i], origin) >= 0.9)
                points_in_radius[j++] = points[i];
        }

        //::std::cout << "points in radius: " << j << ::std::endl;

        total.resume();
        VoronoiCell* cells = vg.generateCap(origin, points_in_radius, j);
        total.stop();
        delete[] cells;

        delete[] points;
        delete[] points_in_radius;
    }
    ::std::cout << (total.elapsed().wall / (runs * 1000000.f)) << "ms\n";
}

TEST(VoronoiTests, TestCapVerifyResult)
{
    for (int w = 0; w < 28; w++)
    {
        VoronoiGenerator vg;
        size_t count = (int)pow(10, ((w / 7) + 3));
        glm::dvec3* points = vg.genRandomInput(count);

        ::std::vector<glm::dvec3> cap_points;
        glm::dvec3 nPos = glm::normalize(glm::dvec3(0,0,-1));
        for (uint i = 0; i < count; i++)
        {
            if (glm::dot(points[i], nPos) > 0.99)
                cap_points.push_back(points[i]);
        }
        delete[] points;

        VoronoiCell* cells = vg.generateCap(glm::dvec3(1.0, 1.0, 0.5), cap_points.data(), cap_points.size());
        if (cells == NULL)
            continue;

        // verify that each corner is closest to its origin point
        unsigned int incorrect = 0;
        unsigned int corner_count_incorrect = 0;
        for (unsigned int i = 0; i < vg.m_size; ++i)
        {
            VoronoiCell* b = cells + i;

            for (auto ct = b->corners.begin(); ct != b->corners.end(); ++ct)
            {
                glm::dvec3 c = *ct;
                c += (b->position - c) * 0.01;

                long double iclose = glm::dot(b->position, c);

                bool correct = true;
                for (unsigned int j = 0; j < vg.m_size; ++j)
                {
                    if (j != i)
                    {
                        VoronoiCell* b2 = cells + j;
                        long double jclose = glm::dot(b2->position, c);
                        if (jclose > iclose)
                            correct = false;
                    }
                }
                if (!correct)
                    incorrect++;
            }

            if (b->corners.size() < 3)
                corner_count_incorrect++;
        }
        delete[] cells;

        // assert correctness == 100%
        EXPECT_EQ((unsigned int)0, incorrect);
        EXPECT_EQ((unsigned int)0, corner_count_incorrect);
        EXPECT_GE(completedCells+2, vg.m_size); // there may be 2 arcs on the beachline, but the vertex they converge to has been added
    }
}

TEST(VoronoiTests, SortPointsTest)
{
    ::boost::timer::cpu_timer total;
    size_t runs = 10;
    for (size_t i = 0; i < runs; i++)
    {
        ::std::vector<VoronoiSite> sites;
        size_t count = 1000000+(i%2);
        sites.reserve(count);
        for (size_t j = 0; j < count; j++)
        {
            glm::dvec3 p = glm::dvec3(static_cast<double>(rand()) / RAND_MAX, static_cast<double>(rand()) / RAND_MAX, static_cast<double>(rand()) / RAND_MAX);
            p = glm::normalize(p);
            sites.push_back(VoronoiSite(p, NULL));
            computePolarAndAzimuth<X>(sites.back());
        }

        TaskGraph taskGraph;
        
        auto p_temps = new ::std::promise<VoronoiSite*>[2];
        auto p_done = new ::std::promise<bool>[2];

        auto addTask = [&](auto task, std::vector<VoronoiSite>& sites, 
                        auto p_temps, auto p_done, 
                        auto p_temps2, auto p_done2)
        {
            task->td = { &sites, p_temps, p_done, p_temps2->get_future(), p_done2->get_future() };
            taskGraph.addTask(task);
        };

        addTask(new SortPoints1Task, sites, p_temps, p_done, p_temps+1, p_done+1);
        addTask(new SortPoints2Task, sites, p_temps+1, p_done+1, p_temps, p_done);
        taskGraph.finalizeGraph();

        total.resume();
        taskGraph.processTasks(2);
        total.stop();

        // verify that the sites are sorted
        bool sorted = true;
        for (size_t i = 0; i < sites.size()-1; i++)
        {
            if (sites[i].m_polar > sites[i+1].m_polar)
            {
                sorted = false;
                break;
            }
        }
        EXPECT_TRUE(sorted);
    }
    ::std::cout << (total.elapsed().wall / (runs * 1000000.f)) << "ms\n";
}

TEST(VoronoiTests, BucketSortTest)
{
    ::boost::timer::cpu_timer total;
    size_t runs = 10;
    for (size_t i = 0; i < runs; i++)
    {
        ::std::vector<VoronoiSite> sites;
        size_t count = 1000000+(i%2);
        sites.reserve(count);
        for (size_t j = 0; j < count; j++)
        {
            glm::dvec3 p = glm::dvec3(static_cast<double>(rand()) / RAND_MAX, static_cast<double>(rand()) / RAND_MAX, static_cast<double>(rand()) / RAND_MAX);
            p = glm::normalize(p);
            sites.push_back(VoronoiSite(p, NULL));
            computePolarAndAzimuth<X>(sites.back());
        }

        TaskGraph taskGraph;
        
        auto p_temps = new ::std::promise<vector<vector<VoronoiSite>>*>[2];
        auto p_done = new ::std::promise<bool>[2];

        auto addTask = [&](auto task, std::vector<VoronoiSite>& sites, 
                        auto p_temps, auto p_done, 
                        auto p_temps2, auto p_done2)
        {
            task->td = { &sites, p_temps, p_done, p_temps2->get_future(), p_done2->get_future() };
            taskGraph.addTask(task);
        };

        addTask(new BucketSort1Task, sites, p_temps, p_done, p_temps+1, p_done+1);
        addTask(new BucketSort2Task, sites, p_temps+1, p_done+1, p_temps, p_done);
        taskGraph.finalizeGraph();

        total.resume();
        taskGraph.processTasks(2);
        total.stop();

        // verify that the sites are sorted
        bool sorted = true;
        for (size_t i = 0; i < sites.size()-1; i++)
        {
            if (sites[i].m_polar > sites[i+1].m_polar)
            {
                sorted = false;
                break;
            }
        }
        EXPECT_TRUE(sorted);
    }
    ::std::cout << (total.elapsed().wall / (runs * 1000000.f)) << "ms\n";
}

}
