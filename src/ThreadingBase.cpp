#include "../include/CORE/ThreadingBase.h"

/**
 * @brief Construct a new Threading Base:: Threading Base object
 * @details Initializes the stop flag and the number of active threads
 */
ThreadingBase::ThreadingBase() : stopFlag(false), numActiveThreads(0)
{

}

/**
 * @brief Destroy the Threading Base:: Threading Base object
 * @details Stops the thread pool and joins all threads
 */
ThreadingBase::~ThreadingBase()
{
    stopThreadPool();
    joinAllThreads();
}

/**
 * @brief Start a new thread
 * @param threadFunction The function to run in the thread
 */
void ThreadingBase::startThread(const std::function<void()>& threadFunction)
{
    std::thread newThread(threadFunction);
    if (newThread.joinable())
    {
        newThread.detach();
    }
}

/**
 * @brief Join all threads
 */
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

/**
 * @brief Enqueue a task to the thread pool
 * @param task The task to enqueue
 */
void ThreadingBase::enqueueTask(const std::function<void()>& task)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push(task);
    condition.notify_one();
}

/**
 * @brief Start the thread pool
 * @param numThreads The number of threads to start
 */
void ThreadingBase::startThreadPool(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(&ThreadingBase::threadPoolFunction, this);
    }
}

/**
 * @brief Stop the thread pool
 */
void ThreadingBase::stopThreadPool()
{
    stopFlag = true;
    condition.notify_all();
}

/**
 * @brief Wait for all tasks to complete
 */
void ThreadingBase::waitForTasks()
{
    while (!taskQueue.empty())
    {
        std::this_thread::yield();
    }
}

/**
 * @brief Get the number of threads in the thread pool
 * @return The number of threads
 */
size_t ThreadingBase::getNumThreads() const
{
    return threads.size();
}

/**
 * @brief Check if the thread pool is running
 * @return True if the thread pool is running, false otherwise
 */
bool ThreadingBase::isThreadPoolRunning() const
{
    return !stopFlag.load();
}

/**
 * @brief Get the number of active threads
 * @return The number of active threads
 */
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
