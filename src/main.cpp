#include <iostream>

#if defined(_WIN32)
#include "windows.cpp"
#endif

#if defined(__APPLE__)
#include "macos.mm"
#endif

#if defined(__linux__)
#include "linux.cpp"
#endif