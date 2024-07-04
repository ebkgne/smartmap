#include "drawcall.hpp"
#include "effector.hpp"
#include "engine.hpp"
#include "utils.hpp"
#include "layer.hpp"


Model* DrawCall::addModel(File* f) {

     auto x = Modelable::addModel(f);

     vbo.add(f, models.size()-1);

     return x;
}

///////// LayerBuilder ////

Layer::ShaderProgramBuilder::ShaderProgramBuilder(DrawCall* dc) : dc(dc) {

    ubos.push_back(&engine.dynamic_ubo);
    ubos.push_back(&engine.static_ubo);

    samplers = {"medias", "render_pass", "uberlayer"};

    build();

}



void Layer::ShaderProgramBuilder::build() {

    ShaderProgram::Builder::build();

    stride_count = 0;

    // FRAGMENT ////////////////

    for (int i = 1; i < dc->vbo.vertice->members.size(); i++) {

        auto m = dc->vbo.vertice->members[i];

        header_fragment += "in "+std::string(m->type() == typeid(int)?"flat ":"")+m->type_name()+" "+m->name()+";\n";

    }

    header_fragment += "\n";
    header_fragment += "in flat int ID;\n";

    header_fragment += "vec2 uv = UV;\n";
    header_fragment += "vec4 color = vec4(0);\n";
    header_fragment += "vec2 aspect_ratio = vec2(1);\n\n";

    int model_id = 0;

    header_fragment += "void tic() { COLOR += color; uv = UV; color = vec4(1); }\n";
    header_fragment += "void tac() { COLOR += color; uv = UV; color = vec4(0); }\n\n";
    header_fragment += "int curr = dynamic_ubo[0].eNGINE.alt;\n";
    header_fragment += "int last = abs(curr-1);\n\n";


    if (dc) {

        std::string indice = "";

        if (dc->models.size() == 1) body_fragment += print_model("ID", *dc->models[0].get()) + "\ttac();\n\n";

        else for (auto &model : dc->models) {

            for (int instance = 0; instance < model.get()->s_->quantity(); instance++) body_fragment += print_model(std::to_string(instance), *model.get());

            model_id++;

            body_fragment += "\ttac();\n\n";

        }


    }

    if (dc) {

        for (auto &effector : dc->refs) {

            std::string arg_str;

            for (auto &arg : effector->s.members) {

                arg_str += "\tdynamic_ubo[cdurr]."+dc->s_->name()+"."+effector->s.name()+"."+arg->name()+", ";

            }

            arg_str.resize(arg_str.size()-2);

            body_fragment += "\t"+effector->s.name()+"("+arg_str+"); // 3\n";



        }

        if (dc->refs.size()) body_fragment += "\ttac();\n\n";

    }

    // VERTEX ////////////////

}

std::string Layer::ShaderProgramBuilder::print_model(std::string xtra, Model& model) {

    std::string body_fragment;

    // auto name = lower(model.s_->name());

    // if (model.s_->quantity() > 1) name += "["+xtra+"]";

    // body_fragment += "\t// "+name+"\n";

    // body_fragment += "\taspect_ratio = static_ubo.layers"+std::string(Layer::glsl_layers->def()->quantity()>1?"[int(LAYER)]":"")+".dim;\n";
    // body_fragment += "\ttic();\n";

    // for (auto &effector : model.refs) {

    //     std::string arg_str;

    //     if (effector->definitions.size() == 1)  {

    //         for (auto &arg : effector->definitions[0]->s.members) {

    //             arg_str += "dynamic_ubo[curr]."+dc->s.name()+"."+name+"."+effector->s.name()+"."+arg->name()+", ";

    //         }

    //         arg_str.resize(arg_str.size()-2);

    //         body_fragment += "\t"+effector->definitions[0]->s.name()+"("+arg_str+"); // 2\n";

    //     }else if (effector->definitions.size())  {

    //         for (auto &arg : effector->s.members) {

    //             arg_str += "dynamic_ubo[curr]."+dc->s.name()+"."+name+"."+effector->s.name()+"."+arg->name()+", ";

    //         }

    //         arg_str.resize(arg_str.size()-2);

    //         body_fragment += "\t"+effector->s.name()+"("+arg_str+"); d\n";


    //     }
    // }

    return body_fragment;
}


// void Layer::ShaderProgramBuilder::frag() {



//     body_fragment.clear();


// }

//



// DRAWCALLL //////////////////////////////////////

DrawCall::DrawCall(std::string name) : Modelable(name.length()?name:"layer"), builder(this) {

    shader.builder(&builder);

    engine.dynamic_ubo.add(s_);

}

void DrawCall::draw() {

    shader.use();

    vbo.draw();

}


void DrawCall::update() {

    static std::filesystem::file_time_type last_modified = std::chrono::file_clock::now();

    static bool has_changed = false;

    for (auto x : models) { // rien a foutre la

        if (x.get()->file->owned) continue;

        auto last_ = std::filesystem::last_write_time(std::filesystem::path(File::loc()) / x.get()->file->path_v);

        if (last_modified  < last_) { last_modified = last_; has_changed = true; }

    } /// ???

    if (has_changed) {

        int i = 0;

        vbo.reset();

        for (auto &x : models) vbo.add_noupload(x.get()->file, i++) ;

        vbo.upload();

        has_changed = false;

    }

    ((ShaderProgram*)&shader)->create();

}
