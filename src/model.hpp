#pragma once

#include <vector>
#include <string>

#include "effector.hpp"

struct Model;

struct Modelable : Effectable {

    using Effectable::Effectable;

    std::vector<std::shared_ptr<Model>> models;
    virtual Model* addModel(File* f); // kinda ctor for Model
    bool removeModel(Model* model);

};

struct Model : Effectable {

    Member stat;

    File* file;

    uint32_t dimensions_v[2] = {0,0};
    
    uint32_t grid_v[2] = {1,1};

    void dimensions(uint32_t width, uint32_t height);

    bool vbo_split_mode = false;

    Model(File* f, std::string name);

    ~Model();

    static void convert(File* model, std::string type);

};
