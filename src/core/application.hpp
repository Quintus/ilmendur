#ifndef APPLICATION_HPP
#define APPLICATION_HPP
#include <vector>
#include <stack>
#include <memory>

// Library forward declarations
namespace Ogre {
    class Plugin;
}

class Scene;

namespace Core {

    // More forward declarations
    class SGTechniqueResolverListener;
    class Window;

    class Application {
    public:
        static Application& getSingleton();

        Application();
        ~Application();

        Window& getWindow();

        void run();
    private:
        void setupGlfw();
        void shutdownGlfw();
        void setupOgre();
        void shutdownOgre();
        void loadOgrePlugins();
        void setupOgreOverlaySystem();
        void loadOgreResources();
        void setupOgreRTSS();
        void shutdownOgreRTSS();

        Window* mp_window;
        SGTechniqueResolverListener* mp_sglistener;
        std::vector<std::unique_ptr<Ogre::Plugin>> m_ogre_plugins;
        std::stack<std::unique_ptr<Scene>> m_scene_stack;
    };

}

#endif /* APPLICATION_HPP */
