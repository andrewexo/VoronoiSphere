#include "task_graph.h"
#include <thread>

namespace VorGen {


void TaskGraph::processTasks(int numThreads)
{
    ::std::vector<::std::thread> threads;
    for (int i = 0; i < numThreads; i++)
    {
        threads.push_back(::std::thread(processTasksThread, this));
    }
    processTasksThread(this);
    for (auto& thread : threads)
    {
        thread.join();
    }
}

void TaskGraph::processTasksThread(TaskGraph* tg)
{
    while (true)
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

void TaskGraph::markTaskComplete(Task* t)
{
    for (auto dependent : t->m_dependents)
    {
        if ( --(dependent->m_preReqs) == 0)
        {
            if (dependent->isEmpty)
                markTaskComplete(dependent);
            else
                m_leaves.push_back(dependent);
        }
    }
}

void TaskGraph::addTask(std::unique_ptr<Task> t)
{
    m_tasks.push_back(std::move(t));
}

void TaskGraph::addDependency(Task* p, Task* dependent)
{
    p->m_dependents.push_back(dependent);
    dependent->m_preReqs++;
}

void TaskGraph::finalizeGraph()
{
    for (auto& task : m_tasks)
    {
        if (task->m_preReqs == 0)
        {
            m_leaves.push_back(task.get());
        }
        if (task->m_dependents.empty())
        {
            task->m_dependents.push_back(&m_final);
            m_final.m_preReqs++;
        }
    }
}

Task::Task() : m_preReqs(0), isEmpty(false) {}

}
