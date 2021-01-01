#ifndef APPLICATION_HPP
#define APPLICATION_HPP
#include <vector>

// Forward declarations
struct GLFWwindow;
namespace Ogre {
    class Plugin;
    class SceneManager;
    class RenderWindow;
}
class SGTechniqueResolverListener;

/// Simple struct that stores the GLFW window along with the associated
/// Ogre window.
struct WindowPair
{
    GLFWwindow* glfw_window {nullptr};
    Ogre::RenderWindow* ogre_window {nullptr};
};

class Application
{
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

    WindowPair m_windowpair;
    std::vector<Ogre::Plugin*> m_ogre_plugins;
    SGTechniqueResolverListener* mp_sglistener;
};

#endif /* APPLICATION_HPP */
