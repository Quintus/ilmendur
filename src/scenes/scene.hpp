#ifndef ILMENDUR_SCENE_HPP
#define ILMENDUR_SCENE_HPP
#include <string>
#include <OGRE/Ogre.h>

namespace PhysicsSystem {
    class PhysicsEngine;
}

enum class entity_type;

namespace SceneSystem {

    class Scene
    {
    public:
        Scene(const std::string& name);
        virtual ~Scene();

        virtual void activate();
        virtual void deactivate();

        const std::string& getName() { return m_name; }

        bool hasPhysics() const { return mp_physics != nullptr; }
        PhysicsSystem::PhysicsEngine* getPhysicsEngine() const { return mp_physics; }
        Ogre::SceneManager& getSceneManager() const { return *mp_scene_manager; }

        virtual void processKeyInput(int key, int scancode, int action, int mods);
        virtual void processCharInput(unsigned int codepoint);
        virtual void processCursorMove(double xpos, double ypos);
        virtual void processMouseButton(int button, int action, int mods);
        virtual void update() {};
        virtual void draw() {};

        void finish(Scene* p_next = nullptr);
        bool isFinishing();

        Scene* nextScene(); // Private API for use in Application::run(). Do not use.
    protected:
        PhysicsSystem::PhysicsEngine* mp_physics;
        Ogre::SceneManager* mp_scene_manager;
        void replaceBlenderEntities(Ogre::SceneNode* p_scene_node);
        void replaceBlenderEntity(Ogre::SceneNode* p_scene_node, entity_type etype);
    private:
        std::string m_name;
        bool m_finish;
        Scene* mp_next_scene;
    };

}

#endif /* ILMENDUR_SCENE_HPP */
