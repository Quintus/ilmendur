#ifndef ILMENDUR_MAP_CONTROLLER_HPP
#define ILMENDUR_MAP_CONTROLLER_HPP
#include <string>
#include <map>

class NonPlayableCharacter;
class Hero;

namespace MapControllers {

    class MapController
    {
    public:
        MapController(std::string map_name);
        virtual ~MapController();

        virtual void setup() {}; // Run when the map is entered

        static void createAllMapControllers();
        static void freeAllMapControllers();
        static bool findMapController(const std::string& name, MapController** pp_ctrl);
    protected:
        // All known map controllers.
        static std::map<std::string,MapController*> s_mcontrollers;

        // Use this function to get the NPC instance from a TMX ID.
        NonPlayableCharacter* findNPC(int id);

        // - Helper functions for common things to do in NPC functions -
        // void displayMessage(); ?? There is already stuff in GUI for that
    };

}

#endif /* ILMENDUR_MAP_CONTROLLER_HPP */
