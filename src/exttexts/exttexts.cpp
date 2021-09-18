#include "exttexts.hpp"
#include <cassert>

using namespace std;

/**
 * The actual array of external texts used in Blender. The array
 * indices are the values for the `il_text` property in Blender,
 * where index 0 is not to be used.
 */
static const char* externalTexts[] = {
    "", // ID=0 should never be used
    "Hello world! This is a test sign."
};

/**
 * This function returns the external text for the given `id`.
 * The `id` parameter corresponds to the `il_text` property
 * in Blender scenes. Its value must be >= 1.
 */
std::string ExternalText::fetchExternalText(unsigned long id)
{
    assert(id > 0);
    return externalTexts[id];
}
