#include "../include/CORE/ThreadingBase.h"

ThreadingBase::ThreadingBase() : stopFlag(false), numActiveThreads(0)
{
}

ThreadingBase::~ThreadingBase()
{
    stopThreadPool();
    joinAllThreads();
}

void ThreadingBase::startThread(const std::function<void()>& threadFunction)
{
    std::thread newThread(threadFunction);
    if (newThread.joinable())
    {
        newThread.detach();
    }
}

void ThreadingBase::joinAllThreads()
{
    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    threads.clear();
}

void ThreadingBase::enqueueTask(const std::function<void()>& task)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push(task);
    condition.notify_one();
}

void ThreadingBase::startThreadPool(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads.push_back(std::thread(&ThreadingBase::threadPoolFunction, this));
    }
}


void ThreadingBase::stopThreadPool()
{
    stopFlag = true;
    condition.notify_all();
}

void ThreadingBase::waitForTasks()
{
    while (!taskQueue.empty())
    {
        std::this_thread::yield();
    }
}

size_t ThreadingBase::getNumThreads() const
{
    return threads.size();
}

bool ThreadingBase::isThreadPoolRunning() const
{
    return !stopFlag.load();
}

void ThreadingBase::threadPoolFunction()
{
    ++numActiveThreads;
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [&]() { return stopFlag.load() || !taskQueue.empty(); });
            if (stopFlag.load() && taskQueue.empty())
            {
                --numActiveThreads;
                return;
            }
            if (!taskQueue.empty())
            {
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
        }
        if (task)
        {
            task();
        }
    }
}
