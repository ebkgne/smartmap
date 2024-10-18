#include <algorithm>
#include <iostream>
#include <string>
#include <memory>
#include <queue>
#include <vector>
#include <set>

#include <boost/type_index.hpp>
#include <map>


#define SHAREDSTRUCT_(__NAME__) STRUCT_(__NAME__) : std::enable_shared_from_this<__NAME__##_>

#define STRUCT_(__NAME__) struct __NAME__##_ ;\
struct __NAME__ : std::shared_ptr<__NAME__##_> {\
\
    __NAME__(std::shared_ptr<__NAME__##_> ptr = nullptr) : std::shared_ptr<__NAME__##_>(ptr) {}\
\
    template <typename... Args>\
    static __NAME__ Create(Args&&... args) { return (__NAME__)std::make_shared<__NAME__##_>(std::forward<Args>(args)...); } \
};\
struct __NAME__##_


// using TypeIndex = boost::typeindex::type_index;
struct TypeIndex : boost::typeindex::type_index {

    using boost::typeindex::type_index::type_index;

    static inline std::map<TypeIndex, int> sizes;
    static int size(TypeIndex t, int def = 0) {

        auto it = TypeIndex::sizes.find(t);
        if (it == TypeIndex::sizes.end())
            TypeIndex::sizes[t] = def;
            
        return it->second;

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

    else if (id == typeid(const char*))
        (const char*&)*ptr =  std::to_string(val).c_str();

}



template <typename T>
struct TYPE : TypeIndex { 
    
    TYPE() : TypeIndex(typeid(T)) { 
        
        // if (TypeIndex::sizes.find(typeid(T)) == TypeIndex::sizes.end())
        //     TypeIndex::sizes[typeid(T)] = sizeof(T);

        TypeIndex::size(typeid(T), sizeof(T));

        // if (std::is_arithmetic<T>::value) Data::tofloat[typeid(T)] = [&](void* ptr){ return (float)(*(T*)ptr); }; // this wont compile :'( plz hlp

    }
};

SHAREDSTRUCT_(Member) {

    Member_(const char* n = nullptr) { 
        if (n) 
            this->name = n; 
    } 

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

    enum Event { PRE};

    SHAREDSTRUCT_(Definition) {
        
        Member type_v;
        std::string label;
        int quantity_v;
    
        Definition clone_v;
        
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


        bool quantity(int q);

        void range(float from, float to, float def) {

            if (!type_v->size()) 
                return;

            rangedef.resize(type_v->size()*3);
            memset(&rangedef[0],0,rangedef.size());

            set(type_v->type_v, this->from(), from);
            set(type_v->type_v, this->def(), def);
            set(type_v->type_v, this->to(), to);
   
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


        template <typename T, typename... Args>
        Definition add(Args&&... args) { return type_v->add<T>(std::forward<Args>(args)...); } 
        template <typename... Args>
        Definition add(Args&&... args) { return type_v->add(std::forward<Args>(args)...); } 
        template <typename T, int q = 1, typename... Args>
        Definition add(Args&&... args) { return type_v->add<T,q>(std::forward<Args>(args)...); } 


        auto begin() { return type_v->members.begin(); }
        auto end() { return type_v->members.end(); }
    
    };

    std::vector<Definition> members;
    
    Definition moving_v;
    
    virtual ~Member_() {
    
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




    // Definition add(Member type, int quantity = 1, const char* name = nullptr, float from = 0, float to = 0, float def = 0) {
    //     return add(type->shared_from_this(), quantity, name, from, to, def);
    // }
    
    operator Member() {

        return shared_from_this();
    }


    // bool owned = false;
    // void* 
    // template <typename T>
    // Definition add(const char* n, bool active = true, bool visible = true) {

    //     owned = true;

    //     return add(n);
    // }
    struct Instance_ : std::enable_shared_from_this<Instance_> {

        STRUCT_(Element) { 
            
            Definition def; int eq = 0; int offset = 0; 

        };

        std::vector<Element> elements = std::vector<Element>();

        int offset = 0;
        
        void findBF(Definition def, int max_depth = 0);

        void calc_offset();

        Member owner;

    };


    struct Instance : std::shared_ptr<Instance_> {

        Instance(std::shared_ptr<Instance_> ptr) : std::shared_ptr<Instance_>(ptr) {}

        static Instance Create(Instance other = std::make_shared<Instance_>()) { 

            return std::make_shared<Instance_>(*other);
            
        }

        static Instance Create(Member m) { 

            return Create(Definition::Create(m->shared_from_this()));

        }

        static Instance Create(Definition def) { 

            return Instance::Create({Instance_::Element::Create(def)});

        }


        static Instance Create(std::vector<Instance_::Element> vec) { 

            auto n = std::make_shared<Instance_>();
            n->elements = vec;
            return n;

        } 

    };
    
    std::vector<Instance> instances;

    static std::string str(Instance stl) {

        std::string out;
        
        for (auto it = stl->elements.rbegin(); it != stl->elements.rend(); ++it) {
            
            if (it != stl->elements.rbegin()) 
                out +=  "::";

            out +=  it->get()->def->label;

            if (it->get()->def->quantity_v > 1)
                out += "[" + std::to_string(it->get()->eq) + "]";


        }

        return out;

    }

    // std::vector<Instance> getInstances2(Instance instance = Instance::Create());
    std::vector<Instance> getInstances(Instance stl, int q);


    Definition add(const char* n);

    Definition add(Member type, int quantity = 1, const char* name = nullptr, float from = 0, float to = 0, float def = 0) {

        type->addObserver(shared_from_this(),quantity);

        auto observers = getTop();

        int compoffset = footprint(); // or pos
        
        for (auto x :  observers)
            x->pre(shared_from_this(), compoffset);
        
        auto definition = Definition::Create(type, name, quantity, from, to, def);

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

using Definition = Member_::Definition;

STRUCT_(Data) : Member_ {

    Data_(const TypeIndex& type, const char* name)  { 

        type_v = type; 

        this->name = name ? name : type_v.pretty_name();
        
        size_v = TypeIndex::size(type_v); 

    }

    int size() override {
        return TypeIndex::size(type_v);
    }

};


SHAREDSTRUCT_(Register) {

    std::set<Data> datatypes;
    std::set<Member> structtypes;

    template <typename T>
    Data create(const char* name = nullptr) {

        return create(TYPE<T>(), name?name:TYPE<T>().pretty_name());
        
    }

    // static inline Register current;

    Data create(const TypeIndex& type, const char* name = nullptr) {

        auto d = Data::Create(type, name);

        // std::cout << "create " << d->quantity_v << " " << d->name << (d->quantity_v>1?"s":"")<<  " " << Type(type).name() << " - " << Type(type).size() << "\n";

        datatypes.insert(d);

        return d;
        
    }

    template <typename T>
    Data create() {

        return create<T>("");
        
    }

    Member create(const char* name) {

        auto s = Member::Create();

        s->name = name;

        // std::cout << "create " << s->quantity_v << " " << s->name << (s->quantity_v>1?"s":"")<<  ""  << "\n";

        structtypes.insert(s);

        return s;
        
    }
 
    Member find(const TypeIndex& type) {

        // type.type_info().name()
        for (const auto& x : datatypes) 
            if (x.get()->type_v == type) 
                return x->shared_from_this();

        return create(type)->shared_from_this();
        
    }
 
    template <typename T>
    Member find() {
        return find(TYPE<T>());
    }

    Member changing;

};



STRUCT_(Buffer) : Member_ {

    Buffer_(std::string name = "") /* : def(Definition::Create(shared_from_this()))  */{ 
        this->name = name;
    }

    operator Member() { return shared_from_this(); }

    std::vector<char> data = {};

    std::vector<uint32_t> changing_offsets;

    // Definition def

    void find(Member x, Instance in = Instance::Create(), int eq = 0) { 

        Instance_::Element inst;
        
        if (!in->elements.size())
            inst = Instance_::Element::Create(Definition::Create(shared_from_this())); 

        else
            inst = in->elements.front();

        if (!eq)
            inst->def->/* type_v-> */trig(Event::PRE);

        in->elements.insert(in->elements.begin(),Instance_::Element::Create());

        for (auto def : inst->def->type_v->members) {

            in->elements.front()->def = def;
            
            def->type_v->size_v = def->type_v->footprint();   
            
            // def->type_v->size_v = 0;
            // for (auto sdef : def->type_v->members) 
            //     def->type_v->size_v += sdef->footprint_all();

            auto &offsets = def->type_v->offsets[shared_from_this()];

            offsets.resize(def->quantity_v);
            
            for (int i = 0; i < def->quantity_v; i++) {
                
                offsets[i] = in->offset;

                if (def->type_v == x) 
                    changing_offsets.emplace_back(in->offset);
                        
                else{
                    in->elements.front()->eq = i;
                    find(x, Instance::Create(in), eq+i);
                }

                // std::cout <<  str(in) << " " << offset << "\n";

                in->offset += def->type_v->size_v;
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

    Buffer_(Buffer_& other) : Member_(other), data(other.data) { }
};

using Instance = Member_::Instance;
using Element = Buffer_::Instance_::Element;

static Register reg = Register::Create();

Definition Member_::add(const TypeIndex& type, int quantity, const char* name, float from, float to, float def) {

    return add(reg->find(type), quantity, name, from, to, def);
}
Definition Member_::add(const char* n) {

    auto x = reg->create(n);

    return add(*x);
    
}

void Buffer_::Instance_::findBF(Definition def, int max_depth) {

    int offset = 0;

    std::queue<std::pair<Definition, int>> q;
    for (auto m : elements.back()->def->type_v->members) 
        q.push({m, 0});

    while (!q.empty()) {

        auto current = q.front().first;
        int depth = q.front().second;
        q.pop();

        if (max_depth && depth >= max_depth) 
            return;
        

        if (current == def) {

            // auto ninst = Instance::Create(shared_from_this());

            std::cout << "FOUND<-------------"<< (current->label) << "\n";

            auto elem = Element::Create( current );
            this->offset = offset;

            elements.push_back(elem);

            return;
        }

        std::cout << ""<< (current->label) << "\n";

        for (auto m : current->type_v->members) 
            q.push({m, depth + 1});

    }


    for (auto x : elements.back()->def->type_v->members) {

        if (x == def) {

            auto ninst = Element::Create( x );

            elements.push_back(ninst);

            return;
        }

        offset += x->footprint_all();

    }

}