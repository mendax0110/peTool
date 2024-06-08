#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <deque>

namespace ConsoleInternals
{
    class Console
    {
    public:
        Console();
        ~Console();

        static void Initialize();
        void run();
        void stop();
        void showConsole();
        std::string executeCommand(const std::string& command);
        void processInput(const std::string& input);
        static std::string executeShellCommand(const std::string& command);

    private:

        static void displayPrompt();
        void handleHelp();
        void handleExit();
        void handleHistory();
        void handleCustomCommand(const std::string& command);

        std::atomic<bool> running;
        std::thread inputThread;
        std::mutex consoleMutex;
        std::unordered_map<std::string, std::function<void()>> commandMap;
        std::deque<std::string> commandHistory;
        size_t historyIndex;
    };
}
