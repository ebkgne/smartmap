
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <set>

#include <boost/type_index.hpp>
#include <map>

using TypeIndex = boost::typeindex::type_index;

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

    else if (id == typeid(const char*))
        (const char*&)*ptr =  std::to_string(val).c_str();

}

struct Definition;

struct Member : std::enable_shared_from_this<Member>  {

    std::string name;

    std::set<std::shared_ptr<Member>> observers;

    TypeIndex type_v = typeid(Member);

    std::shared_ptr<Member> clone_v = nullptr;

    struct Definition : std::enable_shared_from_this<Definition>{
        
        std::shared_ptr<Member> type_v;
        std::string label;
        int quantity_v;
    
        Definition( std::shared_ptr<Member> type, const char* name = nullptr, int q = 1, float from = 0, float to = 0, float def = 0) : label(name?name:type->name), quantity_v(q) {

            this->type(type);

            if (from || to || def)
                range(from, to, def); 
        }

        std::vector<char> rangedef; 
        std::vector<char> temprangedef; 

        uint32_t footprint_all() { return type_v->footprint() *  quantity_v ; }

        void type(std::shared_ptr<Member> type) {

            type_v = type;
            

        }

        bool quantity(int q);

        void range(float from, float to, float def) {

            if (!type_v->size()) 
                return;

            rangedef.resize(type_v->size()*3);
            memset(&rangedef[0],0,rangedef.size());

            set(type_v->type_v, this->from(), from);
            set(type_v->type_v, this->to(), to);
            set(type_v->type_v, this->def(), def);
   
        }

        char* from() { return rangedef.size()?rangedef.data():nullptr; }

        char* to() { 
            
            if (rangedef.size())
                return rangedef.data()+(type_v->size());
            
            static uint64_t ui64 = 9999999999999999999u;
            static uint32_t ui32 = 65535u;
            static uint16_t ui16 = 16654u;
            static uint16_t ui8 = 255u;

            static int64_t i64 = 999999999999999999;
            static int32_t i32 = 65535;
            static int16_t i16 = 16654;
            static int16_t i8 = 255;

            static float f16 = 99999999999999999999999999999999999999.9f;
            static double f32 = 99999999999999999999999999999999999999.9f;

            if (type_v->type_v == typeid(float)) return (char*)&f16;
            if (type_v->type_v == typeid(double)) return (char*)&f32;
            if (type_v->type_v == typeid(uint64_t)) return (char*)&ui64;
            if (type_v->type_v == typeid(uint32_t)) return (char*)&ui32;            
            if (type_v->type_v == typeid(uint16_t)) return (char*)&ui16;
            if (type_v->type_v == typeid(uint8_t)) return (char*)&ui8;
            if (type_v->type_v == typeid(int64_t)) return (char*)&i64;
            if (type_v->type_v == typeid(int32_t)) return (char*)&i32;            
            if (type_v->type_v == typeid(int16_t)) return (char*)&i16;
            if (type_v->type_v == typeid(int8_t)) return (char*)&i8;

            return nullptr;

        }

        char* def() { 

            if (rangedef.size()) return rangedef.data()+(type_v->size()*2);
            
            if (type_v->type_v == typeid(Member)) {

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

    std::vector<std::shared_ptr<Definition>> members;
    
    std::shared_ptr<Definition> moving_v = nullptr;

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
  
    virtual void pre(std::shared_ptr<Member> changing) {}
    virtual void post(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp) {}

    std::set<std::shared_ptr<Member>> observe() {

        std::set<std::shared_ptr<Member>> out;

        if (!observers.size()) 
            out.insert(shared_from_this());
        
        for (auto observer : observers)     
            for (auto x :   observer->observe())
                out.insert(x);

        return out;

    }
};

struct Data : Member {
    
    static inline std::map<TypeIndex, int> type_sizes;

    Data(const TypeIndex& type, const char* name)  { 

        type_v = type; 

        this->name = name ? name : type_v.pretty_name();

    }

    int size() override {
        return type_sizes[type_v];
    }

};

template <typename T>
struct TYPE : TypeIndex { 
    
    TYPE() : TypeIndex(typeid(T)) { 
        
        if (Data::type_sizes.find(typeid(T)) == Data::type_sizes.end())
            Data::type_sizes[typeid(T)] = sizeof(T);

        // if (std::is_arithmetic<T>::value) Data::tofloat[typeid(T)] = [&](void* ptr){ return (float)(*(T*)ptr); }; // this wont compile :'( plz hlp

    }
};

struct Struct : Member {

    std::shared_ptr<Definition> add(std::shared_ptr<Member> type, int quantity = 1, const char* name = nullptr, float from = 0, float to = 0, float def = 0) {

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
            x->post(changing,definition);

        return definition;

    }

    std::shared_ptr<Definition> add(const TypeIndex& type, int quantity, const char* name, float from, float to, float def);

    template <typename T, int q = 1>
    std::shared_ptr<Definition> add(const char* name, float from = 0, float to = 0, float def = 0) {

        return add(TYPE<T>(), q, name, from, to, def);
        
    }
    
    bool quantity(std::shared_ptr<Definition> definition, int q) {

        auto observers = observe();

        std::shared_ptr<Member> changing =  std::make_shared<Member>(*this);
        changing->clone_v =  shared_from_this();
        for (auto &m : changing->members) 
            if (m.get() == definition.get()){
                auto original = m;
                m = std::make_shared<Member::Definition>(*m.get());  // is this really deleted at the end of this method ?
                m.get()->type_v->clone_v = original->type_v->clone_v; // does this help ?
            }

        for (auto x :  observers)
            x->pre(changing);
        
        definition->quantity_v = q;
        
        for (auto x :   observers)
            x->post(changing, definition);

        return true;

    }

    bool remove(const std::string& memberName) {

        auto it = std::find_if(members.begin(), members.end(),
            [&memberName](const std::shared_ptr<Definition>& def) { return def->label == memberName; });

        if (it == members.end()) 
            return false;
        
        auto observers = observe();

        std::shared_ptr<Member> changing =  std::make_shared<Member>(*this);
        changing->clone_v =  shared_from_this();

        for (auto x :  observers)
            x->pre(changing);

        members.erase(it);
        
        for (auto x :   observers)
            x->post(changing, *it);
        
        it->get()->type_v->observers.erase(shared_from_this());

        return true;
    }

};

struct Register {

    std::set<std::shared_ptr<Data>> datatypes;
    std::set<std::shared_ptr<Struct>> structtypes;

    template <typename T>
    std::shared_ptr<Data> create(const char* name) {

        return create(TYPE<T>(), name);
        
    }

    std::shared_ptr<Data> create(const TypeIndex& type, const char* name = nullptr) {

        auto d = std::make_shared<Data>(type, name);

        // std::cout << "create " << d->quantity_v << " " << d->name << (d->quantity_v>1?"s":"")<<  " " << Type(type).name() << " - " << Type(type).size() << "\n";

        datatypes.insert(d);

        return d;
        
    }

    template <typename T>
    std::shared_ptr<Data> create() {

        return create<T>("");
        
    }

    std::shared_ptr<Struct> create(const char* name) {

        auto s = std::make_shared<Struct>();

        s->name = name;

        // std::cout << "create " << s->quantity_v << " " << s->name << (s->quantity_v>1?"s":"")<<  ""  << "\n";

        structtypes.insert(s);

        return s;
        
    }


    std::shared_ptr<Member> find(const char* name) {

        for (const auto& type : structtypes)
            if (type->name == name)
                return type;
 
        return nullptr;

    }
 
    std::shared_ptr<Member> find(const TypeIndex& type) {

        // type.type_info().name()
        for (const auto& x : datatypes) 
            if (x.get()->type_v == type) 
                return x;

        return create(type);
        
    }
 
    template <typename T>
    std::shared_ptr<Member> find() {
        return find(TYPE<T>());
    }

    std::shared_ptr<Member> changing = nullptr;

};


struct Buffer : Struct {

    Buffer(std::string name = "") { 
        this->name = name;
    }

    std::vector<char> data = {};

    std::vector<int> changing_offsets;

    void find(std::shared_ptr<Member> x, std::shared_ptr<Member> curr , int offset= 0) {

        for (auto def : curr->members) {

            if (def->type_v == x) {
            
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

        std::cout << "is this each frame for sure ? could keep track of instances" << std::endl;

        find(changing->clone_v, shared_from_this());

        std::reverse(changing_offsets.begin(), changing_offsets.end());

        if (!changing_offsets.size())
            changing_offsets = {0};

    }

    void remap(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp) {

        int diff =  changing->clone_v->size() - changing->size();

        if (diff > 0) { // thus ADDING

            int last_cursor = data.size();

            data.resize(footprint());
            
            int new_cursor = data.size();
            
            for (auto offset : changing_offsets) {
            
                std::cout <<  "---------"  <<  std::endl;

                int segsize = last_cursor - offset;

                new_cursor -= diff + segsize;

                std::cout 
                
                <<  " move1 "  <<  offset
                
                <<  " to "  <<  last_cursor 

                <<  " @ "  <<  new_cursor  
                
                 << std::endl;

                std::move(data.begin()+offset, data.begin()+last_cursor, data.begin()+new_cursor);
                
                print();

                uint32_t x3 = changing->size(); 
                uint32_t x4 = new_cursor; 
                uint32_t x5 = x3 + x4; 

                std::cout 
                <<  " x3: "  <<  x3
                <<  " x4: "  <<  x4
                <<  " x5: "  <<  x5
                << std::endl;

                int sub_cursor = new_cursor + changing->size(); // <----- missing comdiff ? 

                std::cout 
                
                <<  " move2 "  <<  sub_cursor
                
                <<  " to "  <<  sub_cursor + segsize  - changing->size() 

                <<  " @ "  <<   sub_cursor + diff   // devrait etre : mo2 11 to 12 @ 12 // comp diff is 1
                
                 << std::endl; // il sait pas localiser le debut du deuxieme split ---^

                std::move(data.begin()+sub_cursor, data.begin()+sub_cursor + segsize  - changing->size(), data.begin()+sub_cursor + diff);
                
                print();

                std::cout 
                
                <<  " default "  <<  sub_cursor
                
                 << std::endl;

                if (comp->def())
                    for (int i = 0; i < comp->quantity_v; i++) 
                        memcpy(&data[sub_cursor]+comp->type_v->size()*i, comp->def(), comp->type_v->size());
                else
                    memset(&data[sub_cursor], 0, diff);
                
                print();

                last_cursor-=segsize;


            }

            return;
        
        }else if (diff <0) { // thus REMOVING


            // stdmove

            std::cout << "CACAPITALE\n";

        }
        
        data.resize(footprint());

    }

    void pre(std::shared_ptr<Member> changing) override { bkp(changing); }

    void post(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp) override { remap(changing,comp); }

    void print() {

        for (auto x : data)
            std::cout << std::to_string(x) << " ";
        
        std::cout << "\n";

    }

    Buffer(Buffer& other) : Struct(other), data(other.data) { }

};

Register reg;

std::shared_ptr<Member::Definition> Struct::add(const TypeIndex& type, int quantity, const char* name, float from, float to, float def) {

    return add(reg.find(type), quantity, name, from, to, def);

}

bool Member::Definition::quantity(int q) {

    std::shared_ptr<Struct> found = nullptr;
    
    for (auto x : reg.structtypes) {

        auto it = std::find(x->members.begin(), x->members.end(), shared_from_this()) ;

        if (it != x->members.end()) {

            found = x;
            
            break;

        }

    }


        
    auto observers = found->observe();

    std::shared_ptr<Member> changing =  std::make_shared<Member>(*found);

    // auto bkpdefinition = std::make_shared<Definition>(*definition);

    changing->clone_v =  found->shared_from_this();

    for (auto x :  observers)
        x->pre(changing);
    
    quantity_v = q;
    
    for (auto x :   observers)
        x->post(changing, shared_from_this());

    return true;

}

int main() {
    
    // quantity need to trig a remap 

    // remove

    // instance ( aka Member::tracking_counter ?)

    auto buffer = std::make_shared<Buffer>("Buffy");
    
    auto foo = reg.create("foo");
    foo->add<uint8_t>("val",1,1,1);
    auto bar = reg.create("bar");
    bar->add<uint8_t>("val",2,2,2);
    auto zee = reg.create("zee");
    zee->add<uint8_t>("val",3,3,3);

    auto test = reg.create("test");
    test->add(foo, 2);
    auto barval = test->add(bar, 2);
    test->add(zee, 2);
    
    buffer->add(test, 2);

    // barval->quantity(1);

    test->quantity(barval, 3); // go ds remap

    buffer->print();

    std::cout <<"DONE\n";

}
