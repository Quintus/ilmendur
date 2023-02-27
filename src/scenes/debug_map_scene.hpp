#ifndef ILMENDUR_DEBUG_MAP_SCENE_HPP
#define ILMENDUR_DEBUG_MAP_SCENE_HPP
#include "scene.hpp"
#include <string>

class Hero;
class Actor;
class Camera;
class Map;

class DebugMapScene: public Scene
{
public:
    DebugMapScene(const std::string& map);
    virtual ~DebugMapScene();

    virtual void setup();
    virtual void update();
    virtual void draw(SDL_Renderer* p_renderer);
    virtual void handleKeyDown(const SDL_Event& event);
    virtual void handleKeyUp(const SDL_Event& event);

    Hero* freya() { return mp_freya; }
    Hero* benjamin() { return mp_benjamin; }

    void useEntry(int id);

    inline Map& map() { return *mp_map; }
private:
    Camera* mp_cam1;
    Camera* mp_cam2;
    Map* mp_map;

    Hero* mp_freya;
    Hero* mp_benjamin;
private:
    int m_teleport_entry;
};

#endif /* ILMENDUR_DEBUG_MAP_SCENE_HPP */
