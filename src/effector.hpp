#pragma once

#include "member.hpp"

#include <vector>
#include <string>

struct File;
struct Builder;

struct Effector : Member {

    enum Type { FRAGMENT, VERTEX, COMPUTE } type = FRAGMENT;

    virtual bool setup(Builder* builder) = 0;

    virtual std::string body(Builder* builder, std::string prepend = "");

    virtual std::string header() { return source_v; }

    std::string call(std::string prepend="");

    std::string source_v;

    Effector(std::string name = "effector");

};

struct FileEffector : Effector {

    File* file;

    FileEffector(File* file, std::string name = "");
    
    void load();

    bool setup(Builder* builder) override;

};

struct EffectorRef : Member  {

    Effector* effector;

    EffectorRef(std::string name, Effector* effector);

};

struct Effectable : Member {

    Effectable(std::string name = "Effectable" );
    
    ~Effectable();

    std::vector<std::shared_ptr<EffectorRef>> effector_refs;
    EffectorRef* addEffector(Effector* effector); // kinda ctor for effectors
    bool removeEffector(EffectorRef* ref);

    Instance* dyninst = nullptr; // thus in modelable, dc, layer and so on...
    Instance* statinst = nullptr;
    
};


struct Wrappy : Effector, Effectable {

    bool setup(Builder* builder) override;

    Wrappy(std::vector<Effector*> effectors = {}, int count = 3, std::string name = "wrapperEffector");

    int count;

    void attrs(int count);

    std::string header() override;

};