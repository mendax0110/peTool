#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>

/// @brief ThreadingBase class, which provides methods for managing threads and task queues \class ThreadingBase
class ThreadingBase
{
public:

    /**
     * @brief Construct a new ThreadingBase object
     */
    ThreadingBase();

    /**
     * @brief Destroy the ThreadingBase object
     */
    ~ThreadingBase();

    /**
     * @brief Start a new thread with the specified function
     * @param threadFunction The function to run in the new thread
     */
    void startThread(const std::function<void()>& threadFunction);

    /**
     * @brief Join all threads and wait for them to finish
     */
    void joinAllThreads();

    /**
     * @brief Enqueue a task to be executed by the thread pool
     * @param task The task to enqueue
     */
    void enqueueTask(const std::function<void()>& task);

    /**
     * @brief Start the thread pool with the specified number of threads
     * @param numThreads The number of threads to start in the pool
     */
    void startThreadPool(size_t numThreads);

    /**
     * @brief Stop the thread pool and wait for all tasks to finish
     */
    void stopThreadPool();

    /**
     * @brief Wait for all tasks in the queue to finish
     */
    void waitForTasks();

    /**
     * @brief Get the number of active threads in the thread pool
     * @return The number of active threads
     */
    size_t getNumThreads() const;

    /**
     * @brief Check if the thread pool is currently running
     * @return true if the thread pool is running, false otherwise
     */
    bool isThreadPoolRunning() const;

private:
    std::vector<std::thread> threads;
    std::mutex queueMutex;
    std::queue<std::function<void()>> taskQueue;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    std::atomic<size_t> numActiveThreads;

    /**
     * @brief The function that each thread in the pool will run
     */
    void threadPoolFunction();
};
