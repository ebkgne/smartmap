#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstddef>
#include <map>
#include <memory>
#include <unordered_set>
#include <set>

#include "member.hpp"
#include "data.hpp"
#include "log.hpp"


struct Struct : Member {

    Struct(std::string name = "", uint32_t quantity = 1);

    ~Struct();

    virtual void pre_change() {}
    virtual void post_change() {}

    Struct& add(Member& m);

    template <typename T>
    Struct& add(std::string name = "") { auto n = new Data<T>(name); add(*n); return *this; }

    Struct& add(const char* name) ;

    Struct& range(float from, float to, float def) ;

    Struct& remove(Member& s) ;

    void update() override;

    std::type_index type() override ;

    uint32_t size() override ;

    std::string print(int recurse = 0) override;

    Struct* copy() ;

    Member* copy(Member* x) override ;

    void hard_delete();

protected:
    uint32_t size_v = 0;
public:


    static inline std::set<Struct*> owned;

    static Struct& create(std::string name, uint32_t quantity = 1);

    static Struct* exist(std::string name);

    static Struct& id(std::string name);

    static void clear();

    static bool destroy(std::string name) ;
};
