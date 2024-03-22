#include "ubo.hpp"

#include "shader.hpp"
#include "struct.hpp"
#include "log.hpp"
#include <GL/gl3w.h>

UBO::~UBO() { destroy(); }

UBO::UBO(std::string name) : Buffer(name) { 


    binding = binding_count++;
    if (binding > 100) PLOGW << "MAX_UBO might soon be reached";// can do better ^^
     
    //  for (auto shader:subscribers) link(shader);

} 

Struct& UBO::add(Struct& s) {

        Buffer::add(s);

        update();
        upload();

        return *this;
        
}

void UBO::destroy() { if (id<0) {glDeleteBuffers(1, &id); id = -1;} } 

void UBO::create() {

    if (!data.size()) return;

    glGenBuffers(1, &id);

    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, footprint_all(), NULL, GL_DYNAMIC_COPY);

}

void UBO::update() { Buffer::update(); upload();  }

void UBO::reset() {

    destroy();

    create();

    upload();

}

void UBO::upload(){ upload(data.data(), data.size()); }

void UBO::upload(void* data, size_t size, uint32_t offset){

    // std::string str;
    // for (int i = 0 ; i < this->data.size(); i++) str+= std::to_string(*(((uint8_t*)data)+i)) + " ";
    // PLOGV << name() << " " << binding << ": " << size << " - " << str;
    
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data); 
    
}