
#include "member.hpp"


std::vector<STL> Member_::getSTLS(STL stl, int q) {

    std::vector<STL> out;

    int i = 0;

    if (!observers.size()) {// faster than dynacastbuffer?

        out.push_back(STL::Create(*stl));
        
        return out;
    }

    stl->emplace_back(Instance::Create());

    for (auto observer : observers) {
        
        for (auto def : observer.first->members) 
            if (def->type_v.get() == this) {

                stl->back()->def = def;

                for (auto stl_ : observer.first->getSTLS(stl,0)) {
                    out.push_back(stl_);

                }
            }
    
        i++;
    }

    return out;

}

int main() {

    auto aa = reg.create("aa");
    aa->add<uint8_t>("A", 1,1,1);
    
    auto bb = reg.create("bb");
    bb->add<uint8_t>("B", 2,2,2);
    
    auto cc = reg.create("cc");
    auto cval = cc->add<uint8_t>("C", 3,3,3);
    
    auto trick = reg.create("trick");

    auto t1 = reg.create("t1");
    t1->add<uint8_t>("B", 21,21,21);
    trick->add(t1);
    auto t2 = reg.create("t2");
    auto t22 = reg.create("t22");
    t22->add<uint8_t,2>("B", 122,122,122);
    t2->add(t22,2);
    trick->add(t2);
    auto t3 = reg.create("t3");
    t3->add<uint8_t>("B", 23,23,23);
    trick->add(t3);

    auto test = reg.create("test");
    test->add(aa, 2);
    test->add(trick, 2);
    test->add(bb, 2);
    test->add(cc, 2,"tcc");

    auto buffer = Buffer::Create("Buffy");
    buffer->add(test,2);

    buffer->print();

    std::cout << "############\n";

    trick->add<uint8_t,2>("T", 4,4,4);
    // buffer->print();
    cc->quantity(cval,3);
    cc->quantity(cval,0);


    auto dd = reg.create("dd");
    auto ddin = reg.create("ddin");
    dd->add(ddin,1,"DDIN");
    auto trickdd = ddin->add<uint8_t,3>("D", 5,5,5 );
    trick->add(dd,2);

    // trickdd->track(buffer);

    ;

    auto buffer2 = Buffer::Create("Buffdos");
    auto trickdos = reg.create("trickdos");
    buffer2->add(trickdos);
    trickdos->add(test);

    auto bubuff = Instance::Create(Definition::Create(buffer->shared_from_this()));

    for (auto x : ddin->getSTLS(STL::Create({Instance::Create(trickdd)}),0)) {

        std::cout << str(x) << " - " << x->size() << "\n";
    
    }

    buffer->print();

    // buffer->track({});

    std::cout << buffer->size_v << " " << buffer->data.size() << " DONE\n";

}

// 1 1 21 122 122 122 122 23 4 4 5 5 5 5 5 5 21 122 122 122 122 23 4 4 5 5 5 5 5 5 2 2 1 1 21 122 122 122 122 23 4 4 5 5 5 5 5 5 21 122 122 122 122 23 4 4 5 5 5 5 5 5 2 2 