#include "../include/CLI/Console.h"

#if defined(_WIN32)
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <array>
#elif defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#include <memory>
#include <array>
#include <future>
#include <sstream>
#endif

using namespace ConsoleInternals;

/**
 * @brief Construct a new Console:: Console object
 * @details Initializes the console with default commands.
 */
Console::Console() : running(false), historyIndex(0)
{
    commandMap["help"] = [this]() { handleHelp(); };
    commandMap["exit"] = [this]() { handleExit(); };
    commandMap["history"] = [this]() { handleHistory(); };
    commandMap["custom"] = [this]() { handleCustomCommand("custom"); };
}

/**
 * @brief Destroy the Console:: Console object
 */
Console::~Console()
{
    stop();
}

/**
 * @brief Initialize the console
 */
void Console::Initialize()
{
#if defined(_WIN32)
    AllocConsole();
#include <cstdio>

    FILE* dummy;
    if (freopen_s(&dummy, "CONIN$", "r", stdin) != 0)
    {
        std::cerr << "Error opening CONIN$ for reading." << std::endl;
    }
    if (freopen_s(&dummy, "CONOUT$", "w", stdout) != 0)
    {
        std::cerr << "Error opening CONOUT$ for writing." << std::endl;
}
    if (freopen_s(&dummy, "CONOUT$", "w", stderr) != 0)
    {
        std::cerr << "Error opening CONOUT$ for writing." << std::endl;
    }

#elif defined(__APPLE__) || defined(__linux__)
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

/**
 * @brief Run the console
 */
void Console::run()
{
    running = true;
    while (running)
    {
        displayPrompt();
        std::string input;
        std::getline(std::cin, input);
        if (!input.empty())
        {
            processInput(input);
        }
    }
}

/**
 * @brief Stop the console
 */
void Console::stop()
{
    running = false;

#if defined(_WIN32)
    FreeConsole();
#elif defined(__APPLE__) || defined(__linux__)
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

/**
 * @brief Process the input from the console
 * @param input The input string
 */
void Console::processInput(const std::string &input)
{
    if (input.empty())
        return;

    commandHistory.push_front(input);
    historyIndex = commandHistory.size();

    auto command = commandMap.find(input);
    if (command != commandMap.end())
    {
        command->second();
    }
    else
    {
        std::cout << executeShellCommand(input) << std::endl;
    }
}

/**
 * @brief Display the console prompt
 */
void Console::displayPrompt()
{
    std::cout << "> " << std::flush;
}

/**
 * @brief Handle the help command
 */
void Console::handleHelp()
{
    std::cout << "Available commands:" << std::endl;
    for (const auto& command : commandMap)
    {
        std::cout << "  " << command.first << std::endl;
    }
}

/**
 * @brief Handle the exit command
 */
void Console::handleExit()
{
    std::cout << "Exiting..." << std::endl;
    stop();
}

/**
 * @brief Handle the history command
 */
void Console::handleHistory()
{
    std::cout << "Command history:" << std::endl;
    for (const auto& command : commandHistory)
    {
        std::cout << command << std::endl;
    }
}

/**
 * @brief Handle a custom command
 * @param command The custom command
 */
void Console::handleCustomCommand(const std::string &command)
{
    std::cout << "Custom command: " << command << std::endl;

    if (command.find(' ') != std::string::npos)
    {
        std::string customCommand = command.substr(0, command.find(" "));
        std::string customArgs = command.substr(command.find(" ") + 1);
        commandMap[customCommand] = [this, customArgs]() { handleCustomCommand(customArgs); };
    }
}

/**
 * @brief Execute a command
 * @param command The command to execute
 * @return The output of the command
 */
std::string Console::executeCommand(const std::string &command)
{
    std::stringstream output;
    std::streambuf* coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    processInput(command);
    std::cout.rdbuf(coutBuffer);
    return output.str();
}

/**
 * @brief Show the console
 */
void Console::showConsole()
{
    Initialize();
    run();
}

/**
 * @brief Execute a shell command
 * @param command The command to execute
 * @return The output of the command
 */
std::string Console::executeShellCommand(const std::string &command)
{
    std::array<char, 128> buffer{};
    std::string result;

#if defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
#endif
#if defined(__APPLE__)
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
#endif

    if (!pipe)
    {
        std::cerr << "popen() failed!" << std::endl;
        return "Error executing command";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    return result;
}
