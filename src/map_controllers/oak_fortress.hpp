#ifndef ILMENDUR_OAK_FORTRESS_HPP
#define ILMENDUR_OAK_FORTRESS_HPP
#include "map_controller.hpp"

namespace MapControllers
{
    class OakFortress: public MapController
    {
    public:
        OakFortress();
        virtual ~OakFortress();
        virtual void setup();

    private:
        // -- NPC Activation functions follow --
        static void clownGuy(NonPlayableCharacter* p_npc, Player* p_hero);
    };
}

#endif /* ILMENDUR_OAK_FORTRESS_HPP */
