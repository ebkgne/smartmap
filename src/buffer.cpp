#include "buffer.hpp"
#include "widgets/buffer.hpp"
        
        int Buffer::Object::create() {
            
            quantity+= 1;
            buffer->updateBuffer();
            return components.size()-1;
        }

        void Buffer::Object::add(std::string component) { 

            components.push_back(Component::id(component.c_str())); 

            size += Component::id(component.c_str())->size;

        }

        void *Buffer::Object::data() { return (void*)&buffer->data[offset]; }

        void Buffer::Object::set(const char* name, void* data, int id) { 

            Component *comp = nullptr;

            int offset = this->offset;

            for (auto &c:components) { 
                
                if (!strcmp(c->name.c_str(),name)) {

                    comp = c;
                    break;

                }

                offset += c->size;
            
            }

            if (comp) { memcpy(&buffer->data[size*id+offset], data, comp->size); }
            else{ std::cout << name << " does not recall" << std::endl; }

        }

        void Buffer::Object::set(int member_id, void* data, int obj_id) { 

            Component *comp = components[member_id];

            int offset = this->offset;
            for (size_t i = 0; i < member_id; i++) offset += components[i]->size;
           
            if (comp) memcpy(&buffer->data[size*obj_id+offset], data, comp->size);

        }



    Buffer::Object *Buffer::add(std::string name, std::vector<std::string> components, int quantity) { 

        objects.push_back({name, {}, quantity}); 

        for (auto comp : components) objects.back().add(comp);
        
        updateBuffer(); 

        objects.back().buffer = this;

        return &objects.back();

    }

    void Buffer::reset() { objects.resize(0); data.resize(0); }

    void Buffer::updateBuffer() {

        int size = 0;
        for (auto &obj:objects) { for (auto comp:obj.components) { size += comp->size*obj.quantity; } }

        data.resize(size);
        memset(&data[0],0,data.size());

        int offset = 0;
        for (auto &obj:objects) {
            
            obj.offset = offset;

            offset += obj.size*obj.quantity;
            
        }

        callback();

    }

    Buffer::Buffer(std::string name) : name(name) { pool.push_back(this);  objects.reserve(10); BufferWidget::updateBufferList(); }