#include "os.hpp"
#include "buildconfig.hpp"
#include <stdexcept>

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

fs::path OS::userDataDir()
{
#if defined(__linux__)
    // See https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    // This will also work inside a flatpak, because Flatpak sets $XDG_DATA_HOME explicitely.
    string xdg_data_home(getenv("XDG_DATA_HOME"));
    if (xdg_data_home.empty()) {
        string homedir(getenv("HOME"));
        if (homedir.empty()) {
            throw(std::runtime_error("Neither $XDG_DATA_HOME nor $HOME is set in the environment"));
        }
        xdg_data_home = homedir + "/.local/share";
    }

    return fs::path(xdg_data_home) /* getenv() returns native encoding, not necessaryly UTF-8 */
        / fs::u8path("ilmendur");
#else
#error Unsupported system
#endif
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
