#ifndef APPLICATION_HPP
#define APPLICATION_HPP
#include <vector>

// Forward declarations
namespace Ogre {
    class Plugin;
    class SceneManager;
}
class SGTechniqueResolverListener;
class Window;

class Application {
public:
    static Application* getSingleton();

    Application();
    ~Application();

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

    // For debugging purposes
    void _make_a_scene();
    void _destroy_the_scene();
    Ogre::SceneManager* _mp_scene_manager;

    Window* mp_window;
    std::vector<Ogre::Plugin*> m_ogre_plugins;
    SGTechniqueResolverListener* mp_sglistener;
};

#endif /* APPLICATION_HPP */
