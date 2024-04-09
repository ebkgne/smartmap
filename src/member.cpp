#include "member.hpp"
#include "struct.hpp"


#include "log.hpp"
#include "buffer.hpp"
#include "struct.hpp"
#include <cstdint>
#include <unordered_set>
#include <typeindex>
#include <cstdint>


Member::~Member() {

    PLOGV << "~" << name();

}


Member::Member(const Member& other)
 :

    is_striding(other.is_striding) ,
    quantity_v( other.quantity_v ) ,
    name_v(other.name_v) ,
    members(other.members) ,
    size_v( other.size_v )

 { for (auto &m : members) m = m->copy(); }


std::set<Member*> Member::getTop(bool z) {

    std::set<Member*> owners, out;

    for (auto x : structs) if (std::find( x->members.begin(), x->members.end(), this ) != x->members.end()) owners.insert( x );

    if (!owners.size()) { if (!z) return {}; out.insert(this); return out; }

    for (auto owner : owners) {

        auto top = owner->getTop(true);

        out.insert(top.begin(), top.end());

    }

    return out;

}

Member::Member(std::string name_v) {

    name(name_v);

    if (!name_v.length()) quantity_v = 0; // ???????

    PLOGV << "#" << name();

}

void Member::update() { for (auto a : structs) for (auto &m : a->members) if (m == this) a->update(); }

void Member::name(std::string name_v) { this->name_v = name_v; }

std::string Member::name() { if (name_v.length()) return name_v; return "parentName" ; }

uint32_t Member::size() { return 0; }

uint32_t Member::footprint() { if (striding()) return nextFactor2(size(),16);  return size(); }

uint32_t Member::stride() { return (footprint()-size()); }

void Member::quantity(uint32_t quantity_v) { this->quantity_v = quantity_v; update(); }

uint32_t Member::quantity() { return quantity_v; }

uint32_t Member::eq(int i) { return i * footprint(); }

uint32_t Member::footprint_all() { return eq( quantity_v ); }

void Member::striding(bool is_striding){ this->is_striding = is_striding; update(); }

bool Member::striding() { return is_striding; }

std::type_index Member::type() { return typeid( *this ); }

std::string Member::type_name() {

    if (type() == typeid(glm::vec2)) return "vec2";

    if (type() == typeid(glm::vec3)) return "vec3";

    if (type() == typeid(glm::vec4)) return "vec4";

    if (type() == typeid(float)) return "float";

    if (type() == typeid(int)) return "int";

    if (type() == typeid(uint32_t)) return "uint";

        // return "Sampler2D";

    return "unknown";

}

uint8_t Member::count() {

    // if (type() == typeid(Struct)) { int x = 0; for (auto m : members) { x += m.count()}; ); return x; }

    if (type() == typeid(glm::vec2)) return 2;

    if (type() == typeid(glm::vec3)) return 3;

    if (type() == typeid(glm::vec4)) return 4;

    return 1;

}

Member* Member::copy() { return new Member(*this); }

bool Member::typed() { return false; }

std::string Member::print(int recurse) {

    std::string str = type_name() + " " + camel(name())  + ";";

    return str;

}
