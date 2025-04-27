#include "voronoi_generator.h"

#include "voronoi.h"
#include "globals.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <cstring>
#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>
#include <mutex>
#include <atomic>
#include <thread>
#include <future>

namespace VorGen {

// Define this macro to enable timing output for SweepTasks
//#define ENABLE_SWEEP_TIMERS

#ifdef ENABLE_SWEEP_TIMERS
// Global mutex for synchronizing console output
::std::mutex cout_mutex;
#endif

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
    glm::dvec3 origin;
    glm::dvec3 originY;
    glm::dvec3 originZ;
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

class InitSitesCapTask : public Task
{
    public:
        ~InitSitesCapTask() {};
        void process();
        TaskDataSitesCap td;
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

VoronoiGenerator::VoronoiGenerator()
{
  cell_vector = NULL;
}

VoronoiGenerator::VoronoiGenerator(unsigned int seed) : sample_generator(seed)
{
  cell_vector = NULL;
}

VoronoiGenerator::~VoronoiGenerator()
{
  if (cell_vector) delete[] cell_vector;
}

void VoronoiGenerator::clear()
{
  if (cell_vector) delete[] cell_vector;
  cell_vector = NULL;
}

glm::dvec3 * VoronoiGenerator::genRandomInput(int count)
{
  return sample_generator.getRandomPointsSphere(count);
}

void VoronoiGenerator::generate(glm::dvec3* points, int count, int gen, bool writeToFile)
{
  {
    m_size = count;
    m_gen = gen;
    cell_vector = new VoronoiCell[count];

    TaskGraph taskGraph; buildTaskGraph(&taskGraph, points);
    taskGraph.processTasks(6);
    completedCells = 0; // reset after generation for next generation
  }

  if (writeToFile) writeDataToOBJ();
}

void VoronoiGenerator::generateCap(const glm::dvec3& origin, glm::dvec3* points, int count, ::std::vector<VoronoiCell>& out)
{
    m_size = count;
    m_gen = count;
    cell_vector = new VoronoiCell[count];

    TaskGraph taskGraph; buildCapTaskGraph(&taskGraph, origin, points);
    taskGraph.processTasks(2);
    completedCells = 0; // reset after generation for next generation

    //out.assign(cell_vector, cell_vector + count);
}

inline void VoronoiGenerator
::generateInitCellsTasks(
  TaskGraph * tg, 
  glm::dvec3 * points, 
  SyncTask *& syncOut)
{
  InitCellsTask* ict1 = new InitCellsTask;
  InitCellsTask* ict2 = new InitCellsTask;
  InitCellsTask* ict3 = new InitCellsTask;

  ict1->td = { 
    cell_vector, 
    points, 
    0,								  
    (unsigned int)(1.f / 6.f * m_size) - 1 };

  ict2->td = { 
    cell_vector, 
    points, 
    (unsigned int)(1.f / 6.f * m_size), 
    (unsigned int)(2.f / 6.f * m_size) - 1 };

  ict3->td = { 
    cell_vector, 
    points, 
    (unsigned int)(2.f / 6.f * m_size), 
    (unsigned int)(3.f / 6.f * m_size) - 1 };

  tg->addTask(ict1);
  tg->addTask(ict2);
  tg->addTask(ict3);

  InitCellsAndResizeSitesTask* icrt1 = new InitCellsAndResizeSitesTask;
  InitCellsAndResizeSitesTask* icrt2 = new InitCellsAndResizeSitesTask;
  InitCellsAndResizeSitesTask* icrt3 = new InitCellsAndResizeSitesTask;

  icrt1->td = { 
    cell_vector, 
    points, 
    (unsigned int)(3.f / 6.f * m_size), 
    (unsigned int)(4.f / 6.f * m_size) - 1, 
    &m_sitesX, 
    m_size };

  icrt2->td = { 
    cell_vector, 
    points, 
    (unsigned int)(4.f / 6.f * m_size), 
    (unsigned int)(5.f / 6.f * m_size) - 1, 
    &m_sitesY, 
    m_size };

  icrt3->td = { 
    cell_vector, 
    points, 
    (unsigned int)(5.f / 6.f * m_size), 
    m_size - 1,							   
    &m_sitesZ, 
    m_size };

  tg->addTask(icrt1);
  tg->addTask(icrt2);
  tg->addTask(icrt3);

  SyncTask* sync = new SyncTask;
  tg->addTask(sync);

  tg->addDependency(ict1, sync);
  tg->addDependency(ict2, sync);
  tg->addDependency(ict3, sync);
  tg->addDependency(icrt1, sync);
  tg->addDependency(icrt2, sync);
  tg->addDependency(icrt3, sync);

  syncOut = sync;
}

inline void VoronoiGenerator
::generateCapInitCellsTasks(
  TaskGraph * tg, 
  glm::dvec3 * points, 
  SyncTask *& syncOut)
{
  InitCellsTask* ict = new InitCellsTask;
  ict->td = { 
    cell_vector, 
    points, 
    0,								  
    (unsigned int)(1.f / 2.f * m_size) - 1 };
  tg->addTask(ict);

  InitCellsAndResizeSitesTask* icrt = new InitCellsAndResizeSitesTask;
  icrt->td = { 
    cell_vector, 
    points, 
    (unsigned int)(1.f / 2.f * m_size), 
    m_size - 1, 
    &m_sitesX, 
    m_size };
  tg->addTask(icrt);

  SyncTask* sync = new SyncTask;
  tg->addTask(sync);
  tg->addDependency(ict, sync);
  tg->addDependency(icrt, sync);

  syncOut = sync;
}

inline void VoronoiGenerator
::generateInitSitesTasks(
  TaskGraph * tg, 
  SyncTask * syncIn, 
  SyncXYZ & syncOut)
{
  InitSitesTask<X>* ist1 = new InitSitesTask<X>;
  InitSitesTask<X>* ist2 = new InitSitesTask<X>;
  InitSitesTask<Y>* ist3 = new InitSitesTask<Y>;
  InitSitesTask<Y>* ist4 = new InitSitesTask<Y>;
  InitSitesTask<Z>* ist5 = new InitSitesTask<Z>;
  InitSitesTask<Z>* ist6 = new InitSitesTask<Z>;

  ist1->td = { 
    cell_vector, 
    0, 
    (unsigned int)m_size / 2 - 1, 
    &m_sitesX };

  ist2->td = { 
    cell_vector, 
    (unsigned int)m_size / 2,
    (unsigned int)m_size - 1,	  
    &m_sitesX };

  ist3->td = { 
    cell_vector,
    0,						
    (unsigned int)m_size / 2 - 1, 
    &m_sitesY };

  ist4->td = { 
    cell_vector, 
    (unsigned int)m_size / 2, 
    (unsigned int)m_size - 1,	  
    &m_sitesY };

  ist5->td = { 
    cell_vector, 
    0,						
    (unsigned int)m_size / 2 - 1, 
    &m_sitesZ };

  ist6->td = { 
    cell_vector, 
    (unsigned int)m_size / 2, 
    (unsigned int)m_size - 1,	  
    &m_sitesZ };

  tg->addTask(ist1);
  tg->addTask(ist2);
  tg->addTask(ist3);
  tg->addTask(ist4);
  tg->addTask(ist5);
  tg->addTask(ist6);

  tg->addDependency(syncIn, ist1);
  tg->addDependency(syncIn, ist2);
  tg->addDependency(syncIn, ist3);
  tg->addDependency(syncIn, ist4);
  tg->addDependency(syncIn, ist5);
  tg->addDependency(syncIn, ist6);

  SyncTask* syncX = new SyncTask;
  SyncTask* syncY = new SyncTask;
  SyncTask* syncZ = new SyncTask;
  tg->addTask(syncX);
  tg->addTask(syncY);
  tg->addTask(syncZ);

  tg->addDependency(ist1, syncX);
  tg->addDependency(ist2, syncX);
  tg->addDependency(ist3, syncY);
  tg->addDependency(ist4, syncY);
  tg->addDependency(ist5, syncZ);
  tg->addDependency(ist6, syncZ);

  syncOut.syncX = syncX;
  syncOut.syncY = syncY;
  syncOut.syncZ = syncZ;
}

inline void VoronoiGenerator
::generateCapInitSitesTasks(
  TaskGraph * tg, 
  SyncTask *& syncInOut,
  const glm::dvec3 & origin)
{
  InitSitesCapTask* ist1 = new InitSitesCapTask;
  InitSitesCapTask* ist2 = new InitSitesCapTask;

  glm::dvec3 helper;
  if (origin.x < origin.y && origin.x < origin.z)
      helper = glm::dvec3(1.0, 0.0, 0.0);
  else if (origin.y < origin.z)
      helper = glm::dvec3(0.0, 1.0, 0.0);
  else
      helper = glm::dvec3(0.0, 0.0, 1.0);
  glm::dvec3 originY = glm::cross(origin,helper);
  glm::dvec3 originZ = glm::cross(origin,originY);

  ist1->td = { 
    cell_vector, 
    origin,
    originY,
    originZ,
    0, 
    (unsigned int)m_size / 2 - 1, 
    &m_sitesX
  };

  ist2->td = { 
    cell_vector,
    origin,
    originY,
    originZ,
    (unsigned int)m_size / 2,
    (unsigned int)m_size - 1,	  
    &m_sitesX
  };

  tg->addTask(ist1);
  tg->addTask(ist2);

  tg->addDependency(syncInOut, ist1);
  tg->addDependency(syncInOut, ist2);

  SyncTask* syncX = new SyncTask;
  tg->addTask(syncX);

  tg->addDependency(ist1, syncX);
  tg->addDependency(ist2, syncX);

  syncInOut = syncX;
}

inline void VoronoiGenerator::generateSortPointsTasks(TaskGraph * tg, SyncXYZ & syncInOut)
{
  SortPoints1Task* sp1 = new SortPoints1Task;
  SortPoints2Task* sp2 = new SortPoints2Task;
  SortPoints1Task* sp3 = new SortPoints1Task;
  SortPoints2Task* sp4 = new SortPoints2Task;
  SortPoints1Task* sp5 = new SortPoints1Task;
  SortPoints2Task* sp6 = new SortPoints2Task;

  ::std::promise<VoronoiSite*>* p_temps1 = new ::std::promise<VoronoiSite*>[2];
  ::std::promise<VoronoiSite*>* p_temps2 = new ::std::promise<VoronoiSite*>[2];
  ::std::promise<VoronoiSite*>* p_temps3 = new ::std::promise<VoronoiSite*>[2];
  
  ::std::promise<bool>* p_done1 = new ::std::promise<bool>[2];
  ::std::promise<bool>* p_done2 = new ::std::promise<bool>[2];
  ::std::promise<bool>* p_done3 = new ::std::promise<bool>[2];

  sp1->td = { &m_sitesX, p_temps1, p_done1, 
                         (p_temps1+1)->get_future(), 
                         (p_done1+1)->get_future() };
  sp2->td = { &m_sitesX, p_temps1 + 1, p_done1 + 1,             
                         p_temps1->get_future(), 
                         p_done1->get_future() };

  sp3->td = { &m_sitesY, p_temps2, p_done2, 
                         (p_temps2+1)->get_future(), 
                         (p_done2+1)->get_future() };
  sp4->td = { &m_sitesY, p_temps2 + 1, p_done2 + 1,                          
                         p_temps2->get_future(), 
                         p_done2->get_future() };

  sp5->td = { &m_sitesZ, p_temps3, p_done3, 
                         (p_temps3+1)->get_future(), 
                         (p_done3+1)->get_future() };
  sp6->td = { &m_sitesZ, p_temps3 + 1, p_done3 + 1,                          
                         p_temps3->get_future(), 
                         p_done3->get_future() };

  tg->addTask(sp1);
  tg->addTask(sp2);
  tg->addTask(sp3);
  tg->addTask(sp4);
  tg->addTask(sp5);
  tg->addTask(sp6);

  tg->addDependency(syncInOut.syncX, sp1);
  tg->addDependency(syncInOut.syncX, sp2);
  tg->addDependency(syncInOut.syncY, sp3);
  tg->addDependency(syncInOut.syncY, sp4);
  tg->addDependency(syncInOut.syncZ, sp5);
  tg->addDependency(syncInOut.syncZ, sp6);

  SyncTask* syncX = new SyncTask;
  SyncTask* syncY = new SyncTask;
  SyncTask* syncZ = new SyncTask;
  tg->addTask(syncX);
  tg->addTask(syncY);
  tg->addTask(syncZ);

  tg->addDependency(sp1, syncX);
  tg->addDependency(sp2, syncX);
  tg->addDependency(sp3, syncY);
  tg->addDependency(sp4, syncY);
  tg->addDependency(sp5, syncZ);
  tg->addDependency(sp6, syncZ);

  syncInOut.syncX = syncX;
  syncInOut.syncY = syncY;
  syncInOut.syncZ = syncZ;
}

inline void VoronoiGenerator::generateCapSortPointsTasks(TaskGraph * tg, SyncTask* & syncInOut)
{
  SortPoints1Task* sp1 = new SortPoints1Task;
  SortPoints2Task* sp2 = new SortPoints2Task;

  ::std::promise<VoronoiSite*>* p_temps = new ::std::promise<VoronoiSite*>[2];
  ::std::promise<bool>* p_done = new ::std::promise<bool>[2];

  sp1->td = { &m_sitesX, p_temps, p_done, 
                         (p_temps+1)->get_future(), 
                         (p_done+1)->get_future() };
  sp2->td = { &m_sitesX, p_temps + 1, p_done + 1,             
                         p_temps->get_future(), 
                         p_done->get_future() };

  tg->addTask(sp1);
  tg->addTask(sp2);

  tg->addDependency(syncInOut, sp1);
  tg->addDependency(syncInOut, sp2);

  SyncTask* syncX = new SyncTask;
  tg->addTask(syncX);

  tg->addDependency(sp1, syncX);
  tg->addDependency(sp2, syncX);

  syncInOut = syncX;
}

inline void VoronoiGenerator
::generateSweepTasks(
  TaskGraph * tg, 
  SyncXYZ & syncIn, 
  SyncTask *& syncOut)
{
  SweepTask<Increasing, X>* sweepIX = new SweepTask<Increasing, X>;
  SweepTask<Decreasing, X>* sweepDX = new SweepTask<Decreasing, X>;
  SweepTask<Increasing, Y>* sweepIY = new SweepTask<Increasing, Y>;
  SweepTask<Decreasing, Y>* sweepDY = new SweepTask<Decreasing, Y>;
  SweepTask<Increasing, Z>* sweepIZ = new SweepTask<Increasing, Z>;
  SweepTask<Decreasing, Z>* sweepDZ = new SweepTask<Decreasing, Z>;

  sweepIX->td = { &m_sitesX, m_gen, 1 };
  sweepDX->td = { &m_sitesX, m_gen, 1 << 1 };
  sweepIY->td = { &m_sitesY, m_gen, 1 << 2 };
  sweepDY->td = { &m_sitesY, m_gen, 1 << 3 };
  sweepIZ->td = { &m_sitesZ, m_gen, 1 << 4 };
  sweepDZ->td = { &m_sitesZ, m_gen, 1 << 5 };

  tg->addTask(sweepIX);
  tg->addTask(sweepIY);
  tg->addTask(sweepIZ);
  tg->addTask(sweepDX);
  tg->addTask(sweepDY);
  tg->addTask(sweepDZ);

  tg->addDependency(syncIn.syncX, sweepIX);
  tg->addDependency(syncIn.syncX, sweepDX);
  tg->addDependency(syncIn.syncY, sweepIY);
  tg->addDependency(syncIn.syncY, sweepDY);
  tg->addDependency(syncIn.syncZ, sweepIZ);
  tg->addDependency(syncIn.syncZ, sweepDZ);

  SyncTask* sync = new SyncTask;
  tg->addTask(sync);

  tg->addDependency(sweepIX, sync);
  tg->addDependency(sweepIY, sync);
  tg->addDependency(sweepIZ, sync);
  tg->addDependency(sweepDX, sync);
  tg->addDependency(sweepDY, sync);
  tg->addDependency(sweepDZ, sync);

  syncOut = sync;
}

inline void VoronoiGenerator
::generateCapSweepTasks(
  TaskGraph * tg,
  SyncTask *& syncInOut)
{
  SweepTask<Increasing, X>* sweepIX = new SweepTask<Increasing, X>;
  sweepIX->td = { &m_sitesX, m_gen, 0 };
  tg->addTask(sweepIX);
  tg->addDependency(syncInOut, sweepIX);

  SyncTask* sync = new SyncTask;
  tg->addTask(sync);
  tg->addDependency(sweepIX, sync);

  syncInOut = sync;
}

inline void VoronoiGenerator::generateSortCellCornersTasks(TaskGraph * tg, SyncTask * syncIn, size_t threads)
{
  for (size_t i = 0; i<threads; i++)
  {
    SortCellCornersTask* scct = new SortCellCornersTask;
    scct->td = { cell_vector, (unsigned int)(i / (double)threads * m_size), (unsigned int)((i + 1) / (double)threads * m_size - 1) };

    tg->addTask(scct);
    tg->addDependency(syncIn, scct);
  }
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

  generateCapInitCellsTasks(tg, points, sync);
  generateCapInitSitesTasks(tg, sync, origin);
  generateCapSortPointsTasks(tg, sync);
  generateCapSweepTasks(tg, sync);
  generateSortCellCornersTasks(tg, sync, 2);

  tg->finalizeGraph();
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
  boost::timer::auto_cpu_timer t;

  ::std::ofstream file;
  file.open("output/voronoi_data", ::std::ofstream::binary);

  if (!file.is_open())
  {
    ::std::cout << "Unable to write data to file.\n";
    return;
  }

  for (unsigned int i = 0; i < m_size; i++)
  {
    writeCell(file, i);
  }

  file.close();

  ::std::cout << "Data written to: output/voronoi_data\n";
}

void VoronoiGenerator::writeDataToOBJ()
{
  boost::timer::auto_cpu_timer t;

  ::std::ofstream file;
  file.open("output/voronoi_data.obj", ::std::ofstream::binary);

  if (!file.is_open())
  {
    ::std::cout << "Unable to write data to file.\n";
    return;
  }

  for (unsigned int i = 0; i < m_size; i++)
  {
    writeCellOBJ(file, i);
  }

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


void InitCellsTask::process()
{
  for (unsigned int i = td.start; i <= td.end; i++)
  {
    new(td.cells + i) VoronoiCell(td.points[i]);
  }
}

void InitCellsAndResizeSitesTask::process()
{
  for (unsigned int i = td.start; i <= td.end; i++)
  {
    new(td.cells + i) VoronoiCell(td.points[i]);
  }

  td.sites->resize(td.size);
}

template<Axis A>
void InitSitesTask<A>::process()
{
  for (unsigned int i = td.start; i <= td.end; i++)
  {
    VoronoiSite site{(td.cells)[i].position, td.cells + i, A};
    (*(td.sites))[i] = site;
  }
}

template class InitSitesTask<X>;
template class InitSitesTask<Y>;
template class InitSitesTask<Z>;

void InitSitesCapTask::process()
{
  for (unsigned int i = td.start; i <= td.end; i++)
  {
    VoronoiSite site{(td.cells)[i].position, td.cells + i, td.origin, td.originY, td.originZ};
    (*(td.sites))[i] = site;
  }
}

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
  memcpy(scratch, td.sites->data() + size, size * sizeof(VoronoiSite));

  // send data to other thread
  td.p_temps->set_value(scratch);
  VoronoiSite* scratch1 = td.f_temps.get();

  // merge into original array
  int a = size1 - 1; int b = size - 1;
  for (unsigned int i = (unsigned int)td.sites->size() - 1; i >= size; i--)
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

}
