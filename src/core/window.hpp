#ifndef WINDOW_HPP
#define WINDOW_HPP
#include <string>
#include <OGRE/Ogre.h>

// Library forward declarations
struct GLFWwindow;

namespace Core {

    /**
     * This class binds together Ogre's notion of a native window
     * and the actual GLFW native window. It ensurse that any
     * operations carried out always affect both windows so that
     * they might not go out of sync.
     */
    class Window {
    public:
        Window(int width, int height, std::string title);
        ~Window();

        void activate();

        inline GLFWwindow* getGLFWWindow() { return mp_glfw_window; }
        inline Ogre::RenderWindow* getOgreRenderWindow() { return mp_ogre_window; }
    private:
        GLFWwindow* mp_glfw_window;
        Ogre::RenderWindow* mp_ogre_window;
    };

}

#endif /* WINDOW_HPP */
