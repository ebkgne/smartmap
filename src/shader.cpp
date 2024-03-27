#include "engine.hpp"
#include "shader.hpp"
#include "log.hpp"
#include "ubo.hpp"
#include "vbo.hpp"
#include "model.hpp"

#include <GL/gl3w.h>
#include <chrono>

#include <set>
#include "struct.hpp"

//// SHADERBUILDER

std::string ShaderProgram::Builder::layout(UBO* ubo) {

    if (!ubo->members.size() || !ubo->data.size()) return "";

    std::string str;



    str += "layout (binding = " + std::to_string(ubo->binding) + ", std140) uniform " + ubo->name() + " ";

    auto s = ubo->print(1);

    str += s.c_str()+s.find("{");

    str += ";\n\n";

    return str;

}

ShaderProgram::Builder::Builder() : vbo(nullptr) { }

ShaderProgram::Builder::Builder(VBO* vbo) : vbo(vbo) {

    header_common = "#version 430 core\n\n";

    stride_count = 0;

    header_fragment += layout(&engine.static_ubo);
    header_fragment += layout(&engine.dynamic_ubo);

}

std::string ShaderProgram::Builder::frag() {

    std::string str = header_common;

    std::set<Effector*> effectors;

    if (vbo) for (auto &model : vbo->models) for (auto &effector : model.effectors) effectors.insert(&effector);

    for (auto x : effectors)  str += x->s->print()+";\n\n";

    str += comment_line;

    str += "uniform sampler2D texture0;\n\n"; // foreach declared Texture::units maybe ?
    str += "uniform sampler2D medias;\n\n";

    str += "in vec2 UV;\n\n";
    str += "out vec4 COLOR;\n\n";

    str += "vec2 uv = UV;\n\n";
    str += "vec4 color = vec4(1);\n\n";

    str += comment_line;

    if (effectors.size()) str += comment_line;

    str += header_fragment;


    int model_id = 0;


    for (auto x : effectors) str += x->source() + "\n\n";

    str += "void next() { COLOR = color; uv = UV; color = vec4(1); }\n\n";

    str += comment_line;

    // main loop
    str += "void main() {\n\n";

    if (vbo) for (auto &model : vbo->models) {

        for (int instance = 0; instance < model.quantity(); instance++) {

            auto name = model.name();
            // name += "_"+std::to_string(model_id);
            if (model.quantity() > 1) name += "["+std::to_string(instance)+"]";

            str += "\t// "+name+"\n"; // would love this to be a node name instead

            for (auto &effector : model.effectors) {

                std::string arg_str;

                for (auto &arg : effector.args) {

                    arg_str += name+"."+effector.file->name()+"."+arg.second+", ";

                }

                arg_str.resize(arg_str.size()-2);

                str += "\t"+effector.file->name()+"("+arg_str+");\n";
            }

            str += "\tnext();\n\n";

        }

        str += "\n";

        model_id++;
    }

    str += "} ";

    return str;

}

std::string ShaderProgram::Builder::vert() {

    std::string str = header_common;

    str += "layout (location = 0) in vec2 POSITION;\n";
    str += "layout (location = 1) in vec2 TEXCOORD;\n";
    str += "layout (location = 3) in int OBJ;\n\n";

    str += "out vec2 UV;\n\n";

    str += "\nvoid main() {\n\n";

    str += "\tUV = TEXCOORD;\n";

    str += "\tgl_Position = vec4(POSITION.x,POSITION.y,0,1);\n\n";

    str += "}";

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
        PLOGW << (type==1?"vertex: ":"fragment: ") << &infoLog[7];
        PLOGV <<source;

    }

}

Shader::operator GLuint() { return id; }

ShaderProgram::~ShaderProgram() { destroy(); }

ShaderProgram::ShaderProgram() { Builder builder; create(builder.frag(),builder.vert()); }

ShaderProgram::ShaderProgram(VBO* vbo) { Builder builder(vbo); create(builder.frag(),builder.vert()); }

ShaderProgram::ShaderProgram(std::string frag, std::string vert) { create(frag,vert); }

void ShaderProgram::destroy() {

    loaded = false;
    if (id > -1) glDeleteProgram(id);

}

void  ShaderProgram::create(VBO* vbo) {

    Builder builder(vbo);

    create(builder.frag(),builder.vert());

}

void  ShaderProgram::create(std::string frag_src, std::string vert_src) {

    destroy();

    id = glCreateProgram();

    frag.create(frag_src,0);
    glAttachShader(id, frag.id);

    vert.create(vert_src,1);
    glAttachShader(id, vert.id);

    glLinkProgram( id );

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
