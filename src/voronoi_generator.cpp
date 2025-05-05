#include "voronoi_generator.h"
#include "voronoi_tasks.h"
#include "voronoi.h"
#include "globals.h"
#include <fstream>
#include <iostream>

#include "../glm/gtc/matrix_transform.hpp"

namespace VorGen {

// Define this macro to enable timing output for SweepTasks
//#define ENABLE_SWEEP_TIMERS

#ifdef ENABLE_SWEEP_TIMERS
#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>
// Global mutex for synchronizing console output
::std::mutex cout_mutex;
#endif

using ::std::promise;
using ::std::future;
using ::std::unique_ptr;
using ::std::move;

VoronoiGenerator::VoronoiGenerator()
{
    cell_vector = NULL;
}

VoronoiGenerator::VoronoiGenerator(size_t seed) : sample_generator(seed)
{
    cell_vector = NULL;
}

VoronoiGenerator::~VoronoiGenerator()
{
}

glm::dvec3 * VoronoiGenerator::genRandomInput(int count)
{
    return sample_generator.getRandomPointsSphere(count);
}

VoronoiCell* VoronoiGenerator::generate(glm::dvec3* points, int count, int gen, bool writeToFile)
{
    completedCells = 0;
    m_size = count;
    m_gen = gen;
    cell_vector = new VoronoiCell[count];

    TaskGraph taskGraph; buildTaskGraph(&taskGraph, points);
    taskGraph.processTasks(6);

    if (writeToFile) writeDataToOBJ();
    return cell_vector;
}

VoronoiCell* VoronoiGenerator::generateCap(const glm::dvec3& origin, glm::dvec3* points, int count)
{
    if (count < 3) return NULL;

    completedCells = 0;
    m_size = count;
    m_gen = count;
    cell_vector = new VoronoiCell[count];

    glm::dvec3* points_copy = new glm::dvec3[m_size];
    for (size_t i = 0; i < m_size; i++)
        points_copy[i] = points[i];

    TaskGraph taskGraph; buildCapTaskGraph(&taskGraph, origin, points_copy);
    taskGraph.processTasks(2);

    delete[] points_copy;
    
    return cell_vector;
}

void VoronoiGenerator::buildTaskGraph(TaskGraph* tg, glm::dvec3* points)
{
    SyncTask* sync;
    SyncXYZ syncXYZ;

    generateInitCellsTasks(tg, points, sync);
    generateInitSitesTasks(tg, sync, syncXYZ);
    generateSortPointsTasks(tg, syncXYZ);
    generateSweepTasks(tg, syncXYZ, sync);
    generateSortCellCornersTasks(tg, sync, 6);

    tg->finalizeGraph();
}

void VoronoiGenerator::buildCapTaskGraph(TaskGraph* tg, const glm::dvec3& origin, glm::dvec3* points)
{
    SyncTask* sync;

    glm::dvec3 x_axis(1,0,0);
    glm::dvec3 v_normalized = glm::normalize(origin);
    glm::dvec3 rotation_axis = glm::cross(x_axis, v_normalized);
    double angle = glm::acos(glm::dot(x_axis, v_normalized));
    glm::dmat4 rotation = glm::rotate(glm::dmat4(1.0), angle, rotation_axis);
    glm::dmat4 rotation_inv = glm::rotate(glm::dmat4(1.0), -angle, rotation_axis);

    generateRotatePointsTasks(tg, sync, rotation, points);
    generateCapInitCellsTasks(tg, points, sync);
    generateCapInitSitesTasks(tg, sync);
    generateCapSortPointsTasks(tg, sync);
    generateCapSweepTasks(tg, sync);
    generateCapSortCellCornersTasks(tg, sync, 2, rotation_inv);

    tg->finalizeGraph();
}

inline void VoronoiGenerator
::generateInitCellsTasks(
    TaskGraph * tg, 
    glm::dvec3 * points, 
    SyncTask *& syncOut)
{
    syncOut = new SyncTask;
    tg->addTask(unique_ptr<Task>(syncOut));

    auto addTask = [&](auto task, auto && td)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(task, syncOut);
    };

    addTask(new InitCellsTask, TaskDataCells{cell_vector,points,0,(size_t)(1.f / 6.f * m_size) - 1});
    addTask(new InitCellsTask, TaskDataCells{cell_vector,points,(size_t)(1.f / 6.f * m_size), (size_t)(2.f / 6.f * m_size) - 1});
    addTask(new InitCellsTask, TaskDataCells{cell_vector,points,(size_t)(2.f / 6.f * m_size), (size_t)(3.f / 6.f * m_size) - 1});
    addTask(new InitCellsAndResizeSitesTask, TaskDataCellsResize{cell_vector,points,(size_t)(3.f / 6.f * m_size), (size_t)(4.f / 6.f * m_size) - 1, &m_sitesX, m_size});
    addTask(new InitCellsAndResizeSitesTask, TaskDataCellsResize{cell_vector,points,(size_t)(4.f / 6.f * m_size), (size_t)(5.f / 6.f * m_size) - 1, &m_sitesY, m_size});
    addTask(new InitCellsAndResizeSitesTask, TaskDataCellsResize{cell_vector,points,(size_t)(5.f / 6.f * m_size), m_size - 1, &m_sitesZ, m_size});
}

inline void VoronoiGenerator
::generateCapInitCellsTasks(
    TaskGraph * tg, 
    glm::dvec3 * points, 
    SyncTask *& syncInOut)
{
    SyncTask* sync = new SyncTask;
    tg->addTask(unique_ptr<Task>(sync));

    auto addTask = [&](auto task, auto && td)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncInOut, task);
        tg->addDependency(task, sync);
    };

    addTask(new InitCellsTask, TaskDataCells{cell_vector,points,0,(size_t)(m_size/2.f) - 1});
    addTask(new InitCellsAndResizeSitesTask, TaskDataCellsResize{cell_vector,points,(size_t)(m_size/2.f), m_size-1, &m_sitesX, m_size});

    syncInOut = sync;
}

inline void VoronoiGenerator
::generateInitSitesTasks(
    TaskGraph * tg, 
    SyncTask * syncIn, 
    SyncXYZ & syncOut)
{
    syncOut.syncX = new SyncTask;
    syncOut.syncY = new SyncTask;
    syncOut.syncZ = new SyncTask;
    tg->addTask(unique_ptr<Task>(syncOut.syncX));
    tg->addTask(unique_ptr<Task>(syncOut.syncY));
    tg->addTask(unique_ptr<Task>(syncOut.syncZ));

    auto addTask = [&](auto task, auto && td, SyncTask* sync)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncIn, task);
        tg->addDependency(task, sync);
    };

    addTask(new InitSitesTask<X>, TaskDataSites{cell_vector, 0, m_size / 2 - 1, &m_sitesX}, syncOut.syncX);
    addTask(new InitSitesTask<X>, TaskDataSites{cell_vector, m_size / 2, m_size - 1, &m_sitesX}, syncOut.syncX);
    addTask(new InitSitesTask<Y>, TaskDataSites{cell_vector, 0, m_size / 2 - 1, &m_sitesY}, syncOut.syncY);
    addTask(new InitSitesTask<Y>, TaskDataSites{cell_vector, m_size / 2, m_size - 1, &m_sitesY}, syncOut.syncY);
    addTask(new InitSitesTask<Z>, TaskDataSites{cell_vector, 0, m_size / 2 - 1, &m_sitesZ}, syncOut.syncZ);
    addTask(new InitSitesTask<Z>, TaskDataSites{cell_vector, m_size / 2, m_size - 1, &m_sitesZ}, syncOut.syncZ);
}

inline void VoronoiGenerator
::generateRotatePointsTasks(
    TaskGraph * tg, 
    SyncTask *& syncOut,
    glm::dmat4 rotation,
    glm::dvec3* points)
{
    syncOut = new SyncTask;
    tg->addTask(unique_ptr<Task>(syncOut));

    auto addTask = [&](auto task, auto && td)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(task, syncOut);
    };

    addTask(new RotatePointsTask, TaskDataRotatePoints{points,0,m_size/2 - 1, rotation});
    addTask(new RotatePointsTask, TaskDataRotatePoints{points,m_size/2, m_size-1, rotation});
}

inline void VoronoiGenerator
::generateCapInitSitesTasks(
    TaskGraph * tg, 
    SyncTask *& syncInOut)
{
    SyncTask* syncX = new SyncTask;
    tg->addTask(unique_ptr<Task>(syncX));

    auto addTask = [&](auto task, auto && td)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncInOut, task);
        tg->addDependency(task, syncX);
    };

    addTask(new InitSitesTask<X>, TaskDataSites{cell_vector, 0, m_size/2 - 1, &m_sitesX});
    addTask(new InitSitesTask<X>, TaskDataSites{cell_vector, m_size/2, m_size-1, &m_sitesX});

    syncInOut = syncX;
}

inline void VoronoiGenerator::generateSortPointsTasks(TaskGraph * tg, SyncXYZ & syncInOut)
{
    auto p_tempsX1 = new promise<VoronoiSite*>; auto p_tempsX2 = new promise<VoronoiSite*>;
    auto p_tempsY1 = new promise<VoronoiSite*>; auto p_tempsY2 = new promise<VoronoiSite*>;
    auto p_tempsZ1 = new promise<VoronoiSite*>; auto p_tempsZ2 = new promise<VoronoiSite*>;
    
    auto p_doneX1 = new promise<bool>; auto p_doneX2 = new promise<bool>;
    auto p_doneY1 = new promise<bool>; auto p_doneY2 = new promise<bool>;
    auto p_doneZ1 = new promise<bool>; auto p_doneZ2 = new promise<bool>;

    SyncTask* syncX = new SyncTask; tg->addTask(unique_ptr<Task>(syncX));
    SyncTask* syncY = new SyncTask; tg->addTask(unique_ptr<Task>(syncY));
    SyncTask* syncZ = new SyncTask; tg->addTask(unique_ptr<Task>(syncZ));
    
    auto addTask = [&](auto task, auto && td, SyncTask* syncIn, SyncTask* syncOut)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncIn, task);
        tg->addDependency(task, syncOut);
    };

    #define UA unique_ptr<promise<VoronoiSite*>>
    #define UB unique_ptr<promise<bool>>
    #define TD TaskDataDualSort
    addTask(new SortPoints1Task, TD{&m_sitesX, UA(p_tempsX1), UB(p_doneX1), p_tempsX2->get_future(), p_doneX2->get_future()}, syncInOut.syncX, syncX);
    addTask(new SortPoints2Task, TD{&m_sitesX, UA(p_tempsX2), UB(p_doneX2), p_tempsX1->get_future(), p_doneX1->get_future()}, syncInOut.syncX, syncX);
    addTask(new SortPoints1Task, TD{&m_sitesY, UA(p_tempsY1), UB(p_doneY1), p_tempsY2->get_future(), p_doneY2->get_future()}, syncInOut.syncY, syncY);
    addTask(new SortPoints2Task, TD{&m_sitesY, UA(p_tempsY2), UB(p_doneY2), p_tempsY1->get_future(), p_doneY1->get_future()}, syncInOut.syncY, syncY);
    addTask(new SortPoints1Task, TD{&m_sitesZ, UA(p_tempsZ1), UB(p_doneZ1), p_tempsZ2->get_future(), p_doneZ2->get_future()}, syncInOut.syncZ, syncZ);
    addTask(new SortPoints2Task, TD{&m_sitesZ, UA(p_tempsZ2), UB(p_doneZ2), p_tempsZ1->get_future(), p_doneZ1->get_future()}, syncInOut.syncZ, syncZ);

    syncInOut.syncX = syncX;
    syncInOut.syncY = syncY;
    syncInOut.syncZ = syncZ;
}

inline void VoronoiGenerator::generateCapSortPointsTasks(TaskGraph * tg, SyncTask* & syncInOut)
{
    auto p_temps1 = new promise<VoronoiSite*>;
    auto p_temps2 = new promise<VoronoiSite*>;

    auto p_done1 = new promise<bool>;
    auto p_done2 = new promise<bool>;

    SyncTask* syncX = new SyncTask;
    tg->addTask(unique_ptr<Task>(syncX));

    auto addTask = [&](auto task, auto && td)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncInOut, task);
        tg->addDependency(task, syncX);
    };

    #define UA unique_ptr<promise<VoronoiSite*>>
    #define UB unique_ptr<promise<bool>>
    #define TD TaskDataDualSort
    addTask(new SortPoints1Task, TD{&m_sitesX, UA(p_temps1), UB(p_done1), p_temps2->get_future(), p_done2->get_future()});
    addTask(new SortPoints2Task, TD{&m_sitesX, UA(p_temps2), UB(p_done2), p_temps1->get_future(), p_done1->get_future()});

    syncInOut = syncX;
}

inline void VoronoiGenerator
::generateSweepTasks(
    TaskGraph * tg, 
    SyncXYZ & syncIn, 
    SyncTask *& syncOut)
{
    syncOut = new SyncTask;
    tg->addTask(unique_ptr<Task>(syncOut));

    auto addTask = [&](auto task, auto && td, SyncTask* syncIn)
    {
        task->td = move(td);
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncIn, task);
        tg->addDependency(task, syncOut);
    };

    addTask(new SweepTask<Increasing, X>, TaskDataSweep{&m_sitesX, m_gen, 1}, syncIn.syncX);
    addTask(new SweepTask<Decreasing, X>, TaskDataSweep{&m_sitesX, m_gen, 1 << 1}, syncIn.syncX);
    addTask(new SweepTask<Increasing, Y>, TaskDataSweep{&m_sitesY, m_gen, 1 << 2}, syncIn.syncY);
    addTask(new SweepTask<Decreasing, Y>, TaskDataSweep{&m_sitesY, m_gen, 1 << 3}, syncIn.syncY);
    addTask(new SweepTask<Increasing, Z>, TaskDataSweep{&m_sitesZ, m_gen, 1 << 4}, syncIn.syncZ);
    addTask(new SweepTask<Decreasing, Z>, TaskDataSweep{&m_sitesZ, m_gen, 1 << 5}, syncIn.syncZ);
}

inline void VoronoiGenerator
::generateCapSweepTasks(
    TaskGraph * tg,
    SyncTask *& syncInOut)
{
    SweepTask<Increasing, X>* sweepIX = new SweepTask<Increasing, X>;
    sweepIX->td = { &m_sitesX, m_gen, 1 };
    tg->addTask(unique_ptr<Task>(sweepIX));
    tg->addDependency(syncInOut, sweepIX);

    syncInOut = new SyncTask;
    tg->addTask(unique_ptr<Task>(syncInOut));
    tg->addDependency(sweepIX, syncInOut);
}

inline void VoronoiGenerator::generateSortCellCornersTasks(TaskGraph * tg, SyncTask * syncIn, size_t threads)
{
    for (size_t i = 0; i<threads; i++)
    {
        SortCellCornersTask* task = new SortCellCornersTask;
        task->td = { cell_vector, (size_t)(i / (double)threads * m_size), (size_t)((i + 1) / (double)threads * m_size - 1) };
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncIn, task);
    }
}

inline void VoronoiGenerator::generateCapSortCellCornersTasks(TaskGraph * tg, SyncTask * syncIn, size_t threads, glm::dmat4 rotation)
{
    for (size_t i = 0; i<threads; i++)
    {
        SortCornersRotateTask* task = new SortCornersRotateTask;
        task->td = { cell_vector, (size_t)(i / (double)threads * m_size), (size_t)((i + 1) / (double)threads * m_size - 1), rotation };
        tg->addTask(unique_ptr<Task>(task));
        tg->addDependency(syncIn, task);
    }
}

inline void VoronoiGenerator::writeCell(::std::ofstream & os, int i)
{
    if (cell_vector[i].m_arcs != 0)
        return;

    int numCorners = (int)cell_vector[i].corners.size();

    if (numCorners < 3)
        return;

    os.write(reinterpret_cast <const char*>(&numCorners), sizeof(int));
#ifdef CENTROID
    os.write(reinterpret_cast <const char*>(&(cell_vector[i].position.x)), sizeof(double));
    os.write(reinterpret_cast <const char*>(&(cell_vector[i].position.y)), sizeof(double));
    os.write(reinterpret_cast <const char*>(&(cell_vector[i].position.z)), sizeof(double));
#endif
    for (int j = 0; j < numCorners; j++)
    {
        os.write(reinterpret_cast <const char*>(&(cell_vector[i].corners[j].x)), sizeof(double));
        os.write(reinterpret_cast <const char*>(&(cell_vector[i].corners[j].y)), sizeof(double));
        os.write(reinterpret_cast <const char*>(&(cell_vector[i].corners[j].z)), sizeof(double));
    }
        
}

void VoronoiGenerator::writeDataToFile()
{
    #ifdef ENABLE_TIMERS
    boost::timer::auto_cpu_timer t;
    #endif

    ::std::ofstream file;
    file.open("output/voronoi_data", ::std::ofstream::binary);

    if (!file.is_open())
    {
        ::std::cout << "Unable to write data to file.\n";
        return;
    }

    for (size_t i = 0; i < m_size; i++)
        writeCell(file, i);

    file.close();

    ::std::cout << "Data written to: output/voronoi_data\n";
}

void VoronoiGenerator::writeDataToOBJ()
{
    #ifdef ENABLE_TIMERS
    boost::timer::auto_cpu_timer t;
    #endif

    ::std::ofstream file;
    file.open("output/voronoi_data.obj", ::std::ofstream::binary);

    if (!file.is_open())
    {
        ::std::cout << "Unable to write data to file.\n";
        return;
    }

    for (size_t i = 0; i < m_size; i++)
        writeCellOBJ(file, i);

    file.close();

    ::std::cout << "Data written to: output/voronoi_data\n";
}

inline void VoronoiGenerator::writeCellOBJ(::std::ofstream & os, int i)
{
    if (cell_vector[i].m_arcs != 0)
        return;

    int numCorners = (int)cell_vector[i].corners.size();

    if (numCorners < 3)
        return;

    ::std::string idx = "f ";
    for (int j = 0; j < numCorners; j++)
    {
        os.write("v ", 2);

        ::std::string x = ::std::to_string(cell_vector[i].corners[j].x); x += " ";
        ::std::string y = ::std::to_string(cell_vector[i].corners[j].y); y += " ";
        ::std::string z = ::std::to_string(cell_vector[i].corners[j].z); z += "\n";

        os.write(x.c_str(), x.length());
        os.write(y.c_str(), y.length());
        os.write(z.c_str(), z.length());

        idx += ::std::to_string(j-numCorners) + " ";
    }

    // write face
    idx += "\n";
    os.write(idx.c_str(), idx.length());   
}

}
