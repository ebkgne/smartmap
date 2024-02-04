#include "engine.hpp"
#include "directory.hpp"
#include "struct.hpp"
#include "model.hpp"
#include "shaderfx.hpp"
#include "drawcall.hpp"

#include "nodes.hpp"

Engine::Engine(uint16_t width, uint16_t height) 
    : window(1920,1080), 
    // : window(width,height), 
    // dynamic_ubo("dynamic_ubo"),//  static_ubo("static_ubo"), 
     gui(window.id) {
    window.max_fps = 59;

    PLOGD << "Engine created";

}

Engine::~Engine() { PLOGD << "Engine destroyed"; }

void Engine::run() {

    auto &window = getInstance().window;

    while (!glfwWindowShouldClose(window.id)) window.render([](){
        
        auto &engine = Engine::getInstance();
        
        engine.dynamic_ubo.upload();

        // engine.stack->run();

        engine.gui.draw(); 

    });

};

void Engine::init() {

    Nodes::init();

    Node *models = tree->add<Directory>("assets/model/");
    Node *shaders = tree->add<Directory>("assets/shaders/");

    auto test = dynamic_ubo.buffer.addObj(new Struct{"TEST",{"float"}});

    test->push();


    // remove Model keep just file and go to VBO

    // auto dyn = new UBO("dynamic_ubo");
    // tree->add(dyn);
    // dynamic_ubo = &dyn->buffer;

    // dynamic_ubo->add(new Struct("TOUT",{"float"},1));

    // Components::getInstance();
    // Node* comps = tree->add(new Node{"Components"});
    // for (auto c : Component::pool) comps->add(c);


    
    // Node* controllers = tree->add(new Node{"Controllers"});
    // auto an = controllers->add(new Node{"Art-Net"});
    // an->add(new Node{"1"});
    // an->add(new Node{"2"});
    // an->add(new Node{"3"});
    // an->add(new Node{"4"});

    // auto dc = new DrawCall();

    // tree->add(dc);
    
    // dc->vbo.add()

    
    // auto q1 = dc->add(models->childrens[0]);
    // dc->childrens[0]->add(shaders->childrens[0]);



    // atlas = (Atlas*)tree->add(new Atlas(4096, 4096, "assets/media/"));
    
    // stack.list.push_back(new Stack::Action{[](){ 

    //     for (int i = 0; i< File::pool.size(); i++) { 

    //         File* file = File::pool[i];

    //         if (file->survey && file->last_modified) {

    //             auto new_modified = file->getTimeModified();

    //             if (file->last_modified != new_modified) file->callback(file);      

    //         }

    //     }

    //  }, "Files survey"});

     PLOGD << "Engine initialized";

}