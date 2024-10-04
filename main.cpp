
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

struct Member : std::enable_shared_from_this<Member>  {

    std::string name;

    std::set<std::shared_ptr<Member>> observers;

    TypeIndex type_v = typeid(Member);

    std::shared_ptr<Member> clone_v = nullptr;

    struct Definition {
        
        std::shared_ptr<Member> type_v;
        std::string name;
        int quantity_v;
    
        Definition( std::shared_ptr<Member> type, const char* name = nullptr, int q = 1, float from = 0, float to = 0, float def = 0) : name(name?name:""), quantity_v(q) {

            this->type(type);

            if (from || to || def)
                range(from, to, def); 
        }

        std::vector<char> rangedef; 
        std::vector<char> temprangedef; 

        uint32_t footprint_all() { return type_v->size() *  quantity_v ; }

        void type(std::shared_ptr<Member> type) {

            type_v = type;
            

        }

        void quantity(int value) {

            quantity_v = value;

        }

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
  
    virtual void pre(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp, std::shared_ptr<Member> bkp) {}
    virtual void post(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp, std::shared_ptr<Member> bkp) {}

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

    Data(const TypeIndex& type)  { 
        type_v = type; 
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

        // if (std::is_arithmetic<T>::value) Data::tofloat[typeid(T)] = [&](void* ptr){ return (float)(*(T*)ptr); }; // this wont compile

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

        changing->moving_v =  definition;

        for (auto x :  observers)
            x->pre(changing,definition,shared_from_this());

        members.emplace_back(definition);

        for (auto x :   observers)
            x->post(changing,definition,shared_from_this());

        return definition;

    }

    std::shared_ptr<Definition> add(const TypeIndex& type, int quantity, const char* name, float from, float to, float def);

    template <typename T, int q = 1>
    std::shared_ptr<Definition> add(const char* name, float from = 0, float to = 0, float def = 0) {

        return add(TYPE<T>(), q, name, from, to, def);
        
    }
    
    bool remove(const std::string& memberName) {

        auto it = std::find_if(members.begin(), members.end(),
            [&memberName](const std::shared_ptr<Definition>& def) { return def->name == memberName; });

        if (it == members.end()) 
            return false;
        
        auto observers = observe();

        std::shared_ptr<Member> changing =  std::make_shared<Member>(*this);

        changing->clone_v =  shared_from_this();

        for (auto x :  observers)
            x->pre(changing, *it, shared_from_this());

        members.erase(it);
        
        for (auto x :   observers)
            x->post(changing, *it, shared_from_this());
        
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

        auto d = std::make_shared<Data>(type);

        d->name = (name?name:d->type_v.name());

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

    void bkp(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp, std::shared_ptr<Member> bkp) {

        changing_offsets.clear();

        std::cout << "is this each frame for sure ? could keep track of instances" << std::endl;

        find(bkp, shared_from_this());

        std::reverse(changing_offsets.begin(), changing_offsets.end());

        if (!changing_offsets.size())
            changing_offsets = {0};

    }

    void remap(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp, std::shared_ptr<Member> bkp) {

        int diff =  bkp->size() - changing->size();

        if (diff > 0) { // thus ADDING

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

                if (comp->def())
                    for (int i = 0; i < comp->quantity_v; i++) 
                        memcpy(&data[nsplit]+comp->type_v->size()*i, comp->def(), comp->type_v->size());
                else
                    memset(&data[nsplit], 0, diff);

                last_size-=segsize;


            }

            return;
        
        }else if (diff <0) { // thus REMOVING


            // stdmove

            std::cout << "CACAPITALE\n";

        }
        
        data.resize(footprint());

    }

    void pre(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp, std::shared_ptr<Member> bkp) override { this->bkp(changing,comp,bkp); }

    void post(std::shared_ptr<Member> changing, std::shared_ptr<Definition> comp, std::shared_ptr<Member> bkp) override { remap(changing,comp,bkp); }

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

int main() {
    
    // remove

    // instance ( aka Member::tracking_counter ?)

    auto buffer = std::make_shared<Buffer>("Buffy");
    
    buffer->print();
    auto foo = reg.create("foo");

    buffer->print();
    auto f1 = foo->add<uint8_t>("f1", 1,1,1);
    buffer->print();
    f1->type(reg.find<uint8_t>());
    f1->quantity(4);
    f1->range(9,9,9);

    buffer->print();
    auto bar = reg.create("bar");
    buffer->print();
    bar->add<uint8_t,2>("b1", 2,2,2);;

    buffer->print();
    auto zee = reg.create("zee");

    buffer->print();
    buffer->add(foo, 2, "foo1"); 

    buffer->print();
    buffer->add(zee, 2, "zee1");

    buffer->print();
    buffer->add(bar, 2, "bar1");

    buffer->print();
    foo->add<uint8_t, 3>("f1", 4,4,4);

    buffer->print();
    zee->add<uint8_t,2>("z1", 123,123,123);

    buffer->print();
    zee->add<uint8_t>("z2");
    
    buffer->print();
    buffer->add(foo); 
    
    buffer->print();
    buffer->remove("zee1");
    buffer->print();

    std::cout <<"DONE\n";

}
