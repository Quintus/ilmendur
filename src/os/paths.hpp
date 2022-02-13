#ifndef ILMENDUR_OS_PATHS_HPP
#define ILMENDUR_OS_PATHS_HPP

// Remove this once OpenSUSE shops GCC >= 8
#if defined(__GNUC__) && __GNUC__ < 8
#include <experimental/filesystem>
namespace std {
    namespace filesystem = std::experimental::filesystem;
}
#else
#include <filesystem>
#endif

namespace OS {

    // Functions for retrieving system-specific filesystem pathes.
    std::filesystem::path exe_path();  //< Path to running executable.
    std::filesystem::path home_dir();  //< Path to user home directory
    std::filesystem::path saves_dir(); //< Path to savegame slot directory
    std::filesystem::path slot2path(unsigned int slot); //< Full file path to a slot's savegame file

    std::filesystem::path game_resource_dir(); //< Path to game data resource directory
    std::filesystem::path ogre_resource_dir(); //< Path to directory with ogre-internal resources
}

#endif /* ILMENDUR_OS_PATHS_HPP */
