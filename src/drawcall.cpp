#include "drawcall.hpp"
#include "effector.hpp"
#include "engine.hpp"
#include "utils.hpp"
#include "instance.hpp"
#include "layer.hpp"




Model* DrawCall::addModel(File* f) {

     auto x = Modelable::addModel(f);

     vbo.addFile(f, models.size()-1);

     return x;
}

///////// LayerBuilder ////


std::string DrawCall::Builder::print_layer(Effectable &effectable, std::string prepend,std::string instance, std::string ar) {

    auto name = lower(effectable._name());

    if (effectable.quantity() > 1) name += "["+(dc->models.size() == 1?"ID":instance)+"]";

    std::string body_fragment;

    body_fragment += "\t// "+name+"\n";

    body_fragment += "\taspect_ratio = static_ubo."+ar+".dim;\n";
                
	body_fragment += "\ttic();\n";

    for (auto ref : effectable.effector_refs) 

        body_fragment+=ref->effector->body(this, "dynamic_ubo[curr]."+prepend+"."+name+"."+ref->effector->_name());


	body_fragment += "\ttac();\n";

    return body_fragment;

}

void DrawCall::Builder::use(DrawCall* dc, int stride_count) {

    this->dc = dc;
    this->stride_count = stride_count;

}

void DrawCall::Builder::setup() {

    vbo = &dc->vbo;

    ubos.insert(engine.dynamic_ubo);
    ubos.insert(engine.static_ubo);

    for (auto x : effectors_fragment) x->setup(this);

    for (auto x : effectors_vertex) x->setup(this);

    // FRAGMENT ////////////////

    header_fragment.clear();

    header_fragment += "\n";
    header_fragment += "in flat int ID;\n";

    header_fragment += "vec2 uv = UV;\n";
    header_fragment += "vec4 color = vec4(0);\n";
    header_fragment += "vec2 base_uv = uv;\n";
    header_fragment += "vec4 base_color = vec4(1);\n";
    header_fragment += "vec2 aspect_ratio = vec2(1);\n\n";

    header_fragment += "void tic() { COLOR += color; uv = base_uv; color = base_color; }\n";
    header_fragment += "void tac() { COLOR += color; uv = base_uv; color = vec4(0); }\n\n";
    header_fragment += "void base() { base_uv = uv; base_color = color; color = vec4(0); }\n\n";
    header_fragment += "int curr = dynamic_ubo[0].eNGINE.alt;\n";
    header_fragment += "int last = abs(curr-1);\n\n";

    effectors_fragment.clear();
    body_fragment.clear();
    body_fragment += "\tCOLOR = vec4(0);\n\n";

    if (dc) {

        // do all body 

        for (auto &model : dc->models)
            for (int instance = 0; instance < (dc->models.size() == 1?1:model.get()->quantity()); instance++) 
                body_fragment += print_layer(*model, lower(dc->_name()), std::to_string(instance), "layer"+std::string(Layer::glsl_layers->m()->quantity()>1?"[int(LAYER)]":""));


        for (auto ref : dc->effector_refs) 
            body_fragment += ref.get()->effector->body(this, "dynamic_ubo[curr]."+lower(dc->_name())+"."+lower(ref->ref()->_name()));
        
        // setup all effector_refs

        for (auto &model : dc->models) 
            for (auto x : model->effector_refs) 
                x.get()->effector->setup(this);
        
        for (auto x : dc->effector_refs) 
            x.get()->effector->setup(this);


        if (dc->effector_refs.size()) 
            body_fragment += "\ttac();\n\n";

    }

    // VERTEX ////////////////

    header_vertex.clear();

    for (int i = 1; i < globals.vertice.members.size(); i++) {

        auto m = globals.vertice.members[i];

        header_vertex += "out "+std::string(m->type().id == typeid(int)?"flat ":"")+m->type_name()+" "+m->_name()+";\n";

    }

    header_vertex += "out flat int ID;\n\n";

    body_vertex.clear();

    for (int i = 1; i < globals.vertice.members.size(); i++) {

        auto m = globals.vertice.members[i];
        body_vertex += "\t"+m->_name()+" = "+m->_name()+"_;\n\n";

    }

    body_vertex += "\tID = gl_InstanceID;\n\n";

	body_vertex += "\tvec2 POS = POSITION;\n\n";
	body_vertex += "\tgl_Position = vec4(POS, 0, 1);\n\n";


}




// DRAWCALLL //////////////////////////////////////

DrawCall::DrawCall(std::string name) : Modelable(engine.dynamic_ubo->next_name(name.length()?name:"layer")) {

    engine.dynamic_ubo->add(this);

}


void DrawCall::draw() {
    
    static std::vector<GLenum> GL_BLEND_MODES = {
        
        GL_ZERO,
        GL_ONE,
        GL_SRC_COLOR,
        GL_ONE_MINUS_SRC_COLOR,	
        GL_DST_COLOR,
        GL_ONE_MINUS_DST_COLOR,
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
        GL_DST_ALPHA,
        GL_ONE_MINUS_DST_ALPHA,
        GL_CONSTANT_COLOR,
        GL_ONE_MINUS_CONSTANT_COLOR,
        GL_CONSTANT_ALPHA,
        GL_ONE_MINUS_CONSTANT_ALPHA,
        GL_SRC_ALPHA_SATURATE,
        GL_SRC1_COLOR,
        GL_ONE_MINUS_SRC1_COLOR,
        GL_SRC1_ALPHA,
        GL_ONE_MINUS_SRC1_ALPHA

    };

    glBlendFunc(GL_BLEND_MODES[GL_BLEND_MODE_IN], GL_BLEND_MODES[GL_BLEND_MODE_OUT]);
            
    shader.use();

    vbo.draw((models.size() == 1 ? models[0]->quantity():1));

}

