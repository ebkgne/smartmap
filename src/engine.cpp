#include "engine.hpp"

Engine::Engine(uint16_t width, uint16_t height) 

    : window(width,height), 
    dynamic_ubo("dynamic_ubo"), static_ubo("static_ubo"), 
     gui(window.id) {

    fb = new FrameBuffer(0,width,height,"mainFB");
    
    // framebuffersUBO = static_ubo.buffer.add("framebuffersUBO", {"Resolution"}, 100 );

    quad = new VBO("quad.obj", 0, "quad");

    stack.list.push_back(new Stack::Action{[](){ 

        for (int i = 0; i< File::pool.size(); i++) { 

            File* file = File::pool[i];

            if (file->survey && file->last_modified) {

                auto new_modified = file->getTimeModified();

                if (file->last_modified != new_modified) file->callback(file);      

            }

        }

     }, "Files survey"});


}