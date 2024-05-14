#include "window.hpp"
#include "engine.hpp"
#include "framebuffer.hpp"
#include "log.hpp"
#include "gui.hpp"

#include "shader.hpp"
#include "texture.hpp"
#include "vbo.hpp"

#include <chrono>
#include <thread>


Window::Window(uint32_t width, uint32_t height, uint32_t offset_x, uint32_t offset_y, bool fullscreen)

    : fullscreen(fullscreen), Output(width, height) /*, fps("Window")*/ {

    int8_t windows_border = 0;
    int8_t window_on_top = 1;
    int8_t cursor_visibility = 0;
    int8_t uid_callbacks = 1;


    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DECORATED, (windows_border ? GLFW_TRUE : GLFW_FALSE));
    glfwWindowHint(GLFW_FLOATING, (window_on_top ? GLFW_TRUE : GLFW_FALSE));

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[0]);
    uint32_t refreshRate = glfwGetVideoMode(monitors[0])->refreshRate;

    displays.push_back(Display(mode->width, mode->height,refreshRate));
    PLOGI  << " Display @ " << refreshRate << "Hz " << mode->width << "x" << mode->height;

    if (fullscreen)
    {
        auto monitor = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
//        window = glfwCreateWindow(mode->width, mode->height, "OUTPUT", monitor, nullptr);
        id = glfwCreateWindow(1920, 1200, "OUTPUT", monitor, nullptr);
        cursor_visibility = false;
    }
    else
    {

        id = glfwCreateWindow(width, height, "OUTPUT", nullptr, nullptr);

    }


    glfwSetWindowUserPointer(id, this);
#ifdef __linux__
#ifdef NDEBUG
    glfwSetInputMode(id, GLFW_CURSOR, (cursor_visibility ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED));
#endif
#endif
    if (uid_callbacks) {
        keypress();
    }

    glfwMakeContextCurrent(id);

    glfwSwapInterval(1); // VSYNC

    gl3wInit();

    if (!fullscreen) pos( offset_x, offset_y );

    size( width, height );

    glEnable(GL_BLEND);

    std::string frag;
    std::string vert;
    shader = new ShaderProgram(frag,vert);

    // vbo = new VBO();
    // vbo->add();


}

static void framebuffer_size_callback(GLFWwindow* id, int width, int height) { glViewport(0, 0, width, height); }


void Window::pos(uint32_t offset_x, uint32_t offset_y) {

    Output::pos( offset_x, offset_y);

    glfwSetWindowPos(id, offset_x, offset_y);
    glfwSetFramebufferSizeCallback(id, framebuffer_size_callback);
    glfwSetWindowSize(id, width, height);

}

void Window::size(uint32_t width, uint32_t height) {

    Output::size(width, height);

    glfwSetWindowSize(id, width, height);
    glViewport(0, 0, width, height);
}


Window::~Window() { glfwTerminate(); }

void Window::keypress() {
    glfwSetKeyCallback(id, [](GLFWwindow* id, int key, int scancode, int action, int mode) {
        (void)scancode;
        (void)mode;
        auto _this = (Window*)glfwGetWindowUserPointer(id);
        if (action == GLFW_PRESS) {

            if (_this->keypress_cbs.find(key) != _this->keypress_cbs.end()) return _this->keypress_cbs[key](key);

            PLOGD << "key " << (int)key ;

        }
    });
    glfwSetCursorPosCallback(id, [](GLFWwindow* id, double mouse_x, double mouse_y) {
        auto _this = (Window*)glfwGetWindowUserPointer(id);
        _this->mouse_x = mouse_x;
        _this->mouse_y = mouse_y;
    });
    glfwSetMouseButtonCallback(id, [](GLFWwindow* id, int button, int action, int mods) {
        (void)button;
        (void)mods;
        auto _this = (Window*)glfwGetWindowUserPointer(id);
        if (action == GLFW_PRESS) {
            // mouse click
             _this->clickCallBack();
        }
    });
}


void Window::draw() {

    if (texture) {

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        texture->bind();
        shader->use();
        vbo->draw();

    }

}


void Window::render(std::function<void()> callback) {

    glfwPollEvents();

    // fps.run(max_fps);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // BG COLOR
    glClear(GL_COLOR_BUFFER_BIT); //|GL_STENCIL_BUFFER_BIT); ??

    callback();

    if (texture) {

        texture->bind();
        shader->use();
        vbo->draw();
    }

    glfwSwapBuffers(id);
}
