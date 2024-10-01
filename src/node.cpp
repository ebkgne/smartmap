#include "node.hpp"
#include "utils.hpp"

#include <queue>
#include "editor.hpp"

Node::Node(void* ptr, TypeIndex type, bool owned) : void_ptr(ptr), stored_type(type), owned(owned) {

    name_v = (type_name());

    init();

}

Node::Node(std::string name, std::array<float,4> color) : color(color) {

    this->name(name);
    
    init();

}

void Node::init() {

    uid = total_uid++;

    pool.insert(this);
    PLOGV << "#" << name();

    #ifdef ROCH
    TYPE = type_name();
    #endif

    trig(Node::CREATE);

}

const std::string& Node::name() { return name_v; }

Node* Node::name(std::string value) { 
    name_v = value; 
    update();  
    return this;
}

Node::~Node() {

    auto t_childrens = childrens;
    for (auto c : t_childrens) 
        delete c;

    trig(Event::DESTROY);
      
    for (auto x : pool) for (auto r : x->referings) if (r == this) { x->referings.erase(r); break; }

    if (parent_node) parent_node->remove(this);

    pool.erase(this);

    if (owned) 
        dtortyped_cb[stored_type](void_ptr);


    PLOGV << "~" << name();

}

Node* Node::find(std::vector<std::string> names) {

    find_list.clear();
    find_pos = 0;

    BFS([&](Node* n){

        int i = names.size()-1;

        auto curr = n;

        while (curr && curr->name() == names[i]) {

            i--;
            
            curr = curr->parent();

        }

        if (i<0)
            ADD_UNIQUE<Node*>(find_list, n);

        return true;

    });

    return find_next();

}


Node* Node::find_next() {

    if (!find_list.size() || find_pos > find_list.size()-1)
        return nullptr;

    return find_list[find_pos++];

}

Node* Node::find(std::string  name) {

    if (!name.length()) 
        return nullptr;

    return find(split(name));

}

Node* Node::active(bool value) { 

    is_active = value; 
    
    if (is_active)
        trig(Node::ACT);
    else
        trig(Node::DEACT);
    
    return this; 
}

bool Node::DFS(std::function<bool(Node*)> cb, int max_depth) {

    for (auto c : this->childrens) 
        if (max_depth != 0) {

            if (!cb(this))
                return false;

            if (!c->DFS(cb, max_depth-1))
                return false;
        
        }
        
    return true;

}

void Node::BFS(std::function<bool(Node*)> cb, int max_depth) {

    std::queue<std::pair<Node*, int>> q;
    q.push({this, 0});

    while (!q.empty()) {

        auto current = q.front().first;
        int depth = q.front().second;
        q.pop();

        if (max_depth && depth > max_depth) 
            return;
        
        if (!cb(current))
            return;

        for (auto child : current->childrens) 
            q.push({child, depth + 1});

    }

}

Node* Node::top() { auto top = this; while(top->parent()) { top = top->parent(); } return top; }

void Node::addList(std::vector<Node*> *nodes) { for (auto n : *nodes) add(n); }

std::vector<TypeIndex> Node::isList(TypeIndex t) {

    std::vector<TypeIndex> types = {t};

    auto &is_lists = Node::is_lists;

    if (is_lists.find(t) != is_lists.end()) 
        for (auto t : is_lists[t])  
            for (auto x : isList(t.first) )
                types.push_back(x);

    return types;

}

Node* Node::add_typed(TypeIndex t, TypeIndex u, Node* to_add, void* ptr) {



    return to_add;

}

Node* Node::add(void* node_v)  {

    auto og = (Node*)node_v;
    Node* n = og;

    if (!n || n == this || n->parent() == this) return nullptr;

    PLOGV << type_name() << "::" << name() << " add " << n->type_name() << "::" << n->name();

    bool callback_ = false;

    auto is_u = isList(n->type());
    is_u.push_back(typeid(AnyNode));

    for (auto u_ : is_u) {
        std::string u_NAME = u_.pretty_name();
        if (onadd_cb.find(u_) != onadd_cb.end()) {
            n = onadd_cb[u_](this,n);
            callback_ = true;
        }
    }

    if (!callback_ && n && n != Node::no_worry) { 
        
        auto is_t = isList(type());

        for (auto t_ : is_t){

            bool break_ = false;
            for (auto u_ : is_u){

                std::string t_NAME = t_.pretty_name();
                std::string u_NAME = u_.pretty_name();

                if (onaddtyped_cb.find(t_) != onaddtyped_cb.end() && onaddtyped_cb.at(t_).find(u_) != onaddtyped_cb.at(t_).end()) {

                    PLOGV << t_.pretty_name() << "::" << name() << " ONADD " << u_.pretty_name() << "::" << n->name();

                    n = onaddtyped_cb[t_][u_](this,n);    
                    break_ = true;
                    callback_ = true;
                    break;
                }
            }

            if (break_)
                break;
        
        }

    }
    if (!callback_){
        
        PLOGW << "no cb for " << og->name() << " ("<<  og->type_name() << ") in " << name() << " (" << type_name() << ")"; 
        return nullptr;
    
    }

    if (n != node_v) {

        if (n != no_worry) {

            if (n ) 
                og->referings.insert(n);
            
            else 
                { PLOGW << type_name() << "::" << name() << " couldn't add " << og->type_name() << "::" << og->name(); }
        }

        return n;
    }
    
    n->parent(this);

    return n;

}

std::string Node::nameSTL(int depth){ 
    if ((depth < 0 || depth ) && parent()) { 
        std::string out = parent()->nameSTL(depth-1);
        return out + (out.length()?"::":"") + name(); 
    } 
    return ""; 
    }

Node *Node::top_parent() { 
    
    Node* top_parent = this;
    
    while (top_parent->parent()) 
        top_parent = top_parent->parent(); 

    return top_parent; 

}

Node *Node::parent() { return parent_node; }

void Node::parent(Node* parent_node) {

    if (this->parent_node == parent_node) return;

    if (this->parent_node) this->parent_node->remove(this);

    this->parent_node = parent_node;

    if (!parent_node) return;

    is_active = parent_node->is_active;

    parent_node->childrens.push_back(this);

    update();

}

Node* Node::on(Event event, std::function<void(Node*)> cb) { on_cb[event] = cb; return this; }


Node* Node::close() {

    open = false;
    return this;
}

Node* Node::hide() {

    hidden = true;

    return this;

}

Node* Node::operator[](std::string name) { return find(name); }

Node* Node::operator[](int id) { return childrens[id]; }


void Node::update() {

    PLOGV << type_name() << "::" << name();

    error = false;

    trig(Event::CHANGE);

    if (parent_node && parent_node->referings.find(this) == parent_node->referings.end()) 
        parent_node->update();

    if (referings.size() && *referings.begin())
        for (auto x : referings)
            if (x)
                x->update();

    last_change = (start_time - boost::posix_time::microsec_clock::local_time()).total_milliseconds();

}



bool Node::remove(Node *child) {


    PLOGV << type_name() << "::" << name() << " remove " << child->type_name() << "::" << child->name();

    auto it = std::find(childrens.begin(), childrens.end(), child);

    if ( std::distance(childrens.begin(), it) < 0 ) {

        PLOGI << "could not delete, didnt found";
        return false;

    }

    childrens.erase(it);

    if (child->type() != typeid(None) && !child->hidden) 
        update();

    return true;

}

void Node::run() { 
    
    if (!is_active) 
        return; 

    trig(Node::RUN); 

    for (auto x : childrens) 
        x->run(); 

}

uint32_t Node::index() {

    auto it = std::find(parent_node->childrens.begin(), parent_node->childrens.end(), this);

    return std::distance(parent_node->childrens.begin(), it);

}


static std::string event_name(Node::Event event){
    switch (event) {
        case Node::CREATE: return "CREATE";
        case Node::CHANGE: return "CHANGE";
        case Node::RUN: return "RUN";
        case Node::DESTROY: return "DESTROY";
        case Node::ACT: return "ACTIVATE";
        case Node::DEACT: return "DEACTIVATE";
        case Node::EDITOR: return "EDITOR";
    }

    return "UNKNOWN";
}

bool Node::trig_typed(Node::Event e, TypeIndex t, void* out, bool break_) {

    bool found = false;

    std::string t_NAME = t.pretty_name();
    if (ontyped_cb[e].find(t) != ontyped_cb[e].end()) {
        
        if (e != RUN && e != EDITOR)
            {PLOGV << event_name(e)  << " " << t.pretty_name() << "::" << name(); }

        (*(std::function<void(Node*,void*)>*) ontyped_cb[e].at(t))(this,out); 

        if (break_)
            return true;
        
        found = true; 
        
    }

    if (is_lists.find(t) != is_lists.end()) 
        for (auto is : is_lists[t]) {

            std::string is_NAME = is.first.pretty_name();
            
            if (trig_typed(e, is.first, is.second(out), break_)) {
                if (break_)
                    return true;
                found = true;
            }
            

        }



    return found;

}

bool Node::trig(Event e, bool break_)  { 
    
    bool found = false;

    if (on_cb.find(e) != on_cb.end()) {

        on_cb[e](this);
        found = true;
    }

    if (!found || !break_)
        found = trig_typed(e, type(), void_ptr, break_);     

    if (!found)
        found = trig_typed(e, typeid(AnyNode), void_ptr);

    return found;
    
}


void* Node::is_a_untyped(TypeIndex t, TypeIndex u, void* out) {

#ifdef ROCH
    std::string t_NAME = t.pretty_name();
    std::string u_NAME = u.pretty_name();
#endif

    if (t == u) 
        return out;

    if (is_lists.find(t) != is_lists.end()) 

        for (auto t_ : is_lists[t]) {

            if (t_.first == u) 
                return t_.second(out);

            else
                if (auto result = is_a_untyped(t_.first,u, t_.second(out))) 
                    return result;

        }

    return nullptr;

}