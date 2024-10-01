#include "window.hpp"
#include "framebuffer.hpp"
#include "gui.hpp"

#include "shader.hpp"
#include "texture.hpp"
#include "vbo.hpp"

#include <chrono>
#include <thread>


Window::Window(uint32_t width, uint32_t height, uint32_t offset_x, uint32_t offset_y, bool fullscreen)

    : fullscreen(fullscreen), Output(width, height) /*, fps("Window")*/ {

    int8_t windows_border = 0;
    bool window_on_top = true;
    int8_t cursor_visibility = 0;
    int8_t uid_callbacks = 1;


    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DECORATED, (windows_border ? GLFW_TRUE : GLFW_FALSE));
    glfwWindowHint(GLFW_FLOATING, (window_on_top ? GLFW_TRUE : GLFW_FALSE));
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER,  GLFW_TRUE );

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    int t_offset_x = 0;

    displays.clear();

    for (int i = 0 ; i < count; i++) {

        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

        displays.push_back(Display(mode->width, mode->height,mode->refreshRate, t_offset_x ));

        t_offset_x += mode->width;


    }

    if (!displays.size())

        {PLOGE  << " NO MONITOR"; }
        
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
        // keypress();
    }

    glfwMakeContextCurrent(id);

    glfwSwapInterval(1); // VSYNC

    gl3wInit();

    if (!fullscreen) pos( offset_x, offset_y );

    size( width, height );

    glEnable(GL_BLEND);

    GLint gl_major_version, gl_minor_version;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);

    PLOGI  << " Display @ " << displays.back().rate << "Hz " << displays.back().width << "x" << displays.back().height << " - GL_VERSION "<< gl_major_version << "." << gl_minor_version << "";

}

static void framebuffer_size_callback(GLFWwindow* id, int width, int height) { glViewport(0, 0, width, height); }

bool Window::visibility() {

    return visibility_v;

}

void Window::visibility(bool value) {

    visibility_v = value;
    
    if (value){

        pos(offset_x, offset_y);
        glfwShowWindow(id);

    }else
        glfwHideWindow(id);

    // glfwSetWindowAttrib(id, GLFW_VISIBLE, value?GLFW_TRUE:GLFW_FALSE);

}

void Window::pos(int offset_x, int offset_y) {

    // if (offset_x == this->offset_x && offset_y == this->offset_y)
    //     return;
    
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

void Window::fit() {

    if (!displays.size())
        return;

    size(displays[0].width, displays[0].height);

}

void Window::keypress() {

    glfwSetKeyCallback(id, [](GLFWwindow* id, int key, int scancode, int action, int mode) {
        (void)scancode;
        (void)mode;
        auto _this = (Window*)glfwGetWindowUserPointer(id);
        if (!_this)
            return;
        if (action == GLFW_PRESS) 
            _this->keys_down.insert(key);

        if (action == GLFW_RELEASE) 
            _this->keys_down.erase(key);

        if (_this->keys_down.size()){
            for (auto x : _this->keypress_cbs) 
                if (x.first == _this->keys_down) 
                    x.second();
            // PLOGW << _this->keys_down;
        }
        
    });
    glfwSetCursorPosCallback(id, [](GLFWwindow* id, double mouse_x, double mouse_y) {
        
        void* void_ptr = glfwGetWindowUserPointer(id);

        if (!void_ptr) 
            return;

        auto win = (Window*)void_ptr;

        if (win->mousemove_cb)
            win->mousemove_cb(mouse_x, mouse_y);

    });

    glfwSetMouseButtonCallback(id, [](GLFWwindow* id, int button, int action, int mods) {
        (void)button;
        (void)mods;
        void* void_ptr = glfwGetWindowUserPointer(id);

        if (!void_ptr) 
            return;

        auto win = (Window*)void_ptr;

        if (id == win->id && win->mousedown_cb && action == GLFW_PRESS) 
            win->mousedown_cb(button);

    });
}


void Window::draw() {

    static VBO *vbo;

    static struct Quad : File { Quad() : File("quad.obj", "o quad\n\nv -1 -1 0\nv 1 -1 0\nv -1 1 0\nv 1 1 0\n\nvt 0 1\nvt 1 1\nvt 0 0\nvt 1 0\n\nf 1/1 2/2 3/3 \nf 2/2 3/3 4/4") { } } upside_quad;

    static bool init = false;

    if (!init){

        static std::string frag = "#version 430 core\n\nlayout (binding = 0, std140) uniform ubo { int frame, fps, s0, s1; };\n\nuniform sampler2D tex;\n\nin vec2 UV; out vec4 COLOR;\n\nvoid main() {\n\n\tCOLOR = texture(tex, UV);\n\n\tif (fps<60) COLOR+=vec4(mod(frame,10)*.05,0,0,1);\n}\n\n";
        static std::string vert = "#version 430 core\n\nlayout (binding = 0, std140) uniform ubo { int frame, fps, s0, s1; };\n\nlayout (location = 0) in vec2 POSITION;\nlayout (location = 1) in vec2 TEXCOORD;\n\nout vec2 UV;\n\n\nvoid main() {\n    \n	UV = TEXCOORD;\n    \n	gl_Position = vec4(POSITION.x,POSITION.y,0,1);\n\n}\n\n";
        shader = new ShaderProgram(frag,vert);

        vbo = new VBO();
        vbo->addFile(&upside_quad);

        init = true;

    }

    if (fb) {

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);

        fb->texture.bind(0);
        shader->use();
        vbo->draw();

    }

}


void Window::render(std::function<void()> callback) {

    PLOGV << "========================================";

    while (!glfwWindowShouldClose(id)) {

        // PLOGV << "begin frame";

        fps.run();

        glfwPollEvents();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // BG COLOR
        glClear(GL_COLOR_BUFFER_BIT); //|GL_STENCIL_BUFFER_BIT); ??

        callback();

        glfwSwapBuffers(id);

        for (auto cb : end_of_render_cbs)
            cb.second(cb.first);

        end_of_render_cbs.clear();
    
        // PLOGV << "end frame";

    }

}