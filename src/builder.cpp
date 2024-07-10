#include "builder.hpp"


#include "ubo.hpp"
#include "vbo.hpp"
#include "shader.hpp"
#include "effector.hpp"

#include "utils.hpp"

void Builder::build() {

    header_common = version;
    header_common += layout();

    samplers.clear();
    
    if (!shader) return;

    for (auto s : samplers) shader->sendUniform(s.second, s.first);

    for (auto x : ubos) x->bind(shader);

}

std::string Builder::frag() {

    std::string samplers_str; for (auto x : samplers) samplers_str += "uniform sampler2D "+x.second+";\n"; if (samplers.size()) samplers_str += "\n";

    std::string effectors_str; for (auto x : effectors_fragment)  effectors_str += x->method+"\n\n";

    std::string ins_str;

    for (int i = 1; i < vbo->vertice->members.size(); i++) {

        auto m = vbo->vertice->members[i];

        ins_str += "in "+std::string(m->type() == typeid(int)?"flat ":"")+m->type_name()+" "+m->name()+";\n";

    } if (ins_str.length()) ins_str += "\n";

    return

        header_common +

        "out vec4 COLOR;\n\n" +

        ins_str +

        samplers_str + (samplers_str.length()?comment_line:"") +

        header_fragment + (header_fragment.length()?comment_line:"") +

        effectors_str + (effectors_str.length()?comment_line:"") +

        "void main() {\n\n" +

        (body_fragment.length()?body_fragment:"\tCOLOR = vec4(0,1,0,1);\n\n") +

        "}\n\n" + comment_line;

}

std::string Builder::vert() {

    vbo_layouts.clear();

    if (vbo) {

        int count = 0;

        for (auto x : vbo->vertice->members)

            vbo_layouts.push_back({count++,x->type_name(), (x->name()+(count>1?"_":""))});

    }

    if (!vbo_layouts.size()) vbo_layouts.push_back({0,"vec2","POSITION"});

    std::string layouts_str; for (auto l : vbo_layouts) layouts_str += "layout (location = "+std::to_string(l.loc)+") in "+l.type+" "+l.name+";\n"; if (samplers.size()) layouts_str += "\n";

    std::string effectors_str; for (auto x : effectors_vertex)  effectors_str += x->source()+"\n\n";

    return

        header_common + comment_line +

        layouts_str + (layouts_str.length()?comment_line:"") +

        header_vertex + (header_vertex.length()?comment_line:"") +

        effectors_str + (effectors_str.length()?comment_line:"") +

        "void main() {\n\n" +

        (body_vertex.length()?body_vertex:"\tvec2 POS = POSITION;\n\n\tgl_Position = vec4(POS, 0, 1);\n\n") +

        "}\n\n" + comment_line;

}


bool Builder::add(UBO* ubo) { return true; }

std::string Builder::layout() {

    std::vector<Member*> structs;

    for (auto ubo : ubos)    {

        ubo->each([&](Instance& inst) {

            auto m = inst.def();

            if (m->type() == typeid(Struct))
                ADD_UNIQUE<Member*>(structs,m->ref()?m->ref():m);

        });

        ADD_UNIQUE<Member*>(structs,ubo);

    }

    std::string out;

    std::map<Member*,std::string> unique_name_list;

    for (auto m : structs) {

        auto name = camel(m->name());

        while (true) {

            for (auto &x : unique_name_list) if (x.second == name) {

                name += "_";

                continue;

            }

            break;

        }

        unique_name_list[m] = name;

    }

    for (auto x : structs) { auto def = print_struct(x,unique_name_list); if (def.length()) def+=";\n\n"; out += def; }

    for (auto ubo : ubos) {

        out += "layout (binding = " + std::to_string(ubo->binding) + ", std140) uniform " + ubo->name() + "_ ";

        out += " { " + camel(ubo->name()) + " " + lower(ubo->name());

        if (ubo->quantity()>1) out += "["+std::to_string(ubo->quantity())+"]";

        out += + "; };\n\n";

    }

    return out;

}

std::string Builder::print_struct(Member* member, std::map<Member*,std::string> &unique_name_list) {

    if (!member->size()) return "";

    std::string out;
    std::string nl = "";
    std::string tb = "";
    // if (member->members.size() == 1) nl = "";

    std::string content;

    for (auto x : member->ref()?member->ref()->members:member->members) {

        if (!x->size()) continue;

        auto x_ = x->ref()?x->ref():x;

        auto name = unique_name_list.find(x_)!=unique_name_list.end()?unique_name_list[x_]:x_->type_name();

        content+=tb+""+name+" "+lower(x->ref()?x->type_name():x->name());

        if (x->quantity()>1) content += "["+std::to_string(x->quantity())+"]";

        content += "; "+nl;

    }

    if (!content.length()) return "";

    out+="struct "+unique_name_list[member]+" { "+nl+nl+content;

    if (member->stride()) for (int i = 0; i < member->stride()/sizeof(float); i++) {

        out += tb;
        out += (member->members.back()->type() == typeid(int) ? "int" : "float");
        out += " stride";
        out += std::to_string(i) + "; "+nl ;

    }

    out+=nl+"}";
    return out;

}
