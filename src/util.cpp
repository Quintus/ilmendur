#include "util.hpp"
#include <cstdarg>
#include <cstring>
#include <cassert>

std::string format(const char* source, ...)
{
    std::string target("\0", strlen(source) + 50); // Educated guess: The target will be a little longer than the source.

    int result = target.size();
    va_list ap;
    va_list ap2;
    va_start(ap, source);

    do {
        target.resize(result+1);
        va_copy(ap2, ap);
        result = vsnprintf(target.data(), target.size(), source, ap2);
        va_end(ap2);
    } while (static_cast<size_t>(result) >= target.size());

    va_end(ap);
    return std::string(target.c_str()); // Cut off anything after the terminal NUL of `target'
}
