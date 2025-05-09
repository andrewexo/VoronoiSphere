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
#include "../glm/gtc/matrix_transform.hpp"

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
    int runs = 10;
    for (int w = 0; w < runs; w++)
    {
        VoronoiGenerator vg;
        size_t count = 2'000'000;
        glm::dvec3* points = vg.genRandomInput(count);
        glm::dvec3* points_in_radius = new glm::dvec3[count];
        glm::dvec3 origin = glm::normalize(glm::dvec3(1.0, 1.0, 1.0));

        // copy points within radius of origin
        size_t j = 0;
        for (size_t i = 0; i < count; i++)
        {
            if (glm::dot(points[i], origin) >= 0.977)
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

TEST(VoronoiTests, TestCapDeterminism)
{
    VoronoiGenerator vg1;
    VoronoiGenerator vg2;
    size_t count = 500000;
    glm::dvec3* points = vg1.genRandomInput(count);

    ::std::vector<glm::dvec3> cap_points;
    glm::dvec3 nPos = glm::normalize(glm::dvec3(0,0,-1));
    for (uint i = 0; i < count; i++)
    {
        if (glm::dot(points[i], nPos) > 0.99)
            cap_points.push_back(points[i]);
    }

    VoronoiCell* cells1 = vg1.generateCap(glm::dvec3(0,0,-1), cap_points.data(), cap_points.size());
    VoronoiCell* cells2 = vg2.generateCap(glm::dvec3(0,0,-1), cap_points.data(), cap_points.size());

    EXPECT_EQ(vg1.m_size, vg2.m_size);
    uint error_count = 0;
    for (uint i = 0; i < vg1.m_size; i++)
    {
        if (vg1.m_sitesX[i].m_position.x != vg2.m_sitesX[i].m_position.x)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_position.y != vg2.m_sitesX[i].m_position.y)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_position.z != vg2.m_sitesX[i].m_position.z)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_polar !=      vg2.m_sitesX[i].m_polar)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_azimuth !=    vg2.m_sitesX[i].m_azimuth)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_polSin !=     vg2.m_sitesX[i].m_polSin)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_polCos !=     vg2.m_sitesX[i].m_polCos)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_aziSinPS !=   vg2.m_sitesX[i].m_aziSinPS)
        { error_count++; std::cout << i << std::endl; continue; }
        if (vg1.m_sitesX[i].m_aziCosPS !=   vg2.m_sitesX[i].m_aziCosPS)
        { error_count++; std::cout << i << std::endl; continue; }
    }
    std::cout << "error count: " << error_count << std::endl;
    EXPECT_EQ((uint)0, error_count);
    delete[] cells1;
    delete[] cells2;
    delete[] points;
}
TEST(VoronoiTests, SortPointsTest)
{
    ::boost::timer::cpu_timer total;
    size_t runs = 20;
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
        
        auto p_tempsX1 = new promise<VoronoiSite*>;
        auto p_tempsX2 = new promise<VoronoiSite*>;
        auto p_doneX1 = new promise<bool>;
        auto p_doneX2 = new promise<bool>;

        auto addTask = [&](auto task, auto && td)
        {
            task->td = std::move(td);
            taskGraph.addTask(std::unique_ptr<Task>(task));
        };

        addTask(new SortPoints1Task, TaskDataDualSort{&sites, std::unique_ptr<promise<VoronoiSite*>>(p_tempsX1),
                                                              std::unique_ptr<promise<bool>>(p_doneX1),
                                                              (p_tempsX2)->get_future(), (p_doneX2)->get_future()});

        addTask(new SortPoints2Task, TaskDataDualSort{&sites, std::unique_ptr<promise<VoronoiSite*>>(p_tempsX2),
                                                              std::unique_ptr<promise<bool>>(p_doneX2),
                                                              (p_tempsX1)->get_future(), (p_doneX1)->get_future()});
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
    size_t runs = 20;
    for (size_t i = 0; i < runs; i++)
    {
        VoronoiGenerator vg;
        ::std::vector<VoronoiSite> sites;
        size_t count = 1000000+(i%2);
        sites.reserve(count);
        glm::dvec3* points = vg.genRandomInput(count);
        for (size_t j = 0; j < count; j++)
        {
            sites.push_back(VoronoiSite(points[j], NULL));
            computePolarAndAzimuth<X>(sites.back());
        }
        delete[] points;
        TaskGraph taskGraph;
        
        auto p_temps1 = new promise<vector<vector<VoronoiSite>>*>;
        auto p_temps2 = new promise<vector<vector<VoronoiSite>>*>;
        auto p_done1 = new promise<bool>;
        auto p_done2 = new promise<bool>;

        auto addTask = [&](auto task, auto && td)
        {
            task->td = std::move(td);
            taskGraph.addTask(std::unique_ptr<Task>(task));
        };

        addTask(new BucketSort1Task, TaskDataBucketDualSort{&sites, std::unique_ptr<promise<vector<vector<VoronoiSite>>*>>(p_temps1),
                                                                    std::unique_ptr<promise<bool>>(p_done1),
                                                                    (p_temps2)->get_future(), (p_done2)->get_future()});
        addTask(new BucketSort2Task, TaskDataBucketDualSort{&sites, std::unique_ptr<promise<vector<vector<VoronoiSite>>*>>(p_temps2),
                                                                    std::unique_ptr<promise<bool>>(p_done2),
                                                                    (p_temps1)->get_future(), (p_done1)->get_future()});
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

TEST(VoronoiTests, RotatePointsTaskDeterminism)
{
    const int runs = 5;
    const size_t count = 1'000'000;
    
    for (int run = 0; run < runs; run++)
    {
        // Create consistent set of points for each run
        std::vector<::glm::dvec3> points1(count);
        std::vector<::glm::dvec3> points2(count);
        
        // Initialize with same random points
        srand(42); // Fixed seed for determinism
        for (size_t i = 0; i < count; i++)
        {
            ::glm::dvec3 p = ::glm::dvec3(
                static_cast<double>(rand()) / RAND_MAX,
                static_cast<double>(rand()) / RAND_MAX,
                static_cast<double>(rand()) / RAND_MAX
            );
            p = ::glm::normalize(p);
            points1[i] = p;
            points2[i] = p;
        }
        
        // Create rotation matrix
        ::glm::dvec3 origin(0.5, 0.7, 0.3);
        origin = ::glm::normalize(origin);
        ::glm::dvec3 x_axis(1, 0, 0);
        ::glm::dvec3 rotation_axis = ::glm::cross(x_axis, origin);
        double angle = ::glm::acos(::glm::dot(x_axis, origin));
        ::glm::dmat4 rotation = ::glm::rotate(::glm::dmat4(1.0), angle, rotation_axis);
        
        auto RotatePoints = [&](std::vector<::glm::dvec3>& points) {
            RotatePointsTask* task = new RotatePointsTask;
            task->td = TaskDataRotatePoints{points.data(), 0, count - 1, rotation};
            task->process();
            delete task;
        };
        
        RotatePoints(points1);
        RotatePoints(points2);
        
        // Verify that both rotations produced identical results
        bool identical = true;
        for (size_t i = 0; i < count; i++)
        {
            if (::glm::distance(points1[i], points2[i]) != 0.0)
            {
                identical = false;
                std::cout << "Difference at index " << i << ": " 
                          << ::glm::distance(points1[i], points2[i]) << std::endl;
                break;
            }
        }
        
        EXPECT_TRUE(identical) << "RotatePointsTask is not deterministic";
    }
}


}
