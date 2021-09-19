#ifndef ILMENDUR_SCENE_HPP
#define ILMENDUR_SCENE_HPP
#include <string>

namespace Ogre {
    class SceneManager;
    class SceneNode;
}

enum class entity_type;

namespace SceneSystem {

    class Scene
    {
    public:
        Scene(const std::string& name);
        virtual ~Scene();

        const std::string& getName() { return m_name; }

        virtual void processKeyInput(int key, int scancode, int action, int mods) {};
        virtual void update() {};

        void finish();
        bool isFinishing();
    protected:
        Ogre::SceneManager* mp_scene_manager;
        void replaceBlenderEntities(Ogre::SceneNode* p_scene_node);
        void replaceBlenderEntity(Ogre::SceneNode* p_scene_node, entity_type etype);
    private:
        std::string m_name;
        bool m_finish;
    };

}

#endif /* ILMENDUR_SCENE_HPP */
