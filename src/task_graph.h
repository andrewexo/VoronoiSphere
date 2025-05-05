#pragma once

#include "spin_lock.h"
#include "globals.h"
#include <atomic>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace VorGen {

class Task
{
    public:

        Task();
        virtual ~Task() = default;

        virtual void process() = 0;

        ::std::vector<Task*> m_dependents;
        ::std::atomic<uint8_t> m_preReqs;
        bool isEmpty;
};

class SyncTask : public Task
{
    public:

        SyncTask() { isEmpty = true; m_preReqs.store(0); };
        virtual ~SyncTask() {};
        void process() override {}
};

class TaskGraph
{
    public:

        ~TaskGraph() = default;

        void processTasks(int numThreads);

        static void processTasksThread(TaskGraph* tg);

        void markTaskComplete(Task* t);

        void addTask(std::unique_ptr<Task> t);
        void addDependency(Task* p, Task* dependent);
        void finalizeGraph();

    private:

        ::std::vector<std::unique_ptr<Task>> m_tasks;
        ::std::vector<Task*> m_leaves;

        SyncTask m_final;
        SpinLock m_lock;
};

}