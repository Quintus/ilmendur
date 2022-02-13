#include "save_system.hpp"
#include "../core/game_state.hpp"
#include "../os/paths.hpp"
#include <buildconfig.hpp>
#include <fstream>
#include <string>

// Remove this once OpenSUSE shops GCC >= 8
#if defined(__GNUC__) && __GNUC__ < 8
#include <experimental/filesystem>
namespace std {
    namespace filesystem = std::experimental::filesystem;
}
#else
#include <filesystem>
#endif

using namespace std;

// TODO: Currently simply dumps the memory occupied by the GameState
// structure. This is not ideal: if something is added to that structure,
// all already stored save games would be invalidated (binary incompatibility).
void SaveSystem::load(unsigned int slot, Core::GameState& gs)
{
    ifstream savefile(OS::slot2path(slot), ios::in | ios::binary);
    savefile.read(reinterpret_cast<char*>(&gs), sizeof(Core::GameState));
    savefile.close();
}

void SaveSystem::save(unsigned int slot, const Core::GameState& gs)
{
    ofstream savefile(OS::slot2path(slot), ios::out | ios::binary | ios::trunc);
    savefile.write(reinterpret_cast<const char*>(&gs), sizeof(Core::GameState));
    savefile.close();
}
