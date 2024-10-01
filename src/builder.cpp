#include "builder.hpp"


#include "ubo.hpp"
#include "globals.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "instance.hpp"
#include "effector.hpp"

#include "utils.hpp"
#include <string>

Builder::Builder() {


}


static void addSmallerFirst(Member* m, std::vector<Member*>& v) {

    m = m->ref();

    if (m->isData())
        return;

    if (!m->force_ref)
        for (auto x : m->members) 
            addSmallerFirst(x->ref(), v);

    ADD_UNIQUE<Member*>(v, m);

}

void Builder::setup() {

}

std::string Builder::unique(Member* m ) {

    auto name = m->_name();

    while (true) {

        bool found = false;

        for (auto &x : unique_names) 
            
            if (lower(x.second )== lower(name)) {
        
                name += "0";
                found = true;
                break;

            }
        
        if (!found)
            break;

    }

    return name;

}

void Builder::build() {

    PLOGV << "build";

    shader->destroy();

    samplers.clear();

    effectors_fragment.clear();
    effectors_vertex.clear();

    header_fragment.clear();
    body_fragment.clear();

    header_common.clear();
    header_vertex.clear();
    body_vertex.clear();

    definitions.clear();
    
    unique_names.clear();

    ubos.clear();

    setup();

    for (auto m : ubos) 
        addSmallerFirst(m, definitions);

    for (auto m : definitions) 
        unique_names[m] = unique(m);
    
    header_common = "#version 430 core\n\n"+header_common;
    
    header_common += layout();

    header_common += ubo();

    if (!shader) 
        return;

    shader->create(frag(), vert());

    for (auto ubo : ubos) 
        ubo->bind(shader->id);

    int sampler_id = 0;
    for (auto s : samplers) {
    
        if (s.first > sampler_id) 
            sampler_id = s.first;

        s.second->bind(s.first);
        shader->sendUniform(s.second->sampler_name, s.first);
        
        sampler_id++;
    } 
}


int Builder::addSampler(Texture* tex, std::string name) {
    
        int i = 1;

        for (auto x : samplers) {
            
            if (x.second == tex)
                return x.first;
            
            if (x.first > i)
                i = x.first+1;  
        
        }

        samplers[i] = tex;

        tex->sampler_name = name+"_pass";

        return i;
}

std::string Builder::frag() {

    std::string samplers_str; 

    int id = 0;
    
    for (auto x : samplers) {

        if (x.first > id) {
            // for ( int i = id; i < x.first; i++) samplers_str += "uniform sampler2D randi"+std::to_string(i)+";\n"; 
            id = x.first;
        }

        samplers_str += "uniform sampler2D "+x.second->sampler_name+";\n"; 
        id++;

    }
    
    if (samplers.size()) samplers_str += "\n";
    

    std::string effectors_str; 
    for (auto x : effectors_fragment)  
        effectors_str += x->header()+"\n\n";

    std::string ins_str;

    for (int i = 1; i < globals.vertice.members.size(); i++) {

        auto m = globals.vertice.members[i];

        ins_str += "in "+std::string(m->type().id == typeid(int)?"flat ":"")+m->type_name()+" "+m->_name()+";\n";

    } 
    
    if (ins_str.length()) ins_str += "\n";

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

        for (auto x : globals.vertice.members)

            vbo_layouts.push_back({count++,x->type_name(), (x->_name()+(count>1?"_":""))});

    }

    if (!vbo_layouts.size()) vbo_layouts.push_back({0,"vec2","POSITION"});

    std::string layouts_str; for (auto l : vbo_layouts) layouts_str += "layout (location = "+std::to_string(l.loc)+") in "+l.type+" "+l.name+";\n"; if (samplers.size()) layouts_str += "\n";

    std::string effectors_str; for (auto x : effectors_vertex)  
        effectors_str += x->header()+"\n\n";

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




std::string Builder::ubo() {

    std::string out;

    for (auto ubo : ubos) {

        if (!ubo->footprint())  
            continue;

        out += "layout (binding = " + std::to_string(ubo->binding) + ", std140) uniform " + ubo->_name() + "_ ";

        out += " { " + camel(ubo->_name()) + " " + lower(ubo->_name());

        if (ubo->quantity()>1) out += "["+std::to_string(ubo->quantity())+"]";

        out += + "; };\n\n";

    }

    return out;

}

std::string Builder::layout() {

    std::string out;

    for (auto member : definitions) { 

        if (!member->size()) continue;

        std::string line;
        std::string nl = "";
        std::string tb = "";
        // if (member->members.size() == 1) nl = "";

        std::string content;

        if (member->ref() != member)
            return line;

        auto name = camel(unique_names[member]);

        if (member->force_ref)
            member = member->members[0];   
        for (auto x : member->members) {

            if (!x->size()) continue;
            
            auto ref = x->ref();   


            content+=tb+""+(x->isData()?x->type_name():camel(unique_names[ref]))+" "+lower(x->_name());

            if (!ref->isData() && ref->quantity()>1) content += "["+std::to_string(ref->quantity())+"]";

            content += "; "+nl;

        }

        if (!content.length()) 
            continue;

        line+="struct "+name+" { "+nl+nl+content;

        if (member->stride()) for (int i = 0; i < member->stride()/sizeof(float); i++) {

            line += tb;
            line += (member->members.size() && member->members.back()->type().id == typeid(int) ? "int" : "float");
            line += " stride";
            line += std::to_string(i) + "; "+nl ;

        }

        line+=nl+"}";
        if (line.length()) 
            line+=";\n\n"; 
        out+=line;

    }
    
    return out;

}


