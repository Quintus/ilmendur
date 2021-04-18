#include "os.hpp"

#if defined(__unix__)
#include <cstring>
#else
#error Unsupported system
#endif

using namespace std;

/// Returns a human-readable error message for errno code `errnum`.
string OS::errnoerrstr(int errnum)
{
#ifdef __unix__
    char buf[4096] = {0}; // Should be enough for basically all error messages
    if (strerror_r(errnum, buf, 4095 /* term. NUL */) != 0) {
        return string("Error on errno retrieval: ") + to_string(errno);
    }
    else {
        return buf;
    }
#else
#error Unsupported system
#endif
}
