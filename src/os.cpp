#include "os.hpp"
#include "buildconfig.hpp"

using namespace std;
namespace fs = std::filesystem;

fs::path OS::game_data_dir()
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
    return fs::path(ILMENDUR_DATADIR);
}
