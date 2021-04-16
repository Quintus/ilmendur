#include "game_state.hpp"
#include <buildconfig.hpp>
#include <filesystem>
#include <fstream>
#include <string>

using namespace std;
namespace fs = std::filesystem;

GameState GameState::instance;

static fs::path slot2path(unsigned int slot)
{
#pragma GCC warning "Implement proper path model"
    return fs::path("/tmp/test-rpg-saves") / (to_string(slot) + ".sav");
}

// TODO: Currently simply dumps the memory occupied by the GameState
// structure. This is not ideal: if something is added to that structure,
// all already stored save games would be invalidated (binary incompatibility).
void GameState::load(unsigned int slot)
{
    ifstream savefile(slot2path(slot), ios::in | ios::binary);
    savefile.read(reinterpret_cast<char*>(&GameState::instance), sizeof(GameState));
    savefile.close();
}

void GameState::save(unsigned int slot)
{
    ofstream savefile(slot2path(slot), ios::out | ios::binary | ios::trunc);
    savefile.write(reinterpret_cast<char*>(this), sizeof(GameState));
    savefile.close();
}
