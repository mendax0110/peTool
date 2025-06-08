#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

/// @brief CLiInterface namespace, which contains the CLI class for command \namespace CliInterface
namespace CliInterface
{
    /// @brief CLI class, which provides methods for command line interface operations \class CLI
    class CLI
    {
    public:

        /**
         * @brief Print the help message for the CLI.
         */
        static void printHelp();

        /**
         * @brief Start the CLI with the given arguments.
         * @param argc The number of arguments.
         * @param argv The array of arguments.
         */
        static void startCli(int argc, char** argv);

        /**
         * @brief Prints Error message to the console.
         * @param message The error message to print.
         */
        static void printError(const std::string& message);

        /**
         * @brief Prints a message to the console.
         * @param message The message to print.
         */
        static void printMessage(const std::string& message);

    private:

    protected:
    };
}