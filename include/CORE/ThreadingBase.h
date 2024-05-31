#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>

class ThreadingBase
{
public:
    ThreadingBase();
    ~ThreadingBase();

    void startThread(const std::function<void()>& threadFunction);
    void joinAllThreads();
    void enqueueTask(const std::function<void()>& task);
    void startThreadPool(size_t numThreads);
    void stopThreadPool();
    void waitForTasks();
    size_t getNumThreads() const;
    bool isThreadPoolRunning() const;

private:
    std::vector<std::thread> threads;
    std::mutex queueMutex;
    std::queue<std::function<void()>> taskQueue;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    std::atomic<size_t> numActiveThreads;

    void threadPoolFunction();
};
