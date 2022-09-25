#ifndef ILMENDUR_SCENE_HPP
#define ILMENDUR_SCENE_HPP
#include <SDL2/SDL.h>

class Scene
{
public:
    Scene();
    virtual ~Scene();

    virtual void update() = 0; // Update scene actors. Also call ImGui methods here, not in draw().
    virtual void draw(SDL_Renderer* p_renderer) = 0; // Draw the state as left by update(). ImGui elements created by update() will be drawn on top of this after draw() completes.
    virtual void handleKeyDown(const SDL_Event&) {};
    virtual void handleKeyUp(const SDL_Event&) {};
};

#endif /* ILMENDUR_SCENE_HPP */
