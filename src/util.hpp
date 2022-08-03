#ifndef ILMENDUR_UTIL_HPP
#define ILMENDUR_UTIL_HPP

/// Reads the entire file `file' (an std::ifstream) and returns it as a string.
#define READ_FILE(file) std::istreambuf_iterator<char>(file.rdbuf()), std::istreambuf_iterator<char>()

#endif /* ILMENDUR_UTIL_HPP */
