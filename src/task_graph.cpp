#include "task_graph.h"
#include <thread>

namespace VorGen {

TaskGraph::~TaskGraph()
{
    for (unsigned int i = 0; i < m_tasks.size(); i++)
    {
        delete m_tasks[i];
    }
}

void TaskGraph::processTasks(int numThreads)
{
    // create threads
    ::std::thread* m_threads = new ::std::thread[numThreads-1];
    for (int i = 0; i < numThreads - 1; i++)
    {
        m_threads[i] = ::std::thread(processTasksThread, this);
    }

    processTasksThread(this);

    // destroy threads
    for (int i = 0; i < numThreads - 1; i++)
    {
        m_threads[i].join();
    }

    delete [] m_threads;
}

void TaskGraph::processTasksThread(TaskGraph* tg)
{
    while (1)
    {
        tg->m_lock.lock();
        
        Task* task = NULL;
        if (tg->m_leaves.size())
        {
            task = tg->m_leaves.back();
            tg->m_leaves.pop_back();
        }

        tg->m_lock.unlock();

        if (task)
        {
            task->process();

            tg->m_lock.lock();
            tg->markTaskComplete(task);
            tg->m_lock.unlock();
        }

        if (tg->m_final.m_preReqs == 0)
            break;
    }
}

void TaskGraph::markTaskComplete(Task * t)
{
    for (unsigned int i = 0; i < t->m_dependents.size(); i++)
    {
        if ( --(t->m_dependents[i]->m_preReqs) == 0)
        {
            if (t->m_dependents[i]->isEmpty)
                markTaskComplete(t->m_dependents[i]);
            else
                m_leaves.push_back(t->m_dependents[i]);
        }
    }
}

void TaskGraph::addTask(Task * t)
{
    m_tasks.push_back(t);
}

void TaskGraph::addDependency(Task * p, Task * dependent)
{
    p->m_dependents.push_back(dependent);
    dependent->m_preReqs++;
}

void TaskGraph::finalizeGraph()
{
    for (unsigned int i = 0; i < m_tasks.size(); i++)
    {
        Task* t = m_tasks[i];
        if (t->m_preReqs == 0)
        {
            m_leaves.push_back(t);
        }
        if (t->m_dependents.empty())
        {
            t->m_dependents.push_back(&m_final);
            m_final.m_preReqs++;
        }
    }
}

Task::Task()
{
    m_preReqs.store(0);
    isEmpty = false;
}

}
