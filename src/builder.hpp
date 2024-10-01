#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <set>
#include "shader.hpp"

struct Member;
struct UBO;
struct VBO;
struct Texture;
struct Effector;

struct Builder {

    Builder();

    static inline std::string struct_taber = "";//\t";

    static inline std::string struct_spacer = " ";//\n\n";

    static inline std::string comment_line  = "///////////////////////////////////////////\n\n";

    std::string header_common, header_fragment , header_vertex, body_fragment , body_vertex , current_layer;

    std::vector<Effector*> effectors_fragment, effectors_vertex;

    std::set<UBO*> ubos;

    ShaderProgram* shader = nullptr;

    VBO* vbo = nullptr;

    std::string unique(Member* m);
    
    void build();
    
    virtual void setup();

    std::string frag();

    std::string vert();

    std::string layout();
    
    std::string ubo();

    bool add(UBO* ubo);

    struct VBOLayout { int loc; std::string type,name;};

    std::vector<VBOLayout> vbo_layouts;

    std::vector<Member*> definitions;

    std::map<Member*,std::string> unique_names;

    std::map<int,Texture*> samplers;

    int addSampler(Texture* tex, std::string name = "");

  };
