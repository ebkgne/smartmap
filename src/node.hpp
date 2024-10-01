#pragma once

#include <cstring>
#include <vector>
#include <set>
#include <map>
#include <functional>

#include "utils.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"


struct None {};
struct Node;
using Flag = Node*;

struct Node {

    enum Event {

        CREATE,
        CHANGE,
        RUN,
        DESTROY,
        ACT,
        DEACT,
        EDITOR

    };

    std::string name_v;

#ifdef ROCH
    std::string TYPE = "Node";
#endif

    std::array<float,4> color = {1,1,1,1};

    std::vector<Node*> childrens;

    bool locked = false, loaded = false, hidden = false, open = true, is_active = false, error = false;

    static inline std::set<Node*> pool;


    static inline Flag Null = nullptr;
    static inline int no_worrint = 0;
    static inline Flag no_worry = (Flag)&no_worrint;
    static inline int breakint = 0;
    static inline Flag Break = (Flag)&breakint;

    Node(std::string name = "node", std::array<float,4> color = {1,1,1,1});
    Node(void* ptr, TypeIndex type, bool owned);

    Node(Node* other) : Node(other->void_ptr, other->stored_type, other->owned) {  }

    virtual ~Node();

    void init();

    const std::string& name();

    Node* name(std::string value);

    virtual void update();

    Node* add(void *node);

    void parent(Node* parent_node);

    Node* parent();
    Node* top_parent();

    Node* find(std::string  name);
    Node* find_next();
    Node* find_nowarning(std::string  name);
    Node* find(std::vector<std::string> names);
    
    bool DFS(std::function<bool(Node*)> cb, int max_depth = 0);
    void BFS(std::function<bool(Node*)> cb, int max_depth = 0);

    uint32_t index();

    void addList(std::vector<Node*> *nodes);

    Node* hide();

    Node* close();

    Node* active(bool value);

    bool remove(Node *child);

    TypeIndex type() { return stored_type; }

    virtual std::string type_name() { return type().pretty_name();  }

    Node* on(Event event, std::function<void(Node*)> cb = nullptr);

    static inline uint32_t total_uid = 0;
    uint32_t uid = 0;

    bool is_typed = false;

    Node* top();

    void run();

    std::string nameSTL(int depth = -1);

    std::set<Node*> referings;

    bool trig(Event event);

    template <typename T>
    void onadd(std::function<Flag(Node*,Node*)> cb) { onadd_cb[typeid(T)] = cb; }

    std::map<Event,std::function<void(Node*)>> on_cb;
    static inline std::map<Event, std::map<TypeIndex, void*>> ontyped_cb;

    std::map<TypeIndex, std::function<Flag(Node*,Node*)>> onadd_cb;
    static inline std::map<TypeIndex, std::map<TypeIndex, std::function<Flag(Node*,Node*)>>> onaddtyped_cb;

    static inline std::map<TypeIndex, std::function<void(void*)>> ctortyped_cb, dtortyped_cb;
    
    static inline std::map<TypeIndex, std::map<TypeIndex, std::function<void*(void*)>>> is_lists;

    static std::vector<TypeIndex> isList(TypeIndex t);

    Node* operator[](int id);
    Node* operator[](std::string name);

    TypeIndex stored_type = typeid(Node);

    void* void_ptr = nullptr;


    bool owned;

    template <typename U>
    Node* addPtr(void* ptr) { 
        
        auto out = new Node(ptr,typeid(U), false);

        return add(out); 
        
    }
    template <typename U>
    static void init() { 

        if (Node::dtortyped_cb.find(typeid(U)) == Node::dtortyped_cb.end()) 
            Node::dtortyped_cb.emplace(typeid(U),[&](void* t){ 

                delete (U*)t; 

            });

        // if (Node::ctortyped_cb.find(typeid(U)) == Node::ctortyped_cb.end()) 
        //     Node::ctortyped_cb.emplace(typeid(U),[&](void* t){ 

        //         return new U(); 

        //     });


    }

    template <typename U, typename... Args>
    Node* addOwnr(Args&&... args) {

        Node::init<U>();

        Node* n;
        
        auto ptr = new U(std::forward<Args>(args)...);

        if (typeid(U) == typeid(Node)) 
            n = (Node*)ptr;

        else 
            n = new Node(ptr, typeid(U), true);
        
        if (!add(n)) { 
            
            delete n; // will delete ptr
            return nullptr; 
        
        } 

        return n;

    }
    

    template <typename U>
    void eachBreak(std::function<bool(Node*, U*)> cb, int max_depth = 0) { 
        
        BFS([&](Node* n) { 
            
            U* isa = n->is_a_nowarning<U>(); 

            if (isa) 
                return (bool)cb(n,isa); 

            return true;

        }, max_depth); 
    }

    template <typename U>
    void each(std::function<void(Node*, U*)> cb, int max_depth = 0) { 

        eachBreak<U>([&](Node* n, U* isa) { cb(n,isa); return true; }, max_depth);

    }
    
    template <typename U>
    int count() { 

        int total = 0;

        eachBreak<U>([&](Node* n, U* isa) { total++; return true; });

        return total;

    }

    static inline std::function<Flag(Node*,Node*)> any_cb = [](Node*_this,Node*node){ return node; };

    template <typename U>
    Node* allow() { 
        
        Node::onadd_cb[typeid(U)] = any_cb;

        return this;
    }

    template <typename U>
    U* is_a() {

        auto x = is_a_nowarning<U>();

        if (!x) 
            { PLOGW << "\"" << name() << "\" is " << type_name() << " not a " << boost::typeindex::type_id_with_cvr<U>().pretty_name() << " ! "; }

        return x;

    }

    void* is_a_untyped(TypeIndex t, TypeIndex u, void* out);

    template <typename U>
    U* is_a_nowarning() { return (U*)is_a_untyped(type(),typeid(U),void_ptr); }

    double last_change = 0;
    static inline boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

private:

    Node* add_typed(TypeIndex t, TypeIndex u, Node* to_add, void* out);
    bool trig_typed(Event event, TypeIndex type, void* out);
    
    Node* parent_node = nullptr;

    bool has_changed = false;

    int find_pos = 0;
    std::vector<Node*> find_list;

};


template <typename T>

struct NODE {
    
    static inline std::map<Node::Event, std::function<void(Node*,T*)>> on_cb;

    template <typename U>
    static inline void  is_a() { 

        Node::is_lists[typeid(T)].emplace(typeid(U),[&](void* t){ return (U*)(T*)t; });
        
    }

    template <typename U>
    static void allow() { 

        NODE<T>::onadd<U>(Node::any_cb); 

    }

    template <typename U>
    static void onadd(std::function<Flag(Node*,Node*)> cb) { Node::onaddtyped_cb[typeid(T)][typeid(U)] = cb;  }

    static void on(Node::Event event, std::function<void(Node*,T*)> cb) { on_cb[event] = cb; Node::ontyped_cb[event][typeid(T)] = &on_cb[event]; }

};


struct AnyNode {

};

