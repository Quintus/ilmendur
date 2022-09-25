#ifndef ILMENDUR_DEBUG_MAP_SCENE_HPP
#define ILMENDUR_DEBUG_MAP_SCENE_HPP
#include "scene.hpp"
#include <string>

class Player;
class Actor;
class Camera;
class Map;

class DebugMapScene: public Scene
{
public:
    DebugMapScene(const std::string& map);
    virtual ~DebugMapScene();

    virtual void update();
    virtual void draw(SDL_Renderer* p_renderer);

    inline void setPlayer(Player* p_player) { mp_player = p_player; }
    inline Map& map() { return *mp_map; }
private:
    Camera* mp_cam1;
    Camera* mp_cam2;
    Map* mp_map;
    Player* mp_player;
};

#endif /* ILMENDUR_DEBUG_MAP_SCENE_HPP */
