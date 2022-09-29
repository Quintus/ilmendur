#ifndef ILMENDUR_TITLE_SCENE_HPP
#define ILMENDUR_TITLE_SCENE_HPP
#include "scene.hpp"
#include <string>
#include <vector>

class TitleScene: public Scene
{
public:
    TitleScene();
    virtual ~TitleScene();

    virtual void update();
    virtual void draw(SDL_Renderer* p_renderer);
private:
    void readUserMapList();
    void startGame(const std::string& mapname);
    void quitGame();
    std::vector<std::string> m_user_maps;
    size_t m_chosen_map;
};

#endif /* ILMENDUR_TITLE_SCENE_HPP */
