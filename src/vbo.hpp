#ifndef VBO_H
#define VBO_H

#include "pch.hpp"
#include "gui.hpp"
#include "buffer.hpp"

struct Models{};

struct VBO {

    static inline std::vector<VBO*> pool;

    std::string name;

    Buffer buffer;
    Buffer::Struct *vertices, *indices;

    GLuint vao=0, vbo, ibo, id;

    std::vector<Models*> models;
    
    ~VBO(); 

    void import(std::string path);

    void upload();
    void destroy();

    VBO(std::string path = "quad.obj", int id = 0, std::string name = "VBO") ;

    void draw(int count = 1);



};

#endif