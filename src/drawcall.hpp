#pragma once

#include "vbo.hpp"
#include "shader.hpp"
#include "model.hpp"
#include "builder.hpp"
#include <cstdint>
;

struct DrawCall : Modelable {

    DrawCall(std::string name = "DrawCall");

    virtual void draw();

    VBO vbo;

    Model* addModel(File* f) override;

    virtual ::Builder* builder() ;
    
    ::Builder* builder_v = nullptr ;

    ShaderProgram shader;

    int GL_BLEND_MODE_IN = 6;
    int GL_BLEND_MODE_OUT = 1;//13

    struct Builder : ::Builder {

        void setup() override;

        DrawCall* dc;
        
        Builder(DrawCall* dc);

        int stride_count;

        std::string print_layer(Effectable &effectable,std::string prepend ,std::string instance, std::string ar = "");

    };

    std::map<File*, int64_t> last_modified;
    
};
