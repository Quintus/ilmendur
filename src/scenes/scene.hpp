#ifndef ILMENDUR_SCENE_HPP
#define ILMENDUR_SCENE_HPP
#include <SDL2/SDL.h>

class Scene
{
public:
    Scene();
    virtual ~Scene();

    virtual void setup(); // Optional code to run once when the scene has been placed on top of the stack for the first time
    virtual void update() = 0; // Update scene actors. Also call ImGui methods here, not in draw().
    virtual void draw(SDL_Renderer* p_renderer) = 0; // Draw the state as left by update(). ImGui elements created by update() will be drawn on top of this after draw() completes.
    virtual void handleKeyDown(const SDL_Event&) {};
    virtual void handleKeyUp(const SDL_Event&) {};

    inline bool isSetUp() const { return m_is_set_up; } // True if setup() has been run
private:
    bool m_is_set_up;
};

#endif /* ILMENDUR_SCENE_HPP */
