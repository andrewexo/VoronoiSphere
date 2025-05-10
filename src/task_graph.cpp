#include "task_graph.h"
#include <thread>
#include <iostream>
#include <algorithm>
#include <unordered_map>

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

void TaskGraph::printGraph()
{
    // Create a copy of the task graph for traversal
    std::vector<Task*> tasks;
    std::vector<uint8_t> inDegree;
    
    // Initialize with all tasks
    for (auto& task : m_tasks)
    {
        tasks.push_back(task.get());
        inDegree.push_back(task->m_preReqs.load());
    }
    
    // Topological sort using Kahn's algorithm
    std::queue<Task*> queue;
    
    // Add all nodes with no incoming edges to the queue
    for (size_t i = 0; i < tasks.size(); i++)
    {
        if (inDegree[i] == 0)
            queue.push(tasks[i]);
    }
    
    // Map to track distance from start for each task
    std::unordered_map<Task*, size_t> distanceFromStart;
    for (Task* task : tasks)
    {
        if (task->m_preReqs == 0)
            distanceFromStart[task] = 0;
    }
    
    std::cout << "Task Graph (topological order):" << std::endl;
    while (!queue.empty())
    {
        Task* current = queue.front();
        queue.pop();
        
        std::cout << std::string(distanceFromStart[current], ' ') << typeid(*current).name() << std::endl;
        
        // Process all neighbors
        for (Task* dependent : current->m_dependents)
        {
            // Skip the final sync task
            if (dependent == &m_final)
                continue;
                
            // Find the dependent in our task list
            auto it = std::find(tasks.begin(), tasks.end(), dependent);
            if (it != tasks.end())
            {
                size_t index = it - tasks.begin();
                inDegree[index]--;
                
                if (inDegree[index] == 0) {
                    distanceFromStart[dependent] = distanceFromStart[current] + 1;
                    queue.push(dependent);
                }
            }
        }
    }
}

Task::Task() : m_preReqs(0), isEmpty(false) {}

}
