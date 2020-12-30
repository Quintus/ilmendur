#include <GLFW/glfw3.h>

int main(int argc, char* argv[])
{
    if (!glfwInit())
        return -1;

    GLFWwindow* p_window = glfwCreateWindow(640, 480, "Hello world", nullptr, nullptr);
    if (!p_window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(p_window);

    while (!glfwWindowShouldClose(p_window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(p_window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
