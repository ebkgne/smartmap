#pragma once

#include <vector>
#include <map>
#include <string>
#include "log.hpp"



#include <boost/algorithm/string/replace.hpp>

#include "boost/date_time/posix_time/posix_time.hpp"

template <typename T>
static bool ADD_UNIQUE( std::vector<T>& list, T n) {

    for (auto x : list) if (x == n) return false;

    list.push_back(n);

    return true;

}
template <typename T>
static bool ADD_UNIQUE( std::map<int,T>& list, T n) {

    for (auto x : list) 
        if (x.second == n) 
            return false;

    list.push_back(n);

    return true;

}

template <typename T>
static bool REMOVE ( std::vector<T>& list, T n) {

    int found = -1;

    for (int i = 0; i < list.size(); i++) if (list[i] == n) { found = i; break; }

    if (found==-1) {PLOGW << "REMOVE : " << 3457 << " not found"; return false; }

    list.erase(list.begin() + found);

    return true;

}

template <typename T>
struct Pool {

    std::vector<T*> list;

    Pool(int size = 0) { list.reserve(size); }

    void add(T* x) { ADD_UNIQUE<T*>(list, x); if (list.size()>20) PLOGW << "viens on en parle"; }

    void remove(T* x) { for (int i = 0; i < list.size(); ) if (list[i] == x) list.erase(list.begin() + i); else i++;} // nbot sure


};

static std::string join(std::vector<std::string> list, std::string delimiter = "::") {

    std::string out;
    for (auto x : list) 
        out += x + delimiter;
    if (out.length()) 
        out = out.substr(0,out.length()-2);
    return out;

}

static std::vector<std::string> split(std::string s, std::string delimiter = "::") {

    std::vector<std::string> out;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        out.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    out.push_back(s);

    return out;

}


static int nextFactor2(int x, int factor = 4) { return ((int)((x-1)/(float)factor)+1)*factor*!!x; }
static std::string camel(std::string str) { str[0] = std::toupper(str[0]); return str; }
static std::string lower(std::string str) { str[0] = std::tolower(str[0]); return str; }

#include <boost/type_index.hpp>
#include <map>
using TypeIndex = boost::typeindex::type_index;
struct Type { 
    
    static inline std::map<TypeIndex, int> sizes;

    TypeIndex id; 

    std::string name() { return id.pretty_name(); }

    int size() { return sizes[id]; }

    static Type find(std::string name) { 

        for (auto x : sizes) 
            if (x.first.pretty_name() == name) 
                return {x.first};

        PLOGE << "nofind";
        
        return Type();
    
    }
    
};

template <typename T>
struct TYPE : Type { TYPE() { id = typeid(T); sizes[id] = {sizeof(T)}; } };

// struct Member;
// struct MemberQ { Member* m; uint32_t eq = 0; uint32_t q = 1; };


struct Stack {  };

static void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static bool is_num(const std::string str) {

    return str.length() && str.find_first_not_of( "0123456789" ) == std::string::npos;

}

