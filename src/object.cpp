#include "object.hpp"

#include <memory>
#include <cstring>

#include "struct.hpp"
#include "buffer.hpp"
#include "entry.hpp"
#include "log.hpp"

size_t Object::size() { return eq(reserved); }
size_t Object::stride() { return s->stride()*is_stride; }

char *Object::data(size_t id) { return &buffer->data[offset + eq(id)]; }

int Object::eq(size_t id) { return ((s->size()+stride()) * id); }

Entry &Object::push() { 

    std::vector<char> data;
    data.resize(s->size());
    memset(&data[0],0,data.size());

    return push(&data[0]);

}

void Object::clear() {

    

}




void Object::addComp(std::string component){

    auto bkp = buffer->bkp();

    PLOGD << "NEEDBKPHERE";

    s->addComp(component);

    buffer->transpose(bkp);

    // niquons l'opti :) | whats teh pb ? a kwa sasssair ? ca pourrait etre ds update ? 
    
    // 1 test de le mettre avanrt buffer->transpose(bkp_v);

    // 2 test de le mettre dans this->s->addComp(component)

    for (auto e : entrys){

        auto c = (*e)[component.c_str()];

        int i = 0;
        for (auto member : Component::id(component.c_str())->members) {

            auto w = &c[i++].get<float>();        
            (*w) = member.default_val;
            if (member.type == Member::Type::VEC2) { *(w+1) = member.default_val; }
            if (member.type == Member::Type::VEC3) { *(w+1) = member.default_val; *(w+2) = member.default_val; }
            if (member.type == Member::Type::VEC4) { *(w+1) = member.default_val; *(w+2) = member.default_val; *(w+3) = member.default_val; }
            
        }

    }

    // -  fin

    buffer->update();

}
void Object::removeComp(std::string component){

    auto bkp = buffer->bkp();

    PLOGD << "NEEDBKPHERE";

    this->s->removeComp(component);

    buffer->transpose(bkp);

    buffer->update();

}


Entry &Object::push(void* data) { 

    Buffer bkp = *buffer;

    PLOGD << "NEEDBKPHERE";

    reserved+=1;

    buffer->transpose(bkp);

    int id = entrys.size();

    memcpy(this->data()+eq(id),data,s->size()); 

    entrys.push_back(new Entry{this,id});

    // shouldnt buffer->update() here ? if not find where does it happn now and if is logic ?
    
    return *entrys.back();

}   