#include "engine.hpp"
#include "drawcall.hpp"
#include "file.hpp"
#include "imgui.h"
#include "layer.hpp"
#include "output.hpp"
#include "struct.hpp"
#include "effector.hpp"
#include "gui.hpp"
#include "editor.hpp"
#include "gui.hpp"
#include "ubo.hpp"
#include "node.hpp"
#include "utils.hpp"
#include "vbo.hpp"
#include "instance.hpp"
#include "open.hpp"
#include "save.hpp"

#include "callbacks.hpp"
#include "editors.hpp"
#include "tree.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>
#include <cmath>


Engine::Engine(uint16_t width, uint16_t height) : window(1,1,0,0), glsl_data("ENGINE") {

    glsl_data.add<int>("frame").add<int>("fps").add<int>("alt");

    PLOGD << "                              __            "  ;
    PLOGD << "  ______ _____ _____ ________/  |_  _____ _____  ______  ";
    PLOGD << " /  ___//     \\\\__  \\\\_  __ \\   __\\/     \\\\__  \\ \\____ \\ ";
    PLOGD << " \\___ \\|  Y Y  \\/ __ \\|  | \\/|  | |  Y Y  \\/ __ \\|  |_> >";
    PLOGD << "/____  >__|_|  (____  /__|   |__| |__|_|  (____  /   __/ ";
    PLOGD << "     \\/      \\/     \\/                  \\/     \\/|__|  ";

    glsl_data.striding(true);

    render_passes.reserve(10);

    window.max_fps = 59;

    static auto exit_cb = []() { logger.cout(Sev::warning); exit(0); };

    window.keypress_cbs[{GLFW_KEY_ESCAPE}] = exit_cb;

    static auto save_cb = [&]() { save(); };

    window.keypress_cbs[{GLFW_KEY_LEFT_CONTROL, GLFW_KEY_S}] = save_cb;

    static auto demo_cb = [&]() { engine.gui_v->show_demo = !engine.gui_v->show_demo; };

    window.keypress_cbs[{GLFW_KEY_LEFT_CONTROL, GLFW_KEY_EQUAL}] = demo_cb;

    window.keypress_cbs[{GLFW_KEY_LEFT_CONTROL, GLFW_KEY_ESCAPE}] = [&](){

        engine.tree->each<Window>([](Node*n, Window*w) {

            static uint32_t bkpcoords[2];
            static bool init = false;
            
            if(!init) {
                
                bkpcoords[0] = engine.window.width;
                bkpcoords[1] = engine.window.height;
                init = true;    

            }

            if (engine.window.width>1){
                engine.window.size(1, 1);
                engine.gui(true);
            }else
                engine.window.size(bkpcoords[0], bkpcoords[1]);


        });

    };

    static auto guiact_cb = [&]() { 

        window.end_of_render_cbs.emplace_back(std::pair<void*,std::function<void(void*)>>{nullptr, std::function<void(void*)>([&](void* ptr) { 
            
            // if (engine.gui_v)
            //     exit_cb();
            glfwFocusWindow(engine.window.id);
            engine.gui(!engine.gui_v);  
        
        })});
        
    };

    window.keypress_cbs[{GLFW_KEY_LEFT_CONTROL, GLFW_KEY_M}] = guiact_cb;
    window.mousedown_cb = [](int button) { 
        
        if (button == GLFW_MOUSE_BUTTON_LEFT) guiact_cb(); 
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) exit_cb();
        
    };

    tree = new Node("tree");
    tree->allow<Node>();
    tree->allow<File>();

    window.keypress();
    window.visibility(false);
    // gui(true);

    // gui(false);
    // gui(true);

    // gui(false);
    NODE<Node>::onadd<AnyNode>(single_type_cb);

    tree->active(1);

}

Engine::~Engine() {

    for (auto x : Struct::owned) {
       delete x;
    }

    if (dynamic_ubo) delete dynamic_ubo;
    if (static_ubo) delete static_ubo;

    PLOGI << "Engine destroyed";

}

void Engine::init() {

    Callbacks::init();
 
    editors.init();
    
    dynamic_ubo = new UBO("dynamic_ubo");
    static_ubo = new UBO("static_ubo");
    
    dynamic_ubo->quantity(2);
    dynamic_ubo->add(&glsl_data);

    
    auto main = tree->addOwnr<Node>("main");
    main->onadd<File>([](Node* _this, Node* n){

        auto x = _this->addOwnr<Layer>();
        if (x)
            x->add(n);

        return Node::no_worry;

    });

    main->on(Node::EDITOR, [](Node* n) {

        if (ImGui::BeginTabBar("##mamain")) {
            if (ImGui::BeginTabItem("log")) {
        
                NODE<Log>::on_cb[Node::EDITOR](n, &logger);
                
                ImGui::EndTabItem(); 
            }
            if (ImGui::BeginTabItem("dynubo")) {
        
                NODE<UBO>::on_cb[Node::EDITOR](n, engine.dynamic_ubo);
                
                ImGui::EndTabItem(); 
            }
            if (ImGui::BeginTabItem("statubo")) {
        
                NODE<UBO>::on_cb[Node::EDITOR](n, engine.static_ubo);
                
                ImGui::EndTabItem(); 
            }

            ImGui::EndTabBar();
        
        }
    }); 

    tree->addPtr<File>(&VBO::quad);

    PLOGI << "Engine initialized";
    PLOGV << "===========================";

}
void Engine::gui(bool active) {

    static std::vector<std::shared_ptr<EditorWidget>>  bkp;

    if (gui_v){

        if (!active) {
            
            auto gui_v_ = gui_v;
            gui_v = nullptr;
            delete gui_v_;

        }

    }else if (active) {

        gui_v = new GUI(&window);
        editors.gui = gui_v;
        gui_v->trees[0]->selected= engine.tree;

    }

}

void Engine::run(std::function<void()> cb) {

    bool found = false;
    tree->each<Layer>([&](Node* n, Layer* layer) {

       if (!n->hidden) 
            found = true;

    });

    if (!found)
        engine.tree->find("quad")->hidden = false;

    if (!gui_v && !window.visibility()){

        window.visibility(true);
        window.size(50,50);

    }

    running = true;
    
    window.render([&](){

        if (gui_v)
        
            gui_v->draw();

        tree->run();

        cb();
        
        if (dynamic_ubo) {   
            
            static int frame = 0;
            memcpy(dynamic_ubo->data(), &(frame), 4); // aka dynamic_ubo["ENGINE"]["frame"]
            int fps = std::ceil(window.fps.fps);
            memcpy(dynamic_ubo->data()+4, &fps, 4); // aka dynamic_ubo["ENGINE"]["fps"]
            int alt = frame % 2;
            memcpy(dynamic_ubo->data()+8, &alt, 4); // aka dynamic_ubo["ENGINE"]["alt"]

            frame++;
            
            int dynubofp = dynamic_ubo->footprint();

            int from = 0;//alt ? dynubofp : 0 ;

            int offset = 0;

            int to = from+dynubofp+offset;
            
            dynamic_ubo->upload(dynamic_ubo->data()+from, to, offset);
            
        }

    });

}




void Engine::reset() {

    gui(false);
    if (main) for (auto x : main->childrens) delete x;
    if (inputs) for (auto x : inputs->childrens) delete x;
    if (medias) for (auto x : medias->childrens) delete x;

}

void Engine::open(const char* file) {

    project_filepath  = file;

    Open o;
    
    o.json(file);

}

void Engine::save() { save(project_filepath.c_str()); }

void Engine::save(const char* file) {

    if (!strlen(file)) { PLOGE << "No project file"; return;}

    Save save;

    save.json(file);

}
