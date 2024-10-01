#include "shader.hpp"
#include "atlas.hpp"
#include "builder.hpp"
#include "texture.hpp"

#include <GL/gl3w.h>
#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <chrono>
#include <regex>

#include <string>

#include <boost/algorithm/string.hpp>

Shader::Shader() { }

Shader::~Shader() { if (id > -1) glDeleteShader(id);  }

Shader::Shader(std::string src, uint8_t type)  { create(src,type); }


bool Shader::create(std::string src, uint8_t type)  {

    this->src = src;

    auto gl_type = GL_FRAGMENT_SHADER;

    if (type == 1) gl_type = GL_VERTEX_SHADER;
    if (type == 2) gl_type = GL_COMPUTE_SHADER;

    id = glCreateShader(gl_type);

    const GLchar *source = src.c_str();
    glShaderSource(id, 1, &source, nullptr);

    glCompileShader(id);

    // Check for errors

    memset(&infoLog[0],0,512);

    GLint success;

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    errors.clear(); 
    if (!success) {

        glGetShaderInfoLog(id, 512, NULL, &infoLog[0]);

        auto lines = split(infoLog, "\n");
    

        std::map<int, std::vector<std::string>> split_errors;

        for (auto line : lines) {
            
            std::smatch match;
            std::regex color_regex(".?([0-9]+):([0-9]+)(\\(([0-9]+)\\): error)?: (.+)", std::regex_constants::icase);

            if (std::regex_search(line, match, color_regex)) {

                if (match.size()<5)
                    continue;

                int lineid = std::stoi(match[2].str());
                auto str = match[5].str();

                if (!lineid || !str.length())
                    continue;
                
                auto& error_strs = split_errors[std::stoi(match[2].str())];
                
                bool found = false;
                for (auto e : error_strs) 
                    if (e == str) {
                        found = true;
                        break;
                    }

                if (found)
                    break;
                else
                    error_strs.push_back(str);

            }

        }
        
        for (auto &e : split_errors) 
            errors[e.first] = join(e.second, " && ");

        std::string output;

        if (type == FRAGMENT) output += "fragment:";
        else if (type == VERTEX) output += "vertex:";
        else if (type == COMPUTE) output += "compute:";
        output += "";

        for (auto error : errors) {

            std::string error_str = std::to_string(error.first);

            error_str += ": " + error.second + " && ";

            error_str = error_str.substr(0,error_str.length()-4);


            int count = 0;
            for (std::string line : split(src, "\n") ) {
                if (count++ == error.first-1) {
                    boost::trim_left(line);
                    error_str += " : " + line;    
                    break;
                    
                }
            }

            output += error_str;
            
        }


        PLOGE << output;
        PLOGV << "\n" << source;
        return success;
        int count = 1;

        for (std::string line: split(source, "\n") ) {

            PLOGV << count++ << ": " << line;

        }

    }

    return success;

}

Shader::operator GLuint() { return id; }

ShaderProgram::~ShaderProgram() { 
    
    destroy();


}
ShaderProgram::ShaderProgram() { 
    


}


ShaderProgram::ShaderProgram(std::string frag, std::string vert) { create(frag,vert); }

void ShaderProgram::destroy() {

    loaded = false;
    if (id > -1)
        glDeleteProgram(id);

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


    use();

}

void ShaderProgram::use() {  glUseProgram(id); }

void ShaderProgram::use(uint32_t x, uint32_t y, uint32_t z) {  glUseProgram(id); glDispatchCompute(x,y,z); }

int ShaderProgram::getLoc(const std::string& name) { return glGetUniformLocation(id, name.c_str()); }

void ShaderProgram::sendUniform(const std::string& name, int i1) { glProgramUniform1i(id, getLoc(name), i1); }

void ShaderProgram::sendUniform(const std::string& name, float f1) { glProgramUniform1f(id, getLoc(name), f1); }

void ShaderProgram::sendUniform(const std::string& name, float f1, float f2) { glProgramUniform2f(id, getLoc(name), f1, f2); }

void ShaderProgram::sendUniform(const std::string& name, float f1, float f2, float f3) { glProgramUniform3f(id, getLoc(name), f1, f2, f3); }

void ShaderProgram::sendUniform(const std::string& name, float f1, float f2, float f3, float f4) { glProgramUniform4f(id, getLoc(name), f1, f2, f3, f4); }

//void Shader::sendUniform(std::string name, glm::mat4 mat) { glProgramUniformMatrix4fv(program, getLoc(name), 1, GL_FALSE, &mat[0][0]); }
