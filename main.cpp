
#include "member.hpp"
#include <memory>

void Buffer_::Instance_::calc_offset() {

    // for (auto x : *list) 
    int offset = 0;

    // for (int i = 0; i < list.size()-2; i++) 
    //     for (auto def : list[i+1]->def->type_v->members) {
            
    //         if (def == list[i]->def)
    //             break;
            
    //         offset += def->footprint_all();
        
    //     }



}

std::vector<Instance> Member_::getInstances(Instance inst, int q) {

    std::vector<Instance> out;

    int i = 0;

    std::cout << inst->elements.back()->def->label << " " << inst->offset << "\n";

    if (!observers.size()) {// faster than dynacastbuffer?

        inst->owner = shared_from_this();
        out.push_back(inst);
        
        return out;
    }

    inst->elements.emplace_back(Element::Create());

    for (auto observer : observers) {
        int offset = inst->offset;
        for (auto def : observer.first->members) {
            if (def->type_v.get() == this) {

                inst->elements.back()->def = def;
                inst->offset = offset;

                for (auto stl_ : observer.first->getInstances(Instance::Create(inst),0)) {
                    out.push_back(stl_);

                }
            }
            offset += def->footprint_all();
        }
        i++;
    }

    return out;

}


auto nulld = Definition::Create(reg->create("null"));

void print(Instance in, Definition x = nulld) { 

    auto inst = in->elements.front();

    in->elements.insert(in->elements.begin(),Element::Create());
    for (auto def : inst->def->type_v->members) {

        in->elements.front()->def = def;
        
        def->type_v->size_v = def->type_v->footprint();   

        for (int i = 0; i < def->quantity_v; i++) {
                    
            in->elements.front()->eq = i;
            print(Instance::Create(in), x);
    
            if (x == nulld || x == def)
                std::cout <<  Buffer_::str(in) << " " << in->offset << "\n";

            in->offset += def->type_v->size_v;
        }

    }

    // in->offset = 0;

}

std::string str2(Instance inst) {

    std::string out;

    if (inst->owner)
        out =  inst->owner->name+"::";
    
    for (auto it = inst->elements.rbegin(); it != inst->elements.rend(); ++it) {
        

        out +=  it->get()->def->label;

        if (it->get()->def->quantity_v > 1)
            out += "[" + std::to_string(it->get()->eq) + "]";

        out += "::";


    }

    if (out.length())
        out = out.substr(0,out.length()-2);

    return out;

}


std::vector<Instance> getInstances2(Member x, Instance inst = Instance::Create()) {

    std::vector<Instance> out;
    
    // std::cout << inst->elements.back()->def->label << " " << inst->offset << "\n";

    std::cout << x->name << "\n";

    Buffer_* buff = dynamic_cast<Buffer_*>(x->shared_from_this().get());

    if (buff){


        inst->owner = x;
        out.push_back(inst);
        
        
    }

    for (auto observer : x->observers) { // find definitions

        for (auto def : observer.first->members) {

            if (def->type_v == x) {

                auto ninst = Instance::Create(inst);
                ninst->elements.emplace_back(Element::Create(def));

                std::cout << def->label << "\n";
                
                for (auto inst : getInstances2(observer.first, ninst)) 
                    out.push_back(inst); // push front

            }

        }
    }

    return out;

}

int main() {

    struct Foo {};
    
    auto f1 = std::make_shared<Foo>();
    auto a1 = Buffer::Create("kouta");

    std::shared_ptr<Member_> a4 = a1;
    Member a5 = a1->shared_from_this();
    auto a2 = a4.get();
    Member_* a3 = a2;
    auto x1 = dynamic_cast<Member_*>(a2);
    auto x2 = dynamic_cast<Member_*>(a3);
    auto x3 = dynamic_cast<Buffer_*>(a2);
    auto x4 = dynamic_cast<Buffer_*>(a3);
    auto x5 = dynamic_cast<Buffer_*>(a5.get());
    Buffer_* x6 = dynamic_cast<Buffer_*>(a5.get());





    auto test = reg->create("test");
    test->add<uint8_t,2>("offset2", 2,2,2 );
    auto target = test->add<uint8_t,2>("targetos", 3,3,3 );


    auto buffer1 = Definition::Create(Buffer::Create("Buffuno")->shared_from_this());
    buffer1->add<uint8_t,4>("offset", 1,1,1 );
    buffer1->add(test, 1 , "test1");
    buffer1->add(test, 1 , "test11");
    
    auto buffer2 = Definition::Create(Buffer::Create("Buffdos")->shared_from_this());
    buffer2->add(test, 1 , "test2");
    buffer2->add(test, 1 , "test22");




    Buffer_* x7 = dynamic_cast<Buffer_*>(buffer2->type_v.get());


    // std::cout << "############\n";
    // print( Instance::Create(buffer2));
    // std::cout << "############\n";
    // print( Instance::Create(buffer2), target);
    // std::cout << "############\n";
    // print( Instance::Create(buffer1));
    // std::cout << "############\n";

    // on veut 2 6 3 7 en ordre wesh bfs

    // finding up the hierarchy nto ok
    auto uc = target->type_v;
    std::cout << "############\n";
    auto xxx = getInstances2(uc); // forx obs recurse ()
    std::cout << "############\n";
    for (auto x : xxx) {

        std::cout << str2(x) << " - " << x->offset << "\n";
    
    }
    
    // // finding down the hierarchy ok 
    // std::cout << "############\n";
    //     Instance::Create(buffer1)->findBF(target); // maake return list // seee in old version


    // std::cout << "############\n";
        std::cout << "DONE" << "\n";
}


// in findbfs should return instance

// buff need to be def, on a dit buffer is not a Buffer , buffer is a def(Buffer)

// 1 1 21 122 122 122 122 23 4 4 5 5 5 5 5 5 21 122 122 122 122 23 4 4 5 5 5 5 5 5 2 2 1 1 21 122 122 122 122 23 4 4 5 5 5 5 5 5 21 122 122 122 122 23 4 4 5 5 5 5 5 5 2 2 