#ifndef PRINTUTILS_H
#define PRINTUTILS_H

#include <cerrno>
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

/**
 * @brief Prints the error message corresponding to the last error that occurred.
 * @return A string containing the error message.
 */
inline std::string getErrorMessage()
{
#ifdef _WIN32
    DWORD error_code = GetLastError();
    char buf[256];
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, 0, buf, sizeof(buf), nullptr))
    {
        return std::string(buf);
    }
    else
    {
        return "Unknown error";
    }
#else
    return strerror(errno);
#endif
}

#endif // PRINTUTILS_H
