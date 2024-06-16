#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

/**
 * @brief The CLI class
 * @namespace CliInterface
 */
namespace CliInterface
{
    /**
     * @brief The CLI class
     * @class CLI used for command line interface
     */
    class CLI
    {
    public:
        static void printHelp();
        static void startCli(int argc, char** argv);
        static void printError(const std::string& message);
        static void printMessage(const std::string& message);

    private:

    protected:
    };
}