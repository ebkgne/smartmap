#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include "utils.hpp"

struct Instance;

struct Member {

    static inline std::set<Member*> structs;

    Member(std::string name);

    Member(std::string name, Type type);

    Member(Member& other) ;

    virtual ~Member();

    void name(std::string value) ;

    std::string next_name(std::string name);

    std::string name();
    std::string _name();

    Type type();

    std::string type_name() ;

    void add(Member* m);

    template <typename T>
    Member& add(std::string name) {

        auto n = new Member(name, TYPE<T>()); 
        
        add(n); 
        
        return *this; 
        
    }

    template <typename T,int q>
    Member& add(std::string name, T from = 0, T to = 0, T def = 0) {        
        
        add<T>(name);

        if (q != quantity_v) 
            members.back()->quantity(q);

        if (to)
            range(from, to, def);

        return *this; 
        
    }

    Member& range(float from, float to, float def) ;

    bool remove(Member& s) ;

    bool removeHard(Member& s) ;

    bool clear();

    Member& quantity(uint32_t value) ;

    uint32_t quantity();

    uint32_t size();

    uint32_t footprint();

    uint32_t footprint_all();

    uint32_t stride();
    
    Member& striding(bool value);

    bool striding();

    bool buffering();

    Member& buffering(bool value);

    uint32_t eq(int i) ;

    std::set<std::shared_ptr<Instance>> getTop(bool z = false);
    
    void simpleupdate();
    
    void calc_size();

    virtual void update_pv();

    virtual void upload();
 
    bool isData();
    
    Member* ref();
    
    void deleteData(bool recurse = false);
    
    template <typename T>
    void type() { type(TYPE<T>()); }

    void type(Type type);

    char* from();
    char* to();
    char* def();

    char* data();

    void bkp();

    uint32_t factor = 1;

// private:

    std::string name_v;

#ifdef ROCH
    std::string _TYPE_;
#endif

    uint32_t size_v = 0;

    uint32_t quantity_v = 1;

public:

    std::vector<Member*> members;
    
    std::set<Instance*> instances;

// private:
    
    std::vector<char> buffer_v;

    Member* bkp_v = nullptr;
public:

    Member* copy_v = nullptr;
    
    
    bool buffering_v = false;
    
    bool force_ref = false;
    
    static inline int MAX_SIZE = 10000;
// private:

    bool striding_v = false;

    std::vector<char> rangedef; 

    Type type_v = TYPE<Member>();

    static inline std::set<Member*> removing;
    
    std::vector<MemberQ> adding;
    std::set<std::shared_ptr<Instance>> tops;


};

inline bool operator==(const MemberQ& this_, const MemberQ& other){ return (this_.m == other.m && this_.eq == other.eq); }
