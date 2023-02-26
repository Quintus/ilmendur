#include "oak_fortress.hpp"
#include "../gui.hpp"
#include "../actors/npc.hpp"
#include <cassert>

#define SOME_ACTOR_ID 38

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
                             {"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, <em>sed diam nonumy</em> eirmod tempor <em>invidunt</em> ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.",
                              "Second Message: Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna.",
                              format("Third Message: >%s<", "Test format")},
                             [&] {});

    // Some pseudo code for what the NPC could possibly do:
    // if (Ilmendur::instance().saveState.globals.archievments.bossAbcBeaten) {
    //     p_npc->moveRelative(Vector2f(1.0f, 1.0f));
    // } else {
    //     p_npc->turn(direction::down);
    //     displayMessage("No!");
    // }
}
