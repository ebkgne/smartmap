

#pragma once

// static unsigned int width = 400;
// static unsigned int height = 200;
// static unsigned int pos_x = 0;
// static unsigned int pos_y = 0;

#include "log.hpp"

#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>
#include <cmath>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct BoilerArtnet {

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    int universe = 0;

    BoilerArtnet() {

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
        servaddr.sin_port = htons(6454);

        if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            return;
        }

        std::cout << "UDP listener running on 2.0.0.111:6454" << std::endl;
    }

    ~BoilerArtnet() {
        close(sockfd);
    }

    char* receive() {

        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, 
                        (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        if (buffer[9] == 80 && buffer[13] == universe)
            return &buffer[18];

        return nullptr;

    }

};



#include <GL/gl3w.h>
#include <GLFW/glfw3.h>


struct BoilerWindow {

    GLuint width , height ,pos_x , pos_y ;

    double lastTime = glfwGetTime();

    GLFWwindow* window;

    float clear_color[4] = {0,0,0.1,1};

    BoilerWindow(GLuint width = 400, GLuint height = 300, GLuint pos_x = 400, GLuint pos_y = 300) : 
        width(width), height(height), pos_x(pos_x), pos_y(pos_y) {

        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "OUTPUT", nullptr, nullptr);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

        gl3wInit();

    }

    void run(std::function<void()> cb) {

            while (!glfwWindowShouldClose(window)) {

                lastTime = glfwGetTime();


                glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]); // BG COLOR
                glClear(GL_COLOR_BUFFER_BIT); //|GL_STENCIL_BUFFER_BIT); ??

                cb();

                glfwPollEvents();

                glfwSwapBuffers(window);

            }
    }

};


struct BoilerQuad {

    GLuint vao = 0,vbo = 0,ibo = 0;

    std::vector<std::array<float, 4>> vertices;

    static inline std::vector<std::array<int, 3>> indices ={ {0,1,2}, {1,2,3} };

    ~BoilerQuad() {

        glBindVertexArray(vao);
        glDisableVertexAttribArray(0);
        glDeleteVertexArrays(1, &vao);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDisableVertexAttribArray(1);

        vbo = 0;
        ibo = 0;
        vao = 0;

    }

    BoilerQuad(float r) {

        vertices =  { {-r, -r, 0, 1}, {r, -r, 1, 1}, {-r, r, 0, 0}, {r, r, 1, 0} };

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(std::array<float, 4>), vertices.data(), GL_STATIC_DRAW);

        if (!vao) {

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

        }

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::array<int, 3>), indices.data(), GL_STATIC_DRAW);

    }

    void draw() {

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glDrawElementsInstanced(GL_TRIANGLES, indices.size()*3, GL_UNSIGNED_INT, 0, 1);

    }

};


#include "image.hpp"

struct BoilerTexture {

    Image image;
    GLuint texture;

    BoilerTexture(std::string file = "assets/medias/boy.jpg") : image(file) {

        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, image.width, image.height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glActiveTexture(GL_TEXTURE0);


        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,image.width, image.height,GL_RGB,GL_UNSIGNED_BYTE,&image.data[0]);
        glGenerateMipmap(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);


    }
};

struct BoilerShader {

    GLuint id = 0, frag_id = 0, vert_id = 0;

    std::string frag,vert;
    
    BoilerShader(

        std::string frag = "#version 430 core \n\nout vec4 COLOR;\nin vec2 UV;\nvoid main() {\n COLOR = vec4(UV.x);\n}",
        std::string vert = "#version 430 core \n\nlayout (location = 0) in vec2 POSITION;\nlayout (location = 1) in vec2 TEXCOORD;\nout vec2 UV;\nvoid main() { UV = TEXCOORD; gl_Position = vec4(POSITION.x,POSITION.y,0,1); }"

    ) : frag(frag), vert(vert) { create(); }

    void destroy() {

        if (id) glDeleteProgram(id);

        if (frag_id) glDeleteShader(frag_id);

        if (vert_id) glDeleteShader(vert_id);

    }

    void create(std::string new_frag="", std::string new_vert="") {

        if (new_frag.length() && new_vert.length()){
            frag =new_frag;
            vert =new_vert;
        }

        destroy();

        id = glCreateProgram();

        auto fragptr = (const GLchar* const ) frag.c_str();

        frag_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_id, 1, &fragptr, nullptr);
        glCompileShader(frag_id);
        // Check for errors

        GLchar infoLog[512];
        GLint success;

        glGetShaderiv(frag_id, GL_COMPILE_STATUS, &success);

        if (!success) {

            glGetShaderInfoLog(frag_id, 512, NULL, infoLog);

            PLOGW << infoLog;

        }

        auto vertptr = (const GLchar* const ) vert.c_str();

        vert_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert_id, 1, &vertptr, nullptr);
        glCompileShader(vert_id);

        glGetShaderiv(vert_id, GL_COMPILE_STATUS, &success);

        if (!success) {

            glGetShaderInfoLog(vert_id, 512, NULL, infoLog);

            PLOGW << infoLog;

        }


        glAttachShader(id, frag_id);
        glAttachShader(id, vert_id);

        glLinkProgram( id );

        glUseProgram(id);

    }


    int loc(std::string name) { return glGetUniformLocation(id, name.c_str()); }

    void set(std::string name, int i1) { glProgramUniform1i(id, loc(name), i1); }

    void use() { glUseProgram(id); }

    void texture(std::string name = "tex", int loc = 0){ auto x = glGetUniformLocation(id, name.c_str()) ; glUniform1i(x, loc); }

};


struct BoilerUBO {

    std::vector<float> data = {1,2,10,100};

    int binding = 0;
    std::string name = "ubo";

    GLuint id;

    BoilerUBO() {

        glGenBuffers(1, &id);

        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, data.size()*4, NULL, GL_DYNAMIC_COPY);
    }

    void upload(){ 

        // std::string str;
        // for (int i = 0 ; i < data.size()*sizeof(int); i++) str+= std::to_string(*(((uint8_t*)data.data())+i)) + " ";
        // PLOGW << name << " " << id << " " << binding << ": " << data.size()*4 << " - " << str;

        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, data.size()*4, &data[0]);

    }

    void bind(GLuint shader, int binding = 0){ 

        this->binding = binding;
       
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glUniformBlockBinding(shader, glGetUniformBlockIndex(shader, name.c_str()), binding);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, id);

        upload();
    }

};



#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

struct BoilerGUI {

    GLFWwindow* window = nullptr;

    BoilerGUI(GLFWwindow* window) {

        create(window);
    }
    
    BoilerGUI() {

    }

    void newframe() {

        if (!window)
            return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    }

    void render() {

        if (!window)
            return;
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

    }
    void destroy() {


        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext();


    }
    void create(GLFWwindow* window) {

        this->window = window;

        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.WantCaptureKeyboard = false;

        ImGui_ImplGlfw_InitForOpenGL(window, false);
        ImGui_ImplGlfw_InstallCallbacks(window);
        ImGui_ImplGlfw_SetCallbacksChainForAllWindows(true);

        const char* glsl_version = "#version 430";
        ImGui_ImplOpenGL3_Init(glsl_version);

    }

    bool draw(float* ptr, int q) {

        bool has_changed = false;

        newframe();
        ImGui::Begin("GUI");

        for (int i = 0; i < q; i++) 
            if (ImGui::SliderFloat(("f"+std::to_string(i)).c_str(), ptr+i,0,1))
                has_changed = true;
            
        ImGui::End();
        render();

        return has_changed;

    }
};
// #include "editor.hpp"
// #include "engine.hpp"

#include "builder.hpp"

struct Boilerplate {

    static void Init(bool run = true) {

        logger.cout(Sev::warning);

        BoilerWindow window;
        BoilerGUI gui(window.window);

        // BoilerArtnet artnet;

        Builder builder;
        builder.build();

        builder.header_fragment += "layout (binding = 0, std140) uniform uboss { vec4 xx; };\n";
        builder.header_vertex += "layout (binding = 0, std140) uniform uboss { vec4 xx; };\n";

        builder.body_fragment += "\tCOLOR = xx;\n";
        
        // PLOGW << builder.frag();

        BoilerShader shader(builder.frag(),builder.vert());
        
        BoilerUBO ubo;
        ubo.bind(shader.id);
        
        BoilerQuad quad;

        window.run([&](){
            
            shader.use();
            quad.draw();

            if (gui.draw(&ubo.data[0], 4))
                ubo.upload();

        });

    }
};
