/*

    SMARTMAP (beta)

                        */


#include "log.hpp"
#include <memory>
#include <unordered_set>
#include <set>
#include <typeindex>


#include "struct2.hpp"
#include "buffer2.hpp"
#include "instance2.hpp"


struct ui { uint32_t v; };
struct float_ { float v; };
struct vec2 { float x = 0, y = 0; };
struct vec3 { float x = 0, y = 0, z = 0; };
struct vec4 { float x = 0, y = 0, z = 0, w = 0; };

namespace TEST {

std::string glsl_layout(Member& s) {

    auto bkp_srtiding = s.striding();

    s.striding(true);

    if (!s.members.size()) return "";

    std::string str = "struct " + s.name() + " {";

    for (auto &m : s.members) {
        
        std::string name = !m->typed() ? "struct" : m->type().name();
    
        str += " " + name + " " + m->name() + ";";
    
    }

    for (int i = 0; i < s.stride()/sizeof(float); i++) str += " float stride" + std::to_string(i) + ";";
    
    str += " };";
    
    s.striding(bkp_srtiding);

    return str;

}

};

void hard_delete(TEST::Member* a) {
    
          for (auto m : a->members) {
                
                hard_delete(m);


            }
                if (!a->typed()) {
                PLOGD << "delete " << a->name() << " :" << a->type().name();    
                    
                    delete a;
                    
                }
}



// struct Foo { 

//     virtual ~Foo() { PLOGD << "foo:"<<name; } 

//     std::string name = "foo"
    
// };

// struct Bar : Foo { 
    
//     Bar() { name = "bar"; }
//     ~Bar() { PLOGD << "bar:"<<name; }
    
// };

// void del(Foo* a) { delete a; }
// Foo* f = new Bar; del(f); return 0;

int main() {

using namespace TEST;

    logger.cout();

    // Struct& rectangle = Struct::create("rectangle").add<vec2>("reca").add<vec2>("recb");
    Struct& Rect = Struct::create("Rect").add<vec2>("pos").add<vec2>("size");//.add(rectangle);
    
    Buffer buff;

    Struct quad("myquad");

    quad.add(Rect);

    buff.add(quad);

    // buff.print();
    
    buff["myquad"].eq(1)["Rect"]["size"].set<uint32_t>(123);


    PLOGD << "BKP";

    auto bkp = buff.copy();

    /// is myquad copyed ? compre adresses

    PLOGD <<buff["myquad"].member;

    PLOGD <<bkp["myquad"].member;

    PLOGD <<"out";

    return 0;

    bkp["myquad"].eq(1)["Rect"]["size"].set<uint32_t>(245);

    PLOGD<<buff["myquad"].eq(1)["Rect"]["size"].get<uint32_t>();

    PLOGD<<bkp["myquad"].eq(1)["Rect"]["size"].get<uint32_t>();

    buff.remap(bkp);

    
    hard_delete(&bkp);


    // PLOGD<<buff["myquad"].eq(1)["Rect"]["size"].get<uint32_t>();

 
}


