#include "encoding.hpp"
#include "os.hpp"
#include <stdexcept>

using namespace std;
using namespace OS;

#if defined(__WIN32)
// Nothing; convert_encodings() is implemented in encoding.hpp as a templated function.

#elif defined(__unix__)
#include <cstring>
#include <iconv.h>

/**
 * This function converts the given string from the given source encoding to
 * another given target encoding. Please note that its function signature
 * looks different on Windows (see header file).
 *
 * If to and from encodings are identical (after resolving the
 * "filesystem" alias), the function returns a copy of
 * `source_string', hence calling it without that check is cheap.
 *
 * This function uses the encoding conversion functions built into
 * the respective operating systems.
 *
 * On Windows, this function can only convert between UTF-8 and UTF-16.
 */
string convert_encodings(const string& from_encoding, const string& to_encoding, const string& source_str)
{
    // Short circuit if to and from encoding are identical
    if (to_encoding == from_encoding) {
        return source_str;
    }

    int errsav = 0;
    size_t input_length = source_str.length();

    // Working copy that isn’t const
    char* copy = (char*) malloc(input_length + 1); // Terminating NUL
    strcpy(copy, source_str.c_str());

    // Set up the encoding converter
    iconv_t converter    = iconv_open(to_encoding.c_str(), from_encoding.c_str());
    size_t outbytes_left = 0;
    size_t inbytes_left  = input_length;

    if (converter == (iconv_t) -1) {
        errsav = errno;
        free(copy);
        throw runtime_error(errnoerrstr(errsav));
    }

    /* There is no way to know how much space iconv() will need. So keep
     * allocating more and more memory as needed. `current_size' keeps track
     * of how large our memory blob is currently. `outbuf' is the pointer to
     * that memory blob. */
    size_t current_size = input_length + 1; // NUL
    char* outbuf        = NULL;
    char* inbuf         = copy; // Copy the pointer, iconv() increments its *argument

    errsav = 0;
    outbytes_left = current_size;
    while(true) {
        outbuf         = (char*) realloc(outbuf - (current_size - outbytes_left), current_size + 10);
        current_size  += 10;
        outbytes_left += 10;

        errno  = 0;
        errsav = 0;

        iconv(converter, &inbuf, &inbytes_left, &outbuf, &outbytes_left); // sets outbytes_left to 0 or very low values if not enough space (E2BIG)
        errsav = errno;

        if (errsav != E2BIG) {
            break;
        }
    }

    iconv_close(converter);
    free(copy);

    size_t count = current_size - outbytes_left;
    outbuf -= count; // iconv() advances the pointer!

    if (errsav != 0) {
        free(outbuf);
        throw(runtime_error(errnoerrstr(errsav)));
    }

    string result(outbuf, count);
    free(outbuf);

    return result;
}

#else
#error Unsupported system
#endif
