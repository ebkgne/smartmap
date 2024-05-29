#include "engine.hpp"
#include "shader.hpp"
#include "log.hpp"
#include "ubo.hpp"
#include "drawcall.hpp"
#include "model.hpp"
#include "atlas.hpp"

#include <GL/gl3w.h>
#include <chrono>

#include <set>
#include "struct.hpp"

//// SHADERBUILDER


static std::string print_recurse(Member* _this, int recurse=-1, int depth=0) {

    static const char* new_line = "\n";
    static const char* tab = "\t";

    static std::set<std::string> used;

    if (!depth) used.clear();

    if (false) { new_line = ""; tab = ""; }

    std::string tab_str;
    for (int i = 0 ; i < depth; i++) tab_str+=tab;
    std::string str;

    for (auto m : _this->members) {

        if (m != _this->members[0]) str += tab_str+tab;

        if ( m->isData() || m->isRef() ) {

            if (m->isRef() && m->members.size()) str += camel(m->type_name());

            else str += m->type_name();

        } else {

            if (recurse) {

                auto m_str = print_recurse(m, recurse-1, depth+1);

                if (!m_str.length()) continue;

                str += m_str;

            } else { str += camel(m->name()); }

        }

        str += " " + lower(m->name());

        if (m->quantity() > 1) str += "[" + std::to_string(m->quantity()) + "]";

        str += "; ";
        str += new_line;


    }

    if (_this->stride()) for (int i = 0; i < _this->stride()/sizeof(float); i++) {

        str += tab_str+tab;
        if (!new_line) str += " ";
        str += (_this->members[0]->type() == typeid(int) ? "int" : "float");
        str += " stride";
        str += std::to_string(i) + "; " + new_line;

    }

    if (!str.length()) return "";

    std::string struct_name = camel(_this->name());

    while (used.find(struct_name) != used.end()) struct_name+="_"; // GLSL DOES NOT DIFFERENCIATE NESTED STRUCTS, HERE Bar() WOULD CONFLICT : struct Bar{}; struct Foo { struct Bar {}; };

    used.insert(struct_name);

    return "struct " + struct_name  + " { " + new_line+new_line + tab_str +tab+ str + new_line + tab_str + "}";

}


void ShaderProgram::Builder::build() {

    header_common.clear();
    header_common += version;
    header_common += layout(&engine.static_ubo);
    header_common += layout(&engine.dynamic_ubo);

    frag();

    vert();

    vertex.clear();
    vertex += header_common;
    vertex += header_vertex;
    vertex += "\nvoid main() {\n\n";
    vertex += body_vertex;
    vertex += "}";

    fragment.clear();
    fragment += header_common;
    fragment += comment_line;
    fragment += header_fragment;
    fragment += "\nvoid main() {\n\n";
    fragment += body_fragment;
    fragment += "}";

}

void ShaderProgram::Builder::frag() { }

void ShaderProgram::Builder::vert() { }

std::string ShaderProgram::Builder::layout(UBO* ubo) {

    if (!ubo->members.size() || !ubo->data.size()) return "";

    std::string str;

    str += ubo->print_uniques();

    str += "layout (binding = " + std::to_string(ubo->binding) + ", std140) uniform " + ubo->name() + " ";

    auto s = print_recurse(ubo);

    str += s.c_str()+s.find("{");

    str += ";\n\n";

    return str;

}


//// SHADER

Shader::Shader() { }

Shader::~Shader() { if (id > -1) glDeleteShader(id);  }

Shader::Shader(std::string src, uint8_t type)  { create(src,type); }

void Shader::create(std::string src, uint8_t type)  {

    this->src = src;

    auto gl_type = GL_FRAGMENT_SHADER;

    if (type == 1) gl_type = GL_VERTEX_SHADER;
    if (type == 2) gl_type = GL_COMPUTE_SHADER;

    id = glCreateShader(gl_type);

    const GLchar *source = src.c_str();
    glShaderSource(id, 1, &source, nullptr);

    glCompileShader(id);

    // Check for errors

    GLchar infoLog[512];
    GLint success;

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {

        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::memset(std::strchr(infoLog, '\n'), 0, 1);
        PLOGE << (type==1?"vertex: ":"fragment: ") << &infoLog[7];
        PLOGV <<source;

    }

}

Shader::operator GLuint() { return id; }

ShaderProgram::~ShaderProgram() { destroy(); }

ShaderProgram::ShaderProgram() { Builder builder; create(builder.fragment,builder.vertex); }

ShaderProgram::ShaderProgram(std::string frag, std::string vert) { create(frag,vert); }

void ShaderProgram::destroy() {

    loaded = false;
    if (id > -1) glDeleteProgram(id);

}


void  ShaderProgram::create() { Builder builder; builder.build(); create(builder.fragment, builder.vertex); }

void  ShaderProgram::create(Builder* builder) { builder->build(); create(builder->fragment, builder->vertex); }

void  ShaderProgram::create(std::string frag_src, std::string vert_src) {

    destroy();

    id = glCreateProgram();

    frag.create(frag_src,0);
    glAttachShader(id, frag.id);

    vert.create(vert_src,1);
    glAttachShader(id, vert.id);

    glLinkProgram( id );

    // engine.atlas->link(this);

    engine.dynamic_ubo.bind(this);

    engine.static_ubo.bind(this);

    sendUniform("medias", 1);
    sendUniform("render_pass", 2);

    loaded = true;

    last_change = std::chrono::system_clock::now();

    use();

}

void ShaderProgram::use() {  glUseProgram(id); }

void ShaderProgram::use(uint32_t x, uint32_t y, uint32_t z) {  glUseProgram(id); glDispatchCompute(x,y,z); }

ShaderProgram::operator uint32_t() { return id; }

int ShaderProgram::getLoc(const std::string& name) { return glGetUniformLocation(id, name.c_str()); }

void ShaderProgram::sendUniform(const std::string& name, int i1) { glProgramUniform1i(id, getLoc(name), i1); }

void ShaderProgram::sendUniform(const std::string& name, float f1) { glProgramUniform1f(id, getLoc(name), f1); }

void ShaderProgram::sendUniform(const std::string& name, float f1, float f2) { glProgramUniform2f(id, getLoc(name), f1, f2); }

void ShaderProgram::sendUniform(const std::string& name, float f1, float f2, float f3) { glProgramUniform3f(id, getLoc(name), f1, f2, f3); }

void ShaderProgram::sendUniform(const std::string& name, float f1, float f2, float f3, float f4) { glProgramUniform4f(id, getLoc(name), f1, f2, f3, f4); }

//void Shader::sendUniform(std::string name, glm::mat4 mat) { glProgramUniformMatrix4fv(program, getLoc(name), 1, GL_FALSE, &mat[0][0]); }
