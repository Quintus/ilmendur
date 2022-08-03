#ifndef ILMENDUR_OS_HPP
#define ILMENDUR_OS_HPP

#include <string>
#include <filesystem>

namespace OS {
    std::filesystem::path exePath();

    std::filesystem::path gameDataDir();
}

#endif /* ILMENDUR_OS_HPP */
