#ifndef APPLICATION_HPP
#define APPLICATION_HPP
#include <vector>
#include <stack>
#include <memory>

// Forward declarations
namespace Ogre {
    class Plugin;
    class SceneManager;
}
class SGTechniqueResolverListener;
class Window;
class Scene;

class Application {
public:
    static Application& getSingleton();

    Application();
    ~Application();

    Window* getWindow();

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
    std::vector<std::unique_ptr<Ogre::Plugin>> m_ogre_plugins;
    SGTechniqueResolverListener* mp_sglistener;
    std::stack<std::unique_ptr<Scene>> m_scene_stack;
};

#endif /* APPLICATION_HPP */
