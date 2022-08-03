#include "os.hpp"
#include "buildconfig.hpp"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__unix__)
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#else
#error Unsupported system
#endif

using namespace std;
namespace fs = std::filesystem;

fs::path OS::gameDataDir()
{
#ifdef ILMENDUR_DEBUG_BUILD
    // Support running from the build directory in debug mode
    if (fs::exists(OS::exePath().parent_path() / fs::u8path("CMakeCache.txt"))) {
        return fs::u8path(ILMENDUR_SOURCE_DIR) / fs::u8path("data");
    } else {
#endif
        return fs::u8path(ILMENDUR_DATADIR);
#ifdef ILMENDUR_DEBUG_BUILD
    }
#endif
    return fs::path(ILMENDUR_DATADIR);
}

fs::path OS::exePath()
{
#if defined(__linux__)
    char buf[PATH_MAX];
    ssize_t size = readlink("/proc/self/exe", buf, PATH_MAX); // This file contains native encoding, whcih fits fs::path constructfor for "char"
    if (size < 0)
        throw(runtime_error("Failed to read /proc/self/exe"));
    return fs::path(string(buf, size));
#elif defined(_WIN32)
    wchar_t buf[MAX_PATH];
    if (GetModuleFileNameW(NULL, buf, MAX_PATH) == 0) {
        throw(runtime_error(winerror(GetLastError())));
    }

    return fs::path(buf); // Windows API returns native wide encoding, which fits fs::path constructor for "wchar_t"
#else
#error Unsupported platform
#endif
}
