#ifndef ILMENDUR_SCENE_HPP
#define ILMENDUR_SCENE_HPP
#include <vector>
#include <SDL2/SDL.h>

class Player;
class Actor;
class Camera;
class Map;

class Scene
{
public:
    Scene();
    ~Scene();

    void update();
    void draw(SDL_Renderer* p_renderer);

    inline void setPlayer(Player* p_player) { mp_player = p_player; }
    inline Map& map() { return *mp_map; }
private:
    Camera* mp_cam1;
    Camera* mp_cam2;
    Map* mp_map;
    Player* mp_player;
};

#endif /* ILMENDUR_SCENE_HPP */
