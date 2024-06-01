#include "../include/CLI/Console.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#include <termios.h>
#include <cstdio>
#include <memory>
#include <array>
#include <future>
#endif

using namespace ConsoleInternals;

Console::Console() : running(false), historyIndex(0)
{
    commandMap["help"] = [this]() { handleHelp(); };
    commandMap["exit"] = [this]() { handleExit(); };
    commandMap["history"] = [this]() { handleHistory(); };
    commandMap["custom"] = [this]() { handleCustomCommand("custom"); };
}

Console::~Console()
{
    stop();
}

void Console::Initialize()
{
#if defined(_WIN32)
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#elif defined(__APPLE__) || defined(__linux__)
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

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

void Console::displayPrompt()
{
    std::cout << "> " << std::flush;
}

void Console::handleHelp()
{
    std::cout << "Available commands:" << std::endl;
    for (const auto& command : commandMap)
    {
        std::cout << "  " << command.first << std::endl;
    }
}

void Console::handleExit()
{
    std::cout << "Exiting..." << std::endl;
    stop();
}

void Console::handleHistory()
{
    std::cout << "Command history:" << std::endl;
    for (const auto& command : commandHistory)
    {
        std::cout << command << std::endl;
    }
}

void Console::handleCustomCommand(const std::string &command)
{
    std::cout << "Custom command: " << command << std::endl;

    if (command.find(" ") != std::string::npos)
    {
        std::string customCommand = command.substr(0, command.find(" "));
        std::string customArgs = command.substr(command.find(" ") + 1);
        commandMap[customCommand] = [this, customArgs]() { handleCustomCommand(customArgs); };
    }
}

std::string Console::executeCommand(const std::string &command)
{
    std::stringstream output;
    std::streambuf* coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    processInput(command);
    std::cout.rdbuf(coutBuffer);
    return output.str();
}

void Console::showConsole()
{
    Initialize();
    run();
}

std::string Console::executeShellCommand(const std::string &command)
{
    std::array<char, 128> buffer;
    std::string result;

#if defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
#elif defined(__APPLE__) || defined(__linux__)
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
