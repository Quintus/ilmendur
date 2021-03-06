#include "window.hpp"
#include "application.hpp"
#include <GLFW/glfw3.h>
#include <OGRE/Ogre.h>
#include <cassert>

using namespace Core;

Window::Window(int width, int height, std::string title)
    : mp_glfw_window(nullptr),
      mp_ogre_window(nullptr)
{
    // Remember currently active OpenGL context
    GLFWwindow* p_prev_context = glfwGetCurrentContext();

    ////////////////////////////////////////
    // Create GLFW window

    // Mandate modern OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // TODO: Set X11 floating hint on Linux for tiling window managers
    // GLFW has functions to retrieve the native window and display.
    // Screen on today's system is always 0 (nobody uses X anymore to
    // render for multiple computers). The Screen* pointer itself can
    // be retrieved as ScreenOfDisplay(p_x11_display, XDefaultScreen(p_x11_display)),
    // where XDefaultScreen() will usually return said 0.

    // Note: glfwCreateWindow requires the window title to be UTF-8.
    mp_glfw_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!mp_glfw_window) {
        throw(std::runtime_error("Failed to create GLFW window!"));
    }

    glfwSetWindowSizeCallback(mp_glfw_window, Window::onWindowResize);

    // Hide system cursor, Imgui will draw a custom one instead.
    glfwSetInputMode(mp_glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    ////////////////////////////////////////
    // Create Ogre window

    // Temporaryly switch to the new GLFW window's context to
    // associate it with Ogre's window.
    glfwMakeContextCurrent(mp_glfw_window);

    // Configure Ogre to just use GLFW's already created OpenGL
    // context. The height/width/fullscreen parameters are thus
    // ignored.
    Ogre::NameValuePairList misc_params;
    misc_params["currentGLContext"]  = Ogre::String("true");
    misc_params["externalGLControl"] = Ogre::String("true");
    mp_ogre_window = Ogre::Root::getSingleton().createRenderWindow(title, width, height, false, &misc_params);

    // Switch back to whatever was current
    glfwMakeContextCurrent(p_prev_context);
}

Window::~Window()
{
    Ogre::Root::getSingleton().destroyRenderTarget(mp_ogre_window);
    glfwDestroyWindow(mp_glfw_window);
}


void Window::activate()
{
    glfwMakeContextCurrent(mp_glfw_window);
    mp_ogre_window->setVisible(true);
}

void Window::onWindowResize(GLFWwindow* p_win, int width, int height)
{
    /* Note: This function is not the place to enforce a certain aspect
     * ratio. Do this on the viewport level. The purpose of this function
     * is simply to make Ogre (and thereby OpenGL) aware of the actual
     * window size. */

    Window& instance = Application::getSingleton().getWindow();
    assert(instance.mp_glfw_window == p_win);
    instance.mp_ogre_window->resize(width, height);
}
