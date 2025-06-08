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

/// @brief Console namespace, which contains the Console class for command line interface operations \namespace ConsoleInternals
namespace ConsoleInternals
{
    /// @brief Console class, which provides methods for command line interface operations \class Console
    class Console
    {
    public:

        /**
         * @brief Construct a new Console object
         */
        Console();

        /**
         * @brief Destroy the Console object
         */
        ~Console();

        /**
         * @brief Initialize the console
         */
        static void Initialize();

        /**
         * @brief Run the console in a separate thread
         */
        void run();

        /**
         * @brief Stop the console
         */
        void stop();

        /**
         * @brief Show the console window
         */
        void showConsole();

        /**
         * @brief Execute a command in the console
         * @param command The command to execute
         * @return A string containing the output of the command
         */
        std::string executeCommand(const std::string& command);

        /**
         * @brief Process input from the console
         * @param input The input string to process
         */
        void processInput(const std::string& input);

        /**
         * @brief Execute a shell command and return the output
         * @param command The shell command to execute
         * @return A string containing the output of the command
         */
        static std::string executeShellCommand(const std::string& command);

    private:

        /**
         * @brief Display the console prompt
         */
        static void displayPrompt();

        /**
         * @brief Handle the 'help' command
         */
        void handleHelp();

        /**
         * @brief Handle the 'exit' command
         */
        void handleExit();

        /**
         * @brief Handle the 'history' command
         */
        void handleHistory();

        /**
         * @brief Handles a custom command
         * @param command The custom command to handlu
         */
        void handleCustomCommand(const std::string& command);

        std::atomic<bool> running;
        std::thread inputThread;
        std::mutex consoleMutex;
        std::unordered_map<std::string, std::function<void()>> commandMap;
        std::deque<std::string> commandHistory;
        size_t historyIndex;
    };
}
