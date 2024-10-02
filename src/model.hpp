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

    File* file;

    Model(File* f, std::string name);

    ~Model();

    static void convert(File* model, std::string type);
    
    int quantity_v[3] = {1,1,1};
    
    enum Cloner {

        CUBE,
        SPHERE,
        CYLINDER

    } cloner = CUBE;

};


struct Cloner : Model {

    Cloner(File* f, std::string name);

    Member stat;

    uint32_t quantity_v[3] = {1,1,1};
    
    enum Mode {

        CUBE,
        SPHERE,
        CYLINDER

    } mode = CUBE;


};