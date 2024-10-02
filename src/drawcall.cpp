#include "drawcall.hpp"
#include "effector.hpp"
#include "engine.hpp"
#include "utils.hpp"
#include "instance.hpp"
#include "layer.hpp"






Model* DrawCall::addModel(File* f) {

     auto x = Modelable::addModel(f);

    //  vbo.addFile(f, models.size()-1);

     return x;
}

///////// LayerBuilder ////


DrawCall::Builder::Builder(DrawCall* dc) : dc(dc) {

    shader = &dc->shader;
    vbo = &dc->vbo;

}

std::string DrawCall::Builder::print_layer(Effectable &effectable, std::string prepend,std::string instance, std::string ar) {

    auto name = lower(effectable._name());

    if (effectable.quantity() > 1) name += "["+(dc->models.size() == 1?"ID":instance)+"]";

    std::string body_fragment;

    body_fragment += "\t// "+name+"\n";

    // body_fragment += "\taspect_ratio = static_ubo."+ar+".dim;\n";
                
	body_fragment += "\tnext(vec4(1));\n";

    for (auto ref : effectable.effector_refs) 

        body_fragment+=ref->effector->body(this, "dynamic_ubo[curr]."+prepend+"."+name+"."+ref->effector->_name());


	body_fragment += "\tnext(vec4(0));\n";

    return body_fragment;

}


static void addFile(DrawCall*dc, File* file, int id, int w , int h, int x, int y) {

    auto model_inst = Instance(*dc->statinst);
    model_inst[&dc->models_stat];

    dc->vbo.addFile(file, id, w, h, x, y);
    dc->models_stat.quantity(id+1);
    model_inst.eq(id).set<float,4>(w, h, x, y);


}

void DrawCall::Builder::setup() {

    // detect if VBO list has changed 
    bool changed = false;
    for (auto model : dc->models) {

        if (dc->last_modified[model->file] != model->file->last_modified) {
            changed = true;
            break;
        }

    }

    // if so re upload
    if (changed) {

        dc->last_modified.clear();
        vbo->reset();

        int i = 0;
        int model_id = 1;
        for (auto model : dc->models) {

            // here wouild split in case of grid or dimensions |(aka cloning ?)
            // instead of following forloop

            if (model->cloner == Model::Cloner::CUBE) {

                int w = 1/model->quantity_v[0];
                int h = 1/model->quantity_v[1];
                // int d = 1/model->quantity_v[2];
                for (int x = 0; x < model->quantity_v[0]; x++) 
                    for (int y = 0; y < model->quantity_v[1]; y++) 
                        // for (int z = 0; z < model->quantity_v[2]; y++) 
                            addFile(dc, model->file, i++, w, h, x*w, y*h);

            }else

                for (auto e = 0; e < model->quantity(); e++) {

                    addFile(dc, model->file, i++,1,1,0,0);

                }

            model_id++;

        }

    }

    // shader

    ubos.insert(engine.dynamic_ubo);
    ubos.insert(engine.static_ubo);

    // FRAGMENT ////////////////

    header_fragment += "\n";

    header_fragment += "in flat int ID;\n"; // COULD add condition
    header_fragment += "\tint obj  = int(OBJ);\n\n"; // COULD add condition

    header_fragment += "vec2 uv = UV;\n";
    header_fragment += "vec4 color = vec4(0);\n";
    // header_fragment += "vec2 base_uv = uv;\n";
    // header_fragment += "vec4 base_color = vec4(1);\n";
    header_fragment += "vec2 aspect_ratio = vec2(1);\n\n";

    header_fragment += "void next(vec4 base) { COLOR += color; uv = UV; color = base; }\n\n";

    header_fragment += "int curr = 0; //dynamic_ubo[0].eNGINE.alt;\n";
    header_fragment += "int last = abs(curr-1);\n\n";
    
    body_fragment += "\tCOLOR = vec4(0);\n\n";


    // setup models effectors
    for (auto &model : dc->models) 
        for (auto x : model->effector_refs) 
            x.get()->effector->setup(this);
    
    // setup layer effectors
    for (auto x : dc->effector_refs) 
        x.get()->effector->setup(this);
    

    // do models effectors
   std::string ar_str = "layers";//+std::string(Layer::statinst->m()->quantity()>1?"[int(LAYER)]":""); // statint missusded, should be 
    if (dc->models.size() == 1) 
        body_fragment += print_layer( *dc->models[0].get(), lower(dc->name()), "obj", ar_str );

    else {

        int last_id = 0;

        for (auto &x : dc->models) {

            if (last_id) body_fragment += "\n} else ";

            auto l = last_id;
            last_id += x.get()->quantity();

            body_fragment += "if (obj < "+std::to_string(last_id)+" ){ "+(l?"obj -= "+std::to_string(l)+";":"")+"\n\n" + print_layer( *x.get(), lower(dc->name()), "obj", ar_str );

        }

        if (dc->models.size()) body_fragment += "\n}\n";

    }

    
    // do layer effectors
    for (auto ref : dc->effector_refs) 
        body_fragment += ref.get()->effector->body(this, "dynamic_ubo[curr]."+lower(dc->_name())+"."+lower(ref->ref()->_name()));
    
    if (dc->effector_refs.size()) 
            body_fragment += "\tnext(vec4(0));\n\n";

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

DrawCall::DrawCall(std::string name) : 
    Modelable(engine.dynamic_ubo->next_name(name.length()?name:"layer")),
    models_stat(""),
    stat(name)
{

    models_stat.quantity(0);
    models_stat.add(&globals.model);
    models_stat.force_ref = true;
    stat.add(&models_stat);

    engine.static_ubo->add(&stat);
    engine.dynamic_ubo->add(this);

    statinst =  &(*new Instance(*engine.static_ubo))[&stat];
    statinst->track();

    dyninst = &(new Instance(*engine.dynamic_ubo))->loc(this);
    dyninst->track();

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

    glBlendFunc(GL_BLEND_MODES[GL_BLEND_MODE_IN], GL_BLEND_MODES[(models.size() != 1 ? GL_BLEND_MODE_OUT : GL_ONE)]);
            
    shader.use();

    vbo.draw((models.size() == 1 ? models[0]->quantity():1));

}

::Builder* DrawCall::builder() {

    if (!builder_v)
        builder_v = new Builder(this);
    
    return builder_v;
    
}