
#include "smartmap.hpp"
#include "stack.hpp"

#include "imgui/imgui.h"
#include "smartmap/layer.hpp"

#include "include/vendor/rapidjson/document.h"
#include "include/vendor/rapidjson/stringbuffer.h"

namespace SmartMap {

Base::Base() {

    // Engine::getInstance().stack.childrens.push_back(new Stack{&Engine::getInstance().stack});
    // stack = Engine::getInstance().stack.childrens.back();
    // stack->childrens.push_back(new Stack{stack});


#ifdef ROCH
    artnet = new Artnet{"10.0.0.123"};
#elif defined(PLUGREGIE)
    artnet = new Artnet{"10.0.0.49"};
#else
    artnet = new Artnet{"10.0.0.102"};
#endif

    shader = new ShaderProgram({"smartmap.frag", "smartmap.vert"});
    
    layershader = new ShaderProgram({"layer.frag", "basic.vert"});

    shader->sendUniform("freetype", 2);
            
    auto &window = Engine::getInstance().window;
    window.setSize(1920,1080);
    
#ifdef ROCH
    window.setPos(2560,0);
    // window.setPos(1920-400,0);
    // window.setSize(400,300);
#elif defined(PLUGREGIE)
    window.setPos(1600,200);
    window.setSize(640,480);
#endif
              
    shader->use();
    atlas = new Atlas(4096,2048, "assets/media/");
    atlas->link(shader);
    
    // smartlayersUBO = Engine::getInstance().static_ubo.buffer.add("Layer", {"int", "ID", "Offset", "ID"}, 10 );

    // fix1UBO = Engine::getInstance().dynamic_ubo.buffer.add("Fixture", {

    //     "Opacity",
    //     "RGB",
    //     "Position",
    //     "Size",
    //     "Gobo",
    //     "Orientation",
    //     "Feedback",
    //     "Strobe",
    //     "float", // for alignmentr
        
    // }, 50 );

    // fix2UBO = Engine::getInstance().dynamic_ubo.buffer.add("Fixture2", {

    //     "Opacity",
    //     "RGB",
    //     "Position",
    //     "Size",
    //     "Gobo",
    //     "Orientation",
    //     "Feedback",
    //     "Strobe",
    //     "float", // for alignmentr
        
    // }, 50 );
    
    // blur_x = new ShaderProgram({"blur_x.comp"});
    // blur_y = new ShaderProgram({"blur_y.comp"});
    // basic = new ShaderProgram({"test.frag", "basic.vert"});
    // outBlur = new Texture(nullptr, FW*.5,FH*.5); 
    // outBlur->format = GL_RGBA8;

     
    // Engine::getInstance().stack.list.push_back(new Stack::Action{[this](){

    //     memcpy(fix2UBO->data(),fix1UBO->data(),fix1UBO->byte_size*fix1UBO->quantity);
    //     artnet->run();

    // }, "Artnet"});
            
    // Engine::getInstance().stack.list.push_back(new Stack::Action{[this](){
  
    //     for (auto layer:SmartMap::Layer::pool) { 
            
    //         shader->use();
            
    //         layer->fb->clear(); // thus bind
            
    //         glBlendFunc(GL_BLEND_MODES[GL_BLEND_MODE_IN2], GL_BLEND_MODES[GL_BLEND_MODE_OUT2]);
    //         layer->pass->bind();
    //         atlas->texture->bind();
    //         layer->FTbuffer->bind();
            
    //         layer->quad->draw(layer->quantity); 
            
    //         // if (shader->loaded) layer->pass->read(layer->fb->texture);
            
    //     }

    // }, "SM Layers"});
    // Engine::getInstance().stack.list.push_back(new Stack::Action{[this](){
  

    

    // }, "Swap"});

    // Engine::getInstance().stack.list.push_back(new Stack::Action{[this](){

    //     for (auto &output:SmartMap::Output::pool) output->fb.clear() ;
            
    //     layershader->use();

    //     for (auto &layer:SmartMap::Layer::pool) { 

    //         Output::pool[layer->output]->fb.bind();

    //         layer->fb->texture->bind();
            
    //         layer->quad->draw();
            
    //     }

    //     //  fix2UBO->update();

    // }, "SM Outputs"});

    // Engine::getInstance().stack.list.push_back(new Stack::Action{[this](){
        
    //     glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //     glViewport(0,0,Engine::getInstance().window.width,Engine::getInstance().window.height);

    //     Engine::getInstance().basicshader->use();
    //     for (auto layer:SmartMap::Layer::pool) { 

    //         layer->fb->texture->bind();
    //         Engine::getInstance().quad->draw();

    //     }

    // }, "SM C"});

            
} 

}; 
