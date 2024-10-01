#include "member.hpp"
#include "struct.hpp"
#include "instance.hpp"


#include "buffer.hpp"
#include "utils.hpp"
#include <cstdint>
#include <cstring>
#include <cstdint>

Member::~Member() {

    // remove from other structs
    for (auto s : structs) 
        if (std::find(s->members.begin(), s->members.end(), this) != s->members.end()) {
            s->remove(*this);  
}
    // remove from Member::structs
    if (!isData()) 
        structs.erase(this);

    deleteData(false);

    if (copy_v) return;

    // std::stringstream ss; ss << std::hex << std::showbase << reinterpret_cast<void*>(this);
    PLOGV << "~" << ref()->name()/* << " ( &" + ss.str() + " )"*/;
    
}

Member::Member(std::string name) {

    this->name(name);

    PLOGV << "#" << this->name();// << " ( &" + ss.str() + " )" ;

    type<Member>();

    structs.insert(this);

}

Member::Member(std::string name, Type type) {

    this->name(name);
    this->type(type);

}

Member::Member(Member& other) :
 
    striding_v(other.striding_v) ,
    quantity_v( other.quantity_v ) ,
    name_v(other.name_v) , // could no need by no use in bkpremap ? 
    members(other.members),
    type_v(other.type_v), 
    size_v(other.size_v), 
    rangedef(other.rangedef) ,
    buffering_v(other.buffering_v),
    buffer_v(other.buffer_v),
#ifdef ROCH
    _TYPE_(other._TYPE_),
#endif
    copy_v(&other)

{

    for (auto &m : members) 
        m = new Member(*m);

}


void Member::name(std::string name_v) { 
    this->name_v = next_name(name_v); 
        
}

std::string Member::_name() { 

    std::string name = this->name();

    std::replace( name.begin(), name.end(), ' ', '_');

    return name;
}

std::string Member::name() { 
    if (name_v.length()) 
        return name_v; 
    if (ref() != this)
        return ref()->name_v;
    return "NO_NAME" ; 
}

std::string Member::next_name( std::string name ) {

    if (!name.length())
        return name;

    int count = 0;

    for (auto x : members) {

        auto name_ = x->name();

        size_t pos = name_.find("_");

        int i = 1;

        if (pos != std::string::npos) {

            name_ = x->name().substr(0, pos);

            auto numstr = x->name().substr(pos+1);

            if (numstr.length() && std::all_of(numstr.begin(), numstr.end(), ::isdigit))

                i = std::stoi(numstr);

        }

        if (!strcmp(name.c_str(), name_.c_str())) {

            if ( i > count) count =  i;

            else count++;

        }

    }

    if (count) 
        name += "_" + std::to_string(count) ;

    return name;

}

std::string Member::type_name() {

    if (type().id == typeid(Struct)) return camel(name());

    if (type().id == typeid(float)) {
        
        if (quantity_v == 1) return "float";
        else if (quantity_v == 2) return "vec2";
        else if (quantity_v == 3) return "vec3";
        else if (quantity_v == 4) return "vec4";
        
    }

    if (type().id == typeid(int)) return "int";

    if (type().id == typeid(uint32_t)) return "uint";

    if (type().id == typeid(char)) return "char";

    std::string type_ = type().name();

    if (isData()) return "datatype " + type_;

    return "unknown " + type_;

}

void Member::bkp() {

    if (!buffering_v) return;

    bkp_v = new Member(*this);

    PLOGV << "bkp " << name() << " [" << footprint_all() << "]" ;

}



void Member::type(Type value) {

    type_v = value;
#ifdef ROCH
    _TYPE_ = value.name();
#endif

    if (!isData()) {
        
        size_v = 0;

        return;
    
    }

    size_v = value.size();

    rangedef.resize(size_v*3);

    memset(rangedef.data(),0,rangedef.size());

    float to = 1;

    if (value.id == typeid(uint32_t)) to = 65535;
    
    else if (value.id == typeid(int)) to = 0;

    else if (value.id == typeid(uint8_t)) to = 255;

    else if (value.id == typeid(char)) to = -1;
    
    range(0,to,0);


}


Member& Member::buffering(bool value) { 
    
    buffering_v  = value;

    if (buffering_v) {

        buffer_v.reserve(MAX_SIZE);

        memset(buffer_v.data(),0,MAX_SIZE);


    }else{

        buffer_v.reserve(0);

    }

    update_pv();

    return *this;

}


Member& Member::striding(bool is_striding){ 

    tops = getTop();

    for (auto t : tops)  
        t->stl.front().m->bkp();
    
    this->striding_v = is_striding; 

    update_pv();

    tops.clear();

    return *this;
    
}

Member& Member::quantity(uint32_t quantity_v) {

    if (quantity_v == this->quantity_v) return *this;

    PLOGV << name() << " = " << quantity_v;

    tops = getTop();

    for (auto t : tops)  
        t->stl.front().m->bkp();

    int old = this->quantity_v;

    this->quantity_v = quantity_v;

    old = quantity_v-old;

    MemberQ  mq;
    
    if (old>0)
        mq = adding.emplace_back(MemberQ{this, (uint32_t)old, quantity_v-old});
    update_pv();
    if (old>0)
        REMOVE<MemberQ>(adding,mq);

    tops.clear();

    return *this;

}


void Member::add(Member* m) {

    PLOGV << name() << "[" << footprint_all() << "] add " << m->name() << "[" << m->footprint_all() << "]";

    if (m == m->ref()) {

        auto new_mame = next_name(m->name());

        while (m->name_v != new_mame) {

            PLOGW << m->name() << " already exist in " << name() << " !";

            new_mame = next_name(new_mame);

            m->name( new_mame );

        }

    }

    tops = getTop();

    for (auto t : tops)  
        t->stl.front().m->bkp();

    members.push_back(m);

    size_v += members.back()->footprint_all();

    auto mq = adding.emplace_back(MemberQ{m,0,m->quantity()});
    update_pv();
    REMOVE<MemberQ>(adding,mq);

    tops.clear();

}



Member& Member::range(float from, float to, float def) {

    auto m = this;
    if (members.size()) 
        m = members.back();

    for (int i = 0; i < quantity_v; i++) {

        if (m->type().id == typeid(float)) {

            (float&)*m->from() = from;
            (float&)*m->to() = to;
            (float&)*m->def() = def;

        }else if (m->type().id == typeid(uint32_t)) {

            (uint32_t&)*m->from() =  from;
            (uint32_t&)*m->to() =  to;
            (uint32_t&)*m->def() =  def;

        }else if (m->type().id == typeid(int)) {

            (int&)*m->from() =  from;
            (int&)*m->to() =  to;
            (int&)*m->def() =  (int)def;

        }else if (m->type().id == typeid(char)) {

            (char&)*m->from() =  from;
            (char&)*m->to() =  to;
            (char&)*m->def() =  def;

        }else if (m->type().id == typeid(uint8_t)) {

            (uint8_t&)*m->from() =  from;
            (uint8_t&)*m->to() =  to;
            (uint8_t&)*m->def() =  def;

        }

    }

    return *this;

}

bool Member::remove(Member& m) {

    removing.insert(&m);

    PLOGV << name() << "[" << footprint_all() << "] remove " << m.ref()->name() << "[" << m.footprint_all() << "]";

    // if (!buffering_v) // proven to be problematic ex buff.remove(xxx)
    tops = getTop();

    for (auto t : tops)  
        t->stl.front().m->bkp();

    removeHard(m);

    update_pv();

    removing.erase(&m);

    tops.clear();

    return true;

}

bool Member::removeHard(Member& m) {

    auto it = std::find( members.begin(), members.end(), &m );

    if (it == members.end()) { PLOGV << "no find "<< m.name(); return false; }

    size_v -= members.back()->footprint_all();

    members.erase(it);

    return true;
}


bool Member::clear() {

    PLOGV << name() ;

    tops = getTop();

    for (auto t : tops)  
        t->stl.front().m->bkp();

    size_v = 0;

    deleteData(false);

    members.clear();

    update_pv();

    tops.clear();

    return true;

}

void Member::simpleupdate() { 




}

void remap(Instance this_, Instance bkp) {

    auto m = bkp.stl.back().m;
    
    // each member
    for (auto bkp_m : m->members) {

        // look for in this_
        Member* found = nullptr;
        for (auto _this_m : this_.stl.back().m->members) 
            if (bkp_m->copy_v == _this_m) {
                found = _this_m;
                break;
            }

        if (!found) 
            continue;

        auto bkp_ = Instance(bkp)[bkp_m];
        auto this__ = Instance(this_)[found];

        int q = found->quantity()<bkp_m->quantity()?found->quantity():bkp_m->quantity();
        
        // each eq
        for (int i = 0; i < q; i++) 
            remap(this__.eq(i), bkp_.eq(i));

    }

    if (m->isData())
        memcpy(this_.data(), bkp.data(),m->size());

    // PLOGV << bkp.stl_name() << " @ " << bkp.offset << " -> " << this_.offset;
    
}


void Member::calc_size() { 

    size_v = 0;

    if (isData())
        size_v = type_v.size();
    else
        for (auto &m : members) 
            size_v += m->footprint_all();

}

void Member::update_pv() { 

    calc_size();

    for (auto a : structs) 
        for (auto &m : a->members) 
            if (m == this) 
                a->update_pv();

    PLOGV << name() << "[" << footprint_all() << "]";

    //// BUFFER PART

    if (bkp_v) {

        if (footprint_all() > MAX_SIZE) { PLOGE << footprint_all() << " > MAX_SIZE"; }

        buffer_v.resize(footprint_all());

        memset( buffer_v.data(), 0, buffer_v.size() );

    }
        for (auto t : tops)  {

            t->post_change(adding);

            remap(*t,Instance(*t->stl.front().m->bkp_v));

            auto bkp_v = t->stl.front().m->bkp_v;

            if (bkp_v) {

                bkp_v->deleteData();

                delete bkp_v;

                bkp_v = nullptr;

            }

            t->stl.front().m->upload();    
            
        }
    

 }

 void Member::upload() { 

    
}

char* Member::data() { 

    if (buffering_v) {

        char* ptr = &buffer_v[0];  
        return ptr;
        
    }

    return nullptr; 

}

char* Member::from() { return rangedef.size()?rangedef.data():nullptr; }
char* Member::to() { return rangedef.size()?rangedef.data()+(size_v):nullptr; }
char* Member::def() { if (rangedef.size()) return rangedef.data()+(size_v*2); return nullptr; }

bool Member::buffering() { return buffering_v; }

bool Member::striding() { return striding_v; }

bool Member::isData() { return type_v.id != typeid(Member); }

Member* Member::ref() { return (!name_v.length())&&members.size()?members[0]:this; }

uint32_t Member::size() {  return size_v; }

uint32_t Member::footprint() { if (striding()) return nextFactor2(size(),16); return size(); }

uint32_t Member::stride() { return (footprint()-size()); }

uint32_t Member::quantity() { return quantity_v; }

uint32_t Member::eq(int i) { return i * footprint(); }

uint32_t Member::footprint_all() { return eq( quantity_v ); }

Type Member::type() { return type_v; }

void Member::deleteData(bool recurse){

    auto t_members = members;

    for (auto m : t_members) {

        if (recurse) 
            m->deleteData();
        
        if (m->isData()) {

            removeHard(*m);
            
            delete m;
        }
            
    }

}

std::set<std::shared_ptr<Instance>> Member::getTop(bool z) {

    std::set<std::shared_ptr<Instance>> owners, out;

    for (auto x : structs) 
        if (std::find( x->members.begin(), x->members.end(), this ) != x->members.end())
            owners.insert( std::make_shared<Instance>(*x) );



    for (auto owner : owners) {

        auto tops = owner.get()->stl.front().m->getTop(true);

        bool found = false;

        for (auto top : tops) {

            for (auto x : out) {

                if (x->stl.front().m == top->stl.front().m) {
                    found = true;
                    break;
                }

            }
                
            if (found) break;

        }

        if (!found) out.insert(tops.begin(), tops.end());

    }

    if (!out.size() && buffering()) 
        return {std::make_shared<Instance>(*this)};

    return out;

}