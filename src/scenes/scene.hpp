#ifndef ILMENDUR_SCENE_HPP
#define ILMENDUR_SCENE_HPP
#include <SDL2/SDL.h>

class Scene
{
public:
    Scene();
    virtual ~Scene();

    virtual void update() = 0;
    virtual void draw(SDL_Renderer* p_renderer) = 0;
};

#endif /* ILMENDUR_SCENE_HPP */
