#ifndef RPG_OS_PATHS_HPP
#define RPG_OS_PATHS_HPP
#include <filesystem>

namespace OS {

    // Functions for retrieving system-specific filesystem pathes.
    std::filesystem::path exe_path();  //< Path to running executable.
    std::filesystem::path home_dir();  //< Path to user home directory
    std::filesystem::path saves_dir(); //< Path to savegame slot directory
    std::filesystem::path slot2path(unsigned int slot); //< Full file path to a slot's savegame file
}

#endif /* RPG_OS_PATHS_HPP */
