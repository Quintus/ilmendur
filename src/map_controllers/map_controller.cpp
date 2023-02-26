#include "map_controller.hpp"
#include "../ilmendur.hpp"
#include "../map.hpp"
#include "../actors/npc.hpp"
#include "../scenes/debug_map_scene.hpp"
#include "oak_fortress.hpp"

#include <cassert>

using namespace std;

std::map<std::string,MapControllers::MapController*> MapControllers::MapController::s_mcontrollers;

/**
 * Constructs a new map controller. Pass in the name of map
 * it is meant to control, without a directory and without
 * the trailing ".tmx", that is, only the bare file name stem.
 *
 * The constructors of MapController subclasses are run early
 * on game startup, so do depend on much in it. To run code
 * on entering the map, override setup().
 */
MapControllers::MapController::MapController(std::string map_name)
{
    s_mcontrollers[map_name] = this;
}

MapControllers::MapController::~MapController()
{
}

/**
 * Tracks down the instance of NonPlayableCharacter for the TMX
 * ID given and returns it, or `nullptr` if there is no such
 * NPC. Note that this will return `nullptr` if the ID does
 * exist, but does not refer to an NPC, such as a collbox.
 * If you really want that one, use Map::findActor() directly,
 * which is wrapped by this function for ease of use.
 */
NonPlayableCharacter* MapControllers::MapController::findNPC(int id)
{
    Scene* p_scene = &Ilmendur::instance().currentScene();
    DebugMapScene* p_mapscene = dynamic_cast<DebugMapScene*>(p_scene);
    assert(p_mapscene); // findNPC() must not be called outside a map scene, so this should never trigger

    Actor* p_actor = nullptr;
    p_mapscene->map().findActor(id, &p_actor);

    NonPlayableCharacter* p_npc = dynamic_cast<NonPlayableCharacter*>(p_actor);
    return p_npc; // nullptr if it is not an NPC
}

/**
 * Searches for a map controller for the map with the given name
 * (filename stem; no directory, no .tmx file extension), and
 * returns true if it was found, returning the controller
 * in `*pp_ctrl'. Otherwise returns false.
 */
bool MapControllers::MapController::findMapController(const std::string& name, MapController** pp_ctrl)
{
    if (s_mcontrollers.count(name) > 0) {
        *pp_ctrl = s_mcontrollers.at(name);
        return true;
    } else {
        return false;
    }
}

/**
 * Keep this function last in this file. This runs during game
 * startup. It has to fill in the s_mcontrollers variable
 * with a mapping of "map name" => MapController subclass instance.
 * The Map class will look into that variable when trying to
 * attach a map controller. The map name is the file name stem,
 * that is the filename of the map without directory information
 * and without the ".tmx" file extension.
 */
void MapControllers::MapController::createAllMapControllers()
{
    s_mcontrollers["Oak Fortress"] = new MapControllers::OakFortress();
}

/**
 * Frees all map controllers. Runs during game shutdown.
 */
void MapControllers::MapController::freeAllMapControllers()
{
    for (auto p: s_mcontrollers) {
        delete p.second;
    }
}
