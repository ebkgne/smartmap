
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <set>
#include <functional>

#include <boost/type_index.hpp>
#include <map>

using TypeIndex = boost::typeindex::type_index;
struct Type { 

    struct Definition {

        int size = 0;

        std::function<float(void*)> tofloat;

    };

    static inline std::map<TypeIndex, Definition> definitions;

    TypeIndex id;  
    
    std::string name() { return id.pretty_name(); }

    int size() { return definitions[id].size; }

    static Type find(std::string name) { 

        for (auto x : definitions) 
            if (x.first.pretty_name() == name) 
                return {x.first};

        std::cout << "nofind\n";
        
        return Type();
    
    } 
    
};


template <typename T>
struct TYPE : Type { 
    
    TYPE() { 
        
        id = typeid(T); 
        
        definitions[id].size = sizeof(T);

        // if (std::is_arithmetic<T>::value) definitions[id].tofloat = [&](void* ptr){ return (float)(*(T*)ptr); }; // ne compile pas, si trouve comment faire, super !

    }
};

void set(TypeIndex id, char* ptr, float val) {    

    if (id == typeid(float)) 
        (float&)*ptr = val;

    else if (id == typeid(double))
        (double&)*ptr =  val;

    else if (id == typeid(uint8_t))
        (uint8_t&)*ptr =  val;

    else if (id == typeid(uint16_t))
        (uint16_t&)*ptr =  val;

    else if (id == typeid(uint32_t)) 
        (uint32_t&)*ptr = val;

    else if (id == typeid(int8_t))
        (int8_t&)*ptr =  val;

    else if (id == typeid(int16_t))
        (int16_t&)*ptr =  val;

    else if (id == typeid(int32_t)) 
        (int32_t&)*ptr = val;

    else if (id == typeid(std::string))
        (std::string&)*ptr =  std::to_string(val);

}

struct Struct;
struct Clone;

struct Member   {

    struct Definition {
        
        std::shared_ptr<Member> type_v;
        std::string name;
        int quantity_v;
    
        Definition( std::shared_ptr<Member> type, std::string name = "", int q = 1, float from = 0, float to = 0, float def = 0) : name(name), quantity_v(q) {

            this->type(type);

            if (from || to || def)
                range(from, to, def); 
        }

        std::vector<char> rangedef; 
        std::vector<char> temprangedef; 

        uint32_t footprint_all() { return type_v->size() *  quantity_v ; }

        void type(std::shared_ptr<Member> type) {

            type_v = type;

            // if (type->type_v.id == typeid(uint32_t)) to = 65535;
            
            // else if (type->type_v.id == typeid(int)) to = 0;

            // else if (type->type_v.id == typeid(uint8_t)) to = 255;

            // else if (type->type_v.id == typeid(char)) to = -1;
            
            // range(0,to,0);

        }

        void quantity(int value) {

            quantity_v = value;

        }

        void range(float from, float to, float def) {

            if (!type_v->size()) 
                return;

            rangedef.resize(type_v->size()*3);
            memset(&rangedef[0],0,rangedef.size());

            set(type_v->type_v.id, this->from(), from);
            set(type_v->type_v.id, this->to(), to);
            set(type_v->type_v.id, this->def(), def);
   
        }

        char* from() { return rangedef.size()?rangedef.data():nullptr; }
        char* to() { return rangedef.size()?rangedef.data()+(type_v->size()):nullptr; }
        char* def();

    };

    std::string name;

    virtual int size() {
        int size = 0;
        for (auto def : members) 
            size+= def->footprint_all();
        return size;
    }
        

    uint32_t stride() { return 0 ; }
    uint32_t footprint() { return size() + stride() ; }

    std::set<std::shared_ptr<Struct>> observers;

    std::vector<std::shared_ptr<Definition>> members;

    int clone_size = 0;

    Type type_v;

    std::shared_ptr<Member> clone_v = nullptr;

    std::shared_ptr<Member> clone() {

        auto c = std::make_shared<Member>(*this);
        
        // c->clone_v = sha;

        return c;

    }

};

// struct Clone {

//     std::vector<Definition> members;

//     std::shared_ptr<Member> m = nullptr;

// }

struct Data : Member, std::enable_shared_from_this<Data> {

    Data(const TypeIndex& type)  { 
        type_v = Type(type); 
    }

    int size() override {
        return type_v.size();
    }

};

struct Struct : Member, std::enable_shared_from_this<Struct> {

    ~Struct() {
    
        if (!clone_v)
            for (auto def : members) 
                def->type_v->observers.erase(shared_from_this());

    }

    Struct() { type_v = TYPE<Struct>(); }

    virtual std::set<std::shared_ptr<Struct>> observe(int phase = 0) {

        std::set<std::shared_ptr<Struct>> found;

        if (!observers.size()) 
            found.insert(shared_from_this());
        
        for (auto observer : observers)     
            for (auto x :   observer->observe(phase))
                found.insert(x);

        return found;

    }

    std::shared_ptr<Definition> add(std::shared_ptr<Member> type, std::string name, int quantity = 1, float from = 0, float to = 0, float def = 0);

    std::shared_ptr<Definition> add(const TypeIndex& type, std::string name, int quantity, float from, float to, float def);

    template <typename T, int q = 1>
    std::shared_ptr<Definition> add(std::string name, float from = 0, float to = 0, float def = 0) {

        return add(TYPE<T>().id, name, q, from, to, def);
        
    }
    
    virtual void pre() {}
    virtual void post() {}

};

struct Register {

    std::set<std::shared_ptr<Data>> datatypes;
    std::set<std::shared_ptr<Struct>> structtypes;

    template <typename T>
    std::shared_ptr<Data> create(std::string name) {

        return create(TYPE<T>().id, name);
        
    }

    std::shared_ptr<Data> create(const TypeIndex& type, std::string name = "") {

        auto d = std::make_shared<Data>(type);

        d->name = (name.length()?name:d->type_v.name());

        // std::cout << "create " << d->quantity_v << " " << d->name << (d->quantity_v>1?"s":"")<<  " " << Type(type).name() << " - " << Type(type).size() << "\n";
        

        datatypes.insert(d);

        return d;
        
    }

    template <typename T>
    std::shared_ptr<Data> create() {

        return create<T>("");
        
    }

    std::shared_ptr<Struct> create(std::string name) {

        auto s = std::make_shared<Struct>();

        s->name = name;

        // std::cout << "create " << s->quantity_v << " " << s->name << (s->quantity_v>1?"s":"")<<  ""  << "\n";

        structtypes.insert(s);

        return s;
        
    }

    std::shared_ptr<Member> find(const std::string& name) {

        for (const auto& type : structtypes)
            if (type->name == name)
                return type;
 
        return nullptr;

    }
 
    std::shared_ptr<Member> find(const TypeIndex& type) {

        // type.type_info().name()
        for (const auto& x : datatypes) 
            if (x.get()->type_v.id == type) 
                return x;

        return create(type);
        
    }
 
    template <typename T>
    std::shared_ptr<Member> find() {
        return find(TYPE<T>().id);
    }

    std::shared_ptr<Member> changing = nullptr;

};

Register reg;

struct MemberQ { std::shared_ptr<Member::Definition> def; uint32_t eq = 0;};

struct Instance {

    std::vector<MemberQ> stl;

    uint32_t offset = 0;

    Instance(std::shared_ptr<Member::Definition> def) { stl.emplace_back(def); }

    std::string stl_name(int q = 0) {

        std::vector<std::string> names;

        if (q)
            q = stl.size()-q-1;
        
        for (int i = q; i < stl.size(); i++) {

            auto name = stl[i].def->name;
            if (stl[i].def->quantity_v>1) name += "[" + std::to_string(stl[i].eq) + "]";
            names.push_back(name);
            
        }

        std::string out;
        for (auto x : names) {
            
            out += x;
        
            if (x!=names.back())
                out += "::";
            
        }

        return out;

    }

    void each(std::function<void(Instance&)> cb);

    Instance& eq(int id) {

        auto &mq = stl.back();

        if (id >= mq.def->quantity_v) {

            // std::cout << << id << " >= " << mq.def->quantity_v << " (" << mq.def->name << ") ";
        
            return *this;
        
        }

        offset += mq.def->type_v->footprint()*(id-mq.eq);

        mq.eq = id;
        
        return *this;

    }

};


struct Buffer : Struct {

    Buffer(std::string name = "") { 
        this->name = name;
    }

    std::vector<char> data;

    std::vector<Instance> poss;

    void bkp();

    void remap();

    void pre() override { bkp(); }

    void post() override { remap(); }

    void print() {

        for (auto x : data)
            std::cout << std::to_string(x) << " ";
        
        std::cout << "\n";

    }

    Buffer(Buffer& other) : Struct(other), data(other.data) { }

};

std::shared_ptr<Member::Definition> Struct::add(const TypeIndex& type,std::string name, int quantity, float from, float to, float def) {

    return add(reg.find(type), name, quantity, from, to, def);

}

std::shared_ptr<Member::Definition> Struct::add(std::shared_ptr<Member> type,std::string newname, int quantity, float from, float to, float def) {

    type->observers.insert(shared_from_this());

    auto observers = observe();

    auto definition = std::make_shared<Definition>(type, newname, quantity, from, to, def);
    
    // std::cout << name << "[" << footprint() << "] add " << definition->name << "(" << definition->type->name << ":" << definition->type->footprint() << (definition->q > 1 ? "*" + std::to_string(definition->q)  + ":" + std::to_string(definition->footprint_all()) : "") << ")" << std::endl;

    reg.changing = nullptr;
    reg.changing =  std::make_shared<Member>(*this);
    reg.changing->clone_v =  shared_from_this();

    for (auto x :   observers)
        x->pre();

    members.emplace_back(definition);

    for (auto x :   observers)
        x->post();

    return definition;

}

char* Member::Definition::def() { 

    if (rangedef.size()) return rangedef.data()+(type_v->size()*2);
    
    if (type_v->type_v.id == typeid(Struct)) {

        if (!type_v->size())
            return nullptr;

        temprangedef.resize(type_v->size());

        int offset = 0;

        for (auto def : type_v->members) 

            for (int i = 0; i < def->quantity_v; i++) {

                memcpy(temprangedef.data()+offset, def->def(), def->type_v->size());    

                offset += def->type_v->size();

        }

        return temprangedef.data();

    }

    return nullptr; 
    
}

void Instance::each(std::function<void(Instance&)> cb) { 

    int offset= 0;

    for (auto def : stl.back().def->type_v->members) {

        Instance inst(*this);

        inst.stl.emplace_back(def);

        inst.offset += offset;
        
        for (int i = 0; i < def->quantity_v; i++) {

            inst.eq(i);

            inst.each(cb);

            cb(inst);
            
        }

        offset += def->footprint_all();

    }

}

void Buffer::bkp() {

    poss.clear();

    Instance inst(std::make_shared<Member::Definition>(shared_from_this()));

    inst.each([&](Instance& inst) {

        if (inst.stl.back().def->type_v == reg.changing->clone_v){
        
            poss.emplace_back(inst);
        
        }

    });

    std::reverse(poss.begin(), poss.end());

    if (!poss.size())
        poss = {inst};
    
    // std::cout <<  "bkp "  <<  name << " - " << size()  << std::endl;

}


void Buffer::remap() {

    std::shared_ptr<Member::Definition> new_def = nullptr;

    // for (auto x : reg.changing->clone_v->members) {

    //     bool found = false;

    //     for (auto y : reg.changing->members) 

    //         if (x == y) {

    //             found = true;
    //             break;

    //     }

    //     if (!found){

    //         new_def = x;

    //         break;

    //     }
        
    // }

    new_def = reg.changing->clone_v->members.back();

    int diff =  reg.changing->clone_v->size() - reg.changing->size();

    int last_size = data.size();

    data.resize(footprint());
    
    int end = data.size();
    
    
    for (auto x : poss) {

        int segsize = last_size-x.offset;

        end -= diff+ segsize;
        
        // print();

        // std::cout <<  "found "  <<  x.stl.back().def->name 
        
        // <<  " move "  <<  x.offset
        
        // <<  " to "  <<  last_size 

        // <<  " @ "  <<  end  
        
        //  << std::endl;

        std::move(data.begin()+x.offset, data.begin()+last_size, data.begin()+end);


        int nsplit = end + reg.changing->size();
        
        // print();

        // std::cout 
        
        // <<  " move "  <<  nsplit
        
        // <<  " to "  <<  nsplit + segsize  - reg.changing->size() 

        // <<  " @ "  <<   nsplit + diff  
        
        //  << std::endl;

        std::move(data.begin()+nsplit, data.begin()+nsplit + segsize  - reg.changing->size(), data.begin()+nsplit + diff);

        // print();

        if (new_def->def())
            for (int i = 0; i < new_def->quantity_v; i++) 
                memcpy(&data[nsplit]+new_def->type_v->size()*i, new_def->def(), new_def->type_v->size());
        else
            memset(&data[nsplit], 123, diff);

        // print();
        last_size-=segsize;


    }


    // std::cout <<  "rempa "  <<  name << " - " << size()  << std::endl;

}

int main() {





    auto buffer = std::make_shared<Buffer>("Buffy");
    
    auto foo = reg.create("foo");
    auto f1 = foo->add<uint8_t>("f1", 1,1,1);

    f1->type(reg.find<uint16_t>());
    f1->quantity(4);
    f1->range(9,9,9);

    auto bar = reg.create("bar");
    bar->add<uint8_t,2>("b1", 2,2,2);;
    bar->members.back()->range(8, 8, 8);

    auto zee = reg.create("zee");
    zee->add<uint8_t,2>("z1", 3,3,3);

    // test 1 : foo1 + foo2 -> add f2 to Foo
    // test 2 : foo[2] -> add f2 to Foo
    // test 3 : foo[5] -> add f2 to Foo
    // test 4:  foo1 + bar1 -> add f2 to Foo
    // test 5 : resize f1
    // test 6 : resize Foo

    // move rrange to def
    // finish range in ctor(add)

    buffer->add(foo, "foo1", 2); // notcalling def
    // buffer->data = {7};
    buffer->add(bar, "bar1", 2);

    // // buffer->data = {5,6};
    // buffer->print();
    foo->add<uint8_t, 3>("f1", 4,4,4);

    buffer->print();
    // std::cout <<  "-------- "  <<  std::endl;

    std::cout <<"DONE\n";

}
