
#include <algorithm>
#include <iostream>
#include <string>
#include <type_traits>
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


struct Member_;
using Member = std::shared_ptr<Member_>;


struct Buffer_;
using Buffer = std::shared_ptr<Buffer_>;

struct Member_ : std::enable_shared_from_this<Member_>  {

    std::string name;

    std::map<Member,int> observers;
    void addObserver(Member m, int q = 1) {

        if (observers.find(m) == observers.end())
            observers[m] = q;
        else
            observers[m]+=q;
    }
    void removeObserver(Member m, int q = 1) {

        if (observers.find(m) == observers.end())
            std::cout << "\nerore\n\n";
        
        observers[m]-=q;
        
        if (observers[m] < 1)   
            observers.erase(m);

    }


    std::map<Member,std::vector<int>> offsets;

    int size_v = 0;

    TypeIndex type_v = typeid(Member_);

    Member clone_v = nullptr;

    enum Event { PRE};
    struct Definition_;
    using Definition = std::shared_ptr<Definition_>;

    struct Instance_;
    using Instance = std::shared_ptr<Instance_>;
    struct Instance_ { 
        
        Definition def; int eq = 0; int offset = 0; 

        
    };

    using STL = std::shared_ptr<std::vector<Instance>>;
    struct Definition_ : std::enable_shared_from_this<Definition_>{
        
        Member type_v;
        std::string label;
        int quantity_v;
    
        Definition clone_v = nullptr;
        
        Definition_( Member type, const char* name = nullptr, int q = 1, float from = 0, float to = 0, float def = 0) : label(name?name:type->name), quantity_v(q) {

            this->type(type);

            if (from || to || def)
                range(from, to, def); 
        }

        std::vector<char> rangedef; 
        std::vector<char> temprangedef; 

        uint32_t footprint_all() { return type_v->footprint() *  quantity_v ; }

        void trig(Event e){}    
        
        void type(Member type) {

            type_v = type;
            

        }

        void track(Buffer buffer, std::vector<STL> stls = {std::make_shared<std::vector<Instance>>()});

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
            
            static uint64_t ui64 = 0xffffffffffffffffu;
            static int64_t i64   = 0xffffffffffffffff;
            static double f32    = 0xfffffffffffff;

            if (type_v->type_v == typeid(float)) return (char*)&f32;
            if (type_v->type_v == typeid(double)) return (char*)&f32;
            if (type_v->type_v == typeid(uint64_t)) return (char*)&ui64;
            if (type_v->type_v == typeid(uint32_t)) return (char*)&ui64;            
            if (type_v->type_v == typeid(uint16_t)) return (char*)&ui64;
            if (type_v->type_v == typeid(uint8_t)) return (char*)&ui64;
            if (type_v->type_v == typeid(int64_t)) return (char*)&i64;
            if (type_v->type_v == typeid(int32_t)) return (char*)&i64;            
            if (type_v->type_v == typeid(int16_t)) return (char*)&i64;
            if (type_v->type_v == typeid(int8_t)) return (char*)&i64;

            return nullptr;

        }

        char* def() { 

            if (rangedef.size()) return rangedef.data()+(type_v->size()*2);
            
            if (type_v->type_v == typeid(Member_)) {

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

    std::vector<Definition> members;
    
    Definition moving_v = nullptr;

    std::set<STL> instances;
    
    virtual ~Member_() {
    
        if (!clone_v)
            for (auto def : members) 
                def->type_v->observers.erase(shared_from_this());

    }

    virtual int size() {
        // return size_v;
        int size = 0;
        for (auto def : members) 
            size+= def->footprint_all();
        return size;
    }

        
    uint32_t stride() { return 0 ; }
    uint32_t footprint() { return size() + stride() ; }
  
    virtual void pre(Member changing, int compoffset) {}
    virtual void post(int diff, int compsize, char* def, int q) {}

    std::set<Member> getTop() {

        std::set<Member> out;

        if (!observers.size()) 
            out.insert(shared_from_this());
        
        for (auto observer : observers)     
            for (auto x :   observer.first->getTop())
                out.insert(x);

        return out;

    }

    std::set<STL> getSTLS(STL  stl = std::make_shared<std::vector<Instance>>());
};

struct Data_;
using Data = std::shared_ptr<Data_>;
struct Data_ : Member_ {
    
    static inline std::map<TypeIndex, int> type_sizes;

    Data_(const TypeIndex& type, const char* name)  { 

        type_v = type; 

        this->name = name ? name : type_v.pretty_name();
        
        size_v = type_sizes[type_v]; 

    }

    int size() override {
        return type_sizes[type_v];
    }

};
   std::set<Member_::STL> Member_::getSTLS(Member_::STL  stl) {

        std::set<Member_::STL> out;

        // if (!observers.size()) 
        //     out.insert(shared_from_this());
        
        if (observers.size()) {
            
            if(dynamic_cast<Data_*>(this))
                return out;

            for (int i = 1; i < observers.size(); i++) 

                out.insert(std::make_shared<std::vector<Instance>>());

            stl->push_back( std::make_shared<Instance_>(std::make_shared<Definition_>(shared_from_this())));
        }
        for (auto observer : observers){     
            for (auto x :   observer.first->getSTLS(stl))
                out.insert(x);
        }

        return out;

    }
template <typename T>
struct TYPE : TypeIndex { 
    
    TYPE() : TypeIndex(typeid(T)) { 
        
        if (Data_::type_sizes.find(typeid(T)) == Data_::type_sizes.end())
            Data_::type_sizes[typeid(T)] = sizeof(T);

        // if (std::is_arithmetic<T>::value) Data::tofloat[typeid(T)] = [&](void* ptr){ return (float)(*(T*)ptr); }; // this wont compile :'( plz hlp

    }
};

struct Struct_;
using Struct = std::shared_ptr<Struct_>;
struct Struct_ : Member_ {

    Definition add(Member type, int quantity = 1, const char* name = nullptr, float from = 0, float to = 0, float def = 0) {

        type->addObserver(shared_from_this(),quantity);

        auto observers = getTop();

        int compoffset = footprint(); // or pos
        
        for (auto x :  observers)
            x->pre(shared_from_this(), compoffset);
        
        auto definition = std::make_shared<Definition_>(type, name, quantity, from, to, def);

        int compsize = definition->type_v->footprint();

        auto defval = definition->def();

        // std::string cout = this->name+ "[" +std::to_string(footprint()) + "] add  " + definition->label + "(" + definition->type_v->name + ":" + std::to_string(compsize);
        // if (definition->quantity_v > 1)
        //     cout += "*" + std::to_string(definition->quantity_v)  + ":" + std::to_string(definition->footprint_all());
        // cout += ")";
        // std::cout << cout << std::endl;

        members.emplace_back(definition);

        for (auto x :   observers)
            x->post(definition->quantity_v,compsize, defval, 0);

        return definition;

    }

    bool remove(Definition definition) {

        return quantity(definition, 0);

    }

    bool quantity(Definition definition, int q) { // work only adding not removing for now // though this will be in post() normaly by ifelse (diff>0) 

        auto observers = getTop(); // necessary ?

        int compoffset = 0; bool found = false;
        for (auto m : members) {
            if (m == definition){
                found = true; break;
            }
            compoffset += m->footprint_all();
        }
        if (!found)
            compoffset = 0;

        int diff = q-definition->quantity_v;

        if (diff<0) {
            compoffset+= definition->footprint_all();

            definition->type_v->addObserver(shared_from_this(),diff);
        }else
            definition->type_v->removeObserver(shared_from_this(),std::abs(diff));
        for (auto x :  observers)
            x->pre(shared_from_this(), compoffset);

        int compsize = definition->type_v->footprint(); // or before definition->quantity_v = q; ?

        auto defval = definition->def();

        // std::string cout = this->name+ " resize " + definition->label + " from " + std::to_string(definition->quantity_v) + " to " + std::to_string(q) ;
        // cout += "";
        // std::cout << cout << std::endl;

        definition->quantity_v = q;

        for (auto x :   observers)
            x->post(diff,compsize, defval, 0);

        return true;

    }

    Definition add(const TypeIndex& type, int quantity, const char* name, float from, float to, float def);

    template <typename T, int q = 1>
    Definition add(const char* name, float from = 0, float to = 0, float def = 0) {

        return add(TYPE<T>(), q, name, from, to, def);
        
    }
    
  

};

struct Register {

    std::set<Data> datatypes;
    std::set<Struct> structtypes;

    template <typename T>
    Data create(const char* name) {

        return create(TYPE<T>(), name);
        
    }

    Data create(const TypeIndex& type, const char* name = nullptr) {

        auto d = std::make_shared<Data_>(type, name);

        // std::cout << "create " << d->quantity_v << " " << d->name << (d->quantity_v>1?"s":"")<<  " " << Type(type).name() << " - " << Type(type).size() << "\n";

        datatypes.insert(d);

        return d;
        
    }

    template <typename T>
    Data create() {

        return create<T>("");
        
    }

    Struct create(const char* name) {

        auto s = std::make_shared<Struct_>();

        s->name = name;

        // std::cout << "create " << s->quantity_v << " " << s->name << (s->quantity_v>1?"s":"")<<  ""  << "\n";

        structtypes.insert(s);

        return s;
        
    }
 
    Member find(const TypeIndex& type) {

        // type.type_info().name()
        for (const auto& x : datatypes) 
            if (x.get()->type_v == type) 
                return x;

        return create(type);
        
    }
 
    template <typename T>
    Member find() {
        return find(TYPE<T>());
    }

    Member changing = nullptr;

};

static std::string str(Member_::STL stl) {

    std::string out;
    
    for (auto it = stl->rbegin(); it != stl->rend(); ++it) {
        
        if (it != stl->rbegin()) 
            out +=  "::";

        out +=  it->get()->def->label;

        if (it->get()->def->quantity_v > 1)
            out += "[" + std::to_string(it->get()->eq) + "]";


    }

    return out;

}

struct Buffer_ : Struct_ {

    Buffer_(std::string name = "") { 
        this->name = name;
    }

    std::vector<char> data = {};

    std::vector<uint32_t> changing_offsets;
    
    std::vector<STL> instances;

    void find(Member x, STL in = std::make_shared<std::vector<Instance>>(), int offset = 0, int eq = 0) { 

        Instance inst;
        
        if (!in->size())
            inst = std::make_shared<Instance_>(std::make_shared<Member_::Definition_>(shared_from_this())); 

        else
            inst = in->front();

        if (!eq)
            inst->def->/* type_v-> */trig(Event::PRE);

        in->insert(in->begin(),std::make_shared<Instance_>());

        for (auto def : inst->def->type_v->members) {

            in->front()->def = def;
            
            def->type_v->size_v = def->type_v->footprint();   
            
            // def->type_v->size_v = 0;
            // for (auto sdef : def->type_v->members) 
            //     def->type_v->size_v += sdef->footprint_all();

            auto &offsets = def->type_v->offsets[shared_from_this()];

            offsets.resize(def->quantity_v);
            
            for (int i = 0; i < def->quantity_v; i++) {
                
                offsets[i] = offset;

                if (def->type_v == x) 
                    changing_offsets.emplace_back(offset);
                        
                else{
                    in->front()->eq = i;
                    find(x, std::make_shared<std::vector<Instance>>(*in), offset, eq+i);
                }

                std::cout <<  str(in) << " " << offset << "\n";

                offset += def->type_v->size_v;
            }

        }

    }

    void pre(Member changing, int compoffset) override { 
        
        // find offset of each new instance 
        
        changing_offsets.clear();

        if (changing.get() == this)
            changing_offsets = {footprint()};
        else            
            find(changing); 
        
        for (int i = 0; i < changing_offsets.size(); i++) 
            changing_offsets[i] += compoffset;
    }

    void post(int diff, int compsize, char* def, int q) override { 

        int old_cursor, new_cursor, diffsize = compsize * diff;

        if (diff > 0) { // aka ADDING

            old_cursor = data.size();
            new_cursor = footprint();

            std::reverse(changing_offsets.begin(), changing_offsets.end());
 
            data.resize(footprint());

            for (auto offset : changing_offsets) {

                int segment = std::max(0,(int)old_cursor - (int)offset);

                if (segment) {

                    auto old_end = old_cursor;

                    old_cursor -= segment;
                    new_cursor -= segment;

                    std::move(data.begin()+old_cursor, data.begin()+old_cursor+segment, data.begin()+new_cursor); 
                }

                new_cursor -=  diffsize;
            
                if (def) 
                    for (int i = 0; i < diff; i++) 
                        memcpy(&data[new_cursor]+i*compsize, def, compsize);

                else
                    memset(&data[new_cursor], 55, diffsize);
            }
        
        }else if (diff < 0) { // aka REMOVING

            if (changing_offsets.size() == 1 && changing_offsets[0] == data.size())
                changing_offsets = {0};

            else
                changing_offsets.emplace_back(data.size());

            old_cursor = changing_offsets.front();
            new_cursor = old_cursor+diffsize;
            changing_offsets.erase(changing_offsets.begin());
            changing_offsets.emplace_back(old_cursor);

            for (auto offset : changing_offsets) {

                int segment = offset - old_cursor + diffsize ;

                std::move(data.begin()+old_cursor, data.begin()+old_cursor+segment, data.begin()+new_cursor); 

                old_cursor = offset;
                new_cursor += segment;
            }

            data.resize(footprint());

        }

        size_v = 0;
        for (auto x : members) 
            size_v += x->footprint_all();
     }

    void print() {

        for (auto x : data)
            std::cout << std::to_string(x) << " ";
        
        std::cout << "\n";
    }

    Buffer_(Buffer_& other) : Struct_(other), data(other.data) { }
};


void Member_::Definition_::track(Buffer buffer, std::vector<STL> stls) {

    // find each way to owners ( supposely one only )

    ;

    std::vector<STL> sstls;

    for (auto x : type_v->observers) {

        sstls.push_back(std::make_shared<std::vector<Instance>>(*stls.back())); 

    }
 
    // for (auto x : buffer->instances) 

}

Register reg;

Member_::Definition Struct_::add(const TypeIndex& type, int quantity, const char* name, float from, float to, float def) {

    return add(reg.find(type), quantity, name, from, to, def);
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
    test->add(cc, 2);

    auto buffer = std::make_shared<Buffer_>("Buffy");
    buffer->add(test,2);

    buffer->print();

    std::cout << "############\n";

    trick->add<uint8_t,2>("T", 4,4,4);
    // buffer->print();
    cc->quantity(cval,3);
    cc->quantity(cval,0);


    auto dd = reg.create("dd");
    auto trickdd = dd->add<uint8_t,3>("D", 5,5,5 );
    trick->add(dd,2);

    trickdd->track(buffer);

    for (auto x : trickdd->type_v->getSTLS()) 

        std::cout << str(x) << " - " << x->size() << "\n";

    buffer->print();

    // buffer->track({});

    std::cout << buffer->size_v << " " << buffer->data.size() << " DONE\n";

}

// 1 1 21 122 122 122 122 23 4 4 5 5 5 5 5 5 21 122 122 122 122 23 4 4 5 5 5 5 5 5 2 2 1 1 21 122 122 122 122 23 4 4 5 5 5 5 5 5 21 122 122 122 122 23 4 4 5 5 5 5 5 5 2 2 