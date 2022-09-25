#ifndef ILMENDUR_TITLE_SCENE_HPP
#define ILMENDUR_TITLE_SCENE_HPP
#include "scene.hpp"

class TitleScene: public Scene
{
public:
    TitleScene();
    virtual ~TitleScene();

    virtual void update();
    virtual void draw(SDL_Renderer* p_renderer);
private:
    void startGame();
    void quitGame();
};

#endif /* ILMENDUR_TITLE_SCENE_HPP */
