#ifndef RPG_SCENE_HPP
#define RPG_SCENE_HPP
#include <string>

namespace Ogre {
    class SceneManager;
}

class Scene
{
public:
    Scene(const std::string& name);
    virtual ~Scene();

    const std::string& getName() { return m_name; }

    virtual void update() {};

    void finish();
    bool isFinishing();
protected:
    Ogre::SceneManager* mp_scene_manager;
private:
    std::string m_name;
    bool m_finish;
};

#endif /* RPG_SCENE_HPP */
