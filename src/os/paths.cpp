#include "paths.hpp"
#include <buildconfig.hpp>
#include <string>
#include <iostream>

#if defined(__GNUC__) && __GNUC__ < 8
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix__)
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#else
#error Unsupported system
#endif

using namespace std;
namespace fs = std::filesystem;

#ifdef __linux
enum class xdg {
    data_home,
    config_home,
    cache_home,
    runtime_dir
};

// Returns XDG directories from environment variables as per
// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
// If the environment variable does not exist, returns default path
// values as per the XDG specification.
static fs::path xdgdir(xdg directory)
{
    char* envval = nullptr;

    switch (directory) {
    case xdg::data_home:
        envval = getenv("XDG_DATA_HOME");
        break;
    case xdg::config_home:
        envval = getenv("XDG_CONFIG_HOME");
        break;
    case xdg::cache_home:
        envval = getenv("XDG_CACHE_HOME");
        break;
    case xdg::runtime_dir:
        envval = getenv("XDG_RUNTIME_DIR");
        break;
    }

    if (envval) {
        return fs::path(envval); // Environment is in native narrow encoding, which fits fs::path constructor
    }
    else { // Default directories as per XDG spec
        fs::path result;
        switch (directory) {
        case xdg::data_home:
            result = OS::home_dir() / fs::u8path(".local/share");
            break;
        case xdg::config_home:
            result = OS::home_dir() / fs::u8path(".config");
            break;
        case xdg::cache_home:
            result = OS::home_dir() / fs::u8path(".cache");
            break;
        case xdg::runtime_dir: // Warning is recommended by XDG spec
            cout << "Warning: XDG_RUNTIME_DIR is not set in the environment. Using temporary directory instead." << endl;
            result = fs::temp_directory_path();
            break;
        }
        return result;
    }
}
#endif

fs::path OS::home_dir()
{
#if defined(__unix__)
    char* envval = getenv("HOME");
    if (envval) {
        return fs::path(envval); // Environment is in native narrow encoding, which fits fs::path constructor for "char"
    }
    else {
        /* POSIX.1-2008 mandates that $HOME is set in the environment:
         * https://pubs.opengroup.org/onlinepubs/9699919799.2008edition/basedefs/V1_chap08.html
         * If it is not, one can assume the system has severere problems and
         * it should be okay to crash. */
        throw runtime_error("Environment variable HOME is not defined");
    }
#elif defined(_WIN32)
    wchar_t homedir[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, homedir) != S_OK)
        throw(std::runtime_error("Home directory not defined."));

    return fs::path(homedir); // Windows API returns native wide encoding, which fits fs::path constructor for "wchar_t"
#else
#error Unsupported system
#endif
}

fs::path OS::exe_path()
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

fs::path OS::saves_dir()
{
#if defined (__linux__)
    return xdgdir(xdg::data_home) / fs::u8path("ilmendur/saveslots");
#else
#error Unsupported system
#endif
}

fs::path OS::slot2path(unsigned int slot)
{
    return OS::saves_dir() / (to_string(slot) + ".sav");
}

fs::path OS::game_resource_dir()
{
#ifdef ILMENDUR_DEBUG_BUILD
    // Support running from the build directory in debug mode
    if (fs::exists(OS::exe_path().parent_path() / fs::u8path("CMakeCache.txt"))) {
        return fs::u8path(ILMENDUR_SOURCE_DIR) / fs::u8path("data");
    } else {
#endif
        return fs::u8path(ILMENDUR_DATADIR);
#ifdef ILMENDUR_DEBUG_BUILD
    }
#endif
}

fs::path OS::ogre_resource_dir()
{
#ifdef ILMENDUR_DEBUG_BUILD
    // Support running from the build directory in debug mode.
    // Note that Ogre's internal resources in this case are only available
    // within the extracted downloaded ogre tarball.
    if (fs::exists(OS::exe_path().parent_path() / fs::u8path("CMakeCache.txt"))) {
        return OS::exe_path().parent_path() / fs::u8path("deps-source/ogre/Media"); // = ${CMAKE_BINARY_DIR}/deps-source/ogre/Media
    }
    else {
#endif
        return OS::game_resource_dir() / fs::u8path("ogre");
#ifdef ILMENDUR_DEBUG_BUILD
    }
#endif
}

fs::path OS::translations_dir()
{
#ifdef ILMENDUR_DEBUG_BUILD
    // Support running from the build directory in debug mode.
    // The gettext-catalogues directoy is filled during the built process
    // by the `podev' target.
    if (fs::exists(OS::exe_path().parent_path() / fs::u8path("CMakeCache.txt"))) {
        return OS::exe_path().parent_path() / fs::u8path("gettext-catalogues");
    } else {
#endif
        return OS::game_resource_dir() / fs::u8path("translations");
#ifdef ILMENDUR_DEBUG_BUILD
    }
#endif
}
