#include "oak_fortress.hpp"
#include "../gui.hpp"
#include "../actors/npc.hpp"
#include <cassert>

#define SOME_ACTOR_ID 39

MapControllers::OakFortress::OakFortress()
    : MapControllers::MapController("Oak Fortress")
{
}

MapControllers::OakFortress::~OakFortress()
{
}

void MapControllers::OakFortress::setup()
{
    MapController::setup();

    findNPC(SOME_ACTOR_ID)->attachActivationFunction(clownGuy);
    // activateFunction runs when the NPC is "spoken to" (user presses RETURN).
}

void MapControllers::OakFortress::clownGuy(NonPlayableCharacter* p_npc, Player* p_hero)
{
    assert(p_npc->id() == SOME_ACTOR_ID);

    GUISystem::messageDialog(1, GUISystem::text_velocity::normal, "Test NPC",
                             {"I am a test NPC, and I am now going to make a step to the right."},
                             [p_npc] {p_npc->moveRelative(direction::right);});

    // Some pseudo code for what the NPC could possibly do:
    // if (Ilmendur::instance().saveState.globals.archievments.bossAbcBeaten) {
    //     p_npc->moveRelative(Vector2f(1.0f, 1.0f));
    // } else {
    //     p_npc->turn(direction::down);
    //     displayMessage("No!");
    // }
}
