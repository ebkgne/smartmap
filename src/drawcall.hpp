#pragma once

#include "vbo.hpp"
#include "shader.hpp"

#include <vector>
#include <cstring>

struct Renderable {

    Struct s;

    Renderable(std::string name = "sdfklg" ) : s(name) {}

    std::vector<std::shared_ptr<Model>> models;

    std::vector<std::shared_ptr<Effector>> effectors;

    bool removeModel(Model* model); // kinda ctor for Model

    Effector* addEffector(File* f);

    bool removeEffector(Effector* effector); // kinda ctor for Effector

    Model* addModel(File* f);

};

struct DrawCall {

    DrawCall(std::string name);

    VBO vbo;

    ShaderProgram shader;

    Struct s;

    virtual void update();

    virtual void draw();

    std::vector<std::shared_ptr<Model>> models;

    std::vector<std::shared_ptr<Effector>> effectors;

    bool remove(Model* model);

    Effector* addEffector(File* f);

    bool removeEffector(Effector* effector);

    Model* add(File* f);

    struct ShaderProgramBuilder : ShaderProgram::Builder {

        std::set<File*> effectors;

        void build() override;
        void frag() override;
        void vert() override;

        DrawCall* dc;

        ShaderProgramBuilder();

        ShaderProgramBuilder(DrawCall* dc);

        int stride_count = 0;

    };

    ShaderProgramBuilder builder;

};
