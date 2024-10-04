
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <set>
#include <functional>

#include <boost/type_index.hpp>
#include <map>

using TypeIndex = boost::typeindex::type_index;

struct Type { 

    struct Definition { int size = 0; std::function<float(void*)> tofloat; };

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

        // if (std::is_arithmetic<T>::value) definitions[id].tofloat = [&](void* ptr){ return (float)(*(T*)ptr); }; // this wont compile

    }
};

void set(TypeIndex id, char* ptr, float val) { // but hopefully could replace that 

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

struct Member : std::enable_shared_from_this<Member>  {

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
        char* def() { 

            if (rangedef.size()) return rangedef.data()+(type_v->size()*2);
            
            if (type_v->type_v.id == typeid(Member)) {

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

    };

    std::string name;

    virtual ~Member() {
    
        if (!clone_v)
            for (auto def : members) 
                def->type_v->observers.erase(shared_from_this());

    }

    virtual int size() {
        int size = 0;
        for (auto def : members) 
            size+= def->footprint_all();
        return size;
    }
        

    uint32_t stride() { return 0 ; }
    uint32_t footprint() { return size() + stride() ; }

    std::set<std::shared_ptr<Member>> observers;

    std::vector<std::shared_ptr<Definition>> members;

    int clone_size = 0;

    Type type_v = TYPE<Member>();

    std::shared_ptr<Member> clone_v = nullptr;

    std::shared_ptr<Member> clone() {

        auto c = std::make_shared<Member>(*this);
        
        // c->clone_v = sha;

        return c;

    }
  
    virtual void pre(std::shared_ptr<Member> changing = nullptr) {}
    virtual void post(std::shared_ptr<Member> changing = nullptr) {}

    std::set<std::shared_ptr<Member>> observe(int phase = 0) {

        std::set<std::shared_ptr<Member>> found;

        if (!observers.size()) 
            found.insert(shared_from_this());
        
        for (auto observer : observers)     
            for (auto x :   observer->observe(phase))
                found.insert(x);

        return found;

    }
};

struct Data : Member {

    Data(const TypeIndex& type)  { 
        type_v = Type(type); 
    }

    int size() override {
        return type_v.size();
    }

};

struct Struct : Member {

    std::shared_ptr<Definition> add(std::shared_ptr<Member> type, std::string name, int quantity = 1, float from = 0, float to = 0, float def = 0) {

        type->observers.insert(shared_from_this());

        auto observers = observe();

        auto definition = std::make_shared<Definition>(type, name, quantity, from, to, def);
        
        // std::cout << name << "[" << footprint() << "] add " << definition->name << "(" << definition->type->name << ":" << definition->type->footprint() << (definition->q > 1 ? "*" + std::to_string(definition->q)  + ":" + std::to_string(definition->footprint_all()) : "") << ")" << std::endl;

        std::shared_ptr<Member> changing =  std::make_shared<Member>(*this);
        
        changing->clone_v =  shared_from_this();

        for (auto x :  observers)
            x->pre(changing);

        members.emplace_back(definition);

        for (auto x :   observers)
            x->post(changing);

        return definition;

    }

    std::shared_ptr<Definition> add(const TypeIndex& type, std::string name, int quantity, float from, float to, float def);

    template <typename T, int q = 1>
    std::shared_ptr<Definition> add(std::string name, float from = 0, float to = 0, float def = 0) {

        return add(TYPE<T>().id, name, q, from, to, def);
        
    }
    

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


struct Buffer : Struct {

    Buffer(std::string name = "") { 
        this->name = name;
    }

    std::vector<char> data;

    std::vector<int> changing_offsets;

    void find(std::shared_ptr<Member> x, std::shared_ptr<Member> curr = nullptr, int offset= 0) {

        if (!curr)
            curr = shared_from_this();

        for (auto def : curr->members) {

            if (def->type_v == x->clone_v) {
            
                auto ioffset = offset;
            
                for (int i = 0; i < def->quantity_v; i++) {
        
                    changing_offsets.emplace_back(ioffset);

                    ioffset+=def->type_v->footprint();

                }

            }

            find(x,def->type_v,offset);

            offset += def->footprint_all();

        }

    }

    void bkp(std::shared_ptr<Member> changing) {

        changing_offsets.clear();

        find(changing);

        std::reverse(changing_offsets.begin(), changing_offsets.end());

        if (!changing_offsets.size())
            changing_offsets = {0};

    }

    void remap(std::shared_ptr<Member> changing) {

        int diff =  changing->clone_v->size() - changing->size();

        if (diff >0) { // thus ADDING

            std::shared_ptr<Member::Definition> new_def = nullptr;

            // for (auto x : changing->clone_v->members) {

            //     bool found = false;

            //     for (auto y : changing->members) 

            //         if (x == y) {

            //             found = true;
            //             break;

            //     }

            //     if (!found){

            //         new_def = x;

            //         break;

            //     }
                
            // }

            new_def = changing->clone_v->members.back();

            int last_size = data.size();

            data.resize(footprint());
            
            int end = data.size();
            
            for (auto x : changing_offsets) {

                int segsize = last_size-x;

                end -= diff+ segsize;
                
                // print();

                // std::cout <<  "found "  <<  x.stl.back().def->name 
                
                // <<  " move "  <<  x.offset
                
                // <<  " to "  <<  last_size 

                // <<  " @ "  <<  end  
                
                //  << std::endl;

                std::move(data.begin()+x, data.begin()+last_size, data.begin()+end);


                int nsplit = end + changing->size();
                
                // print();

                // std::cout 
                
                // <<  " move "  <<  nsplit
                
                // <<  " to "  <<  nsplit + segsize  - changing->size() 

                // <<  " @ "  <<   nsplit + diff  
                
                //  << std::endl;

                std::move(data.begin()+nsplit, data.begin()+nsplit + segsize  - changing->size(), data.begin()+nsplit + diff);

                if (new_def->def())
                    for (int i = 0; i < new_def->quantity_v; i++) 
                        memcpy(&data[nsplit]+new_def->type_v->size()*i, new_def->def(), new_def->type_v->size());
                else
                    memset(&data[nsplit], 0, diff);

                last_size-=segsize;


            }
        
        }else if (diff <0) { // thus REMOVING


            // stdmove

            data.resize(footprint());

        }

    }

    void pre(std::shared_ptr<Member> changing = nullptr) override { bkp(changing); }

    void post(std::shared_ptr<Member> changing = nullptr) override { remap(changing); }

    void print() {

        for (auto x : data)
            std::cout << std::to_string(x) << " ";
        
        std::cout << "\n";

    }

    Buffer(Buffer& other) : Struct(other), data(other.data) { }

};


Register reg;

std::shared_ptr<Member::Definition> Struct::add(const TypeIndex& type,std::string name, int quantity, float from, float to, float def) {

    return add(reg.find(type), name, quantity, from, to, def);

}


int main() {

    auto buffer = std::make_shared<Buffer>("Buffy");
    
    auto foo = reg.create("foo");

    auto f1 = foo->add<uint8_t>("f1", 1,1,1);
    f1->type(reg.find<uint8_t>());
    f1->quantity(4);
    f1->range(9,9,9);

    auto bar = reg.create("bar");
    bar->add<uint8_t,2>("b1", 2,2,2);;

    auto zee = reg.create("zee");

    buffer->add(foo, "foo1", 2); 

    buffer->add(zee, "zee1", 2);

    buffer->add(bar, "bar1", 2);

    foo->add<uint8_t, 3>("f1", 4,4,4);

    zee->add<uint8_t,2>("z1", 123,123,123);

    zee->add<uint8_t>("z2");
    
    buffer->add(foo, "foo2", 1); 
    
    buffer->print();

    std::cout <<"DONE\n";

}
