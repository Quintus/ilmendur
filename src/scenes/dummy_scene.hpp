#ifndef RPG_DUMMY_SCENE_HPP
#define RPG_DUMMY_SCENE_HPP
#include "scene.hpp"

class DummyScene: public Scene
{
public:
    DummyScene();
    virtual ~DummyScene();

    virtual void update();
};

#endif /* RPG_DUMMY_SCENE_HPP */
