#include "effector.hpp"
#include "file.hpp"
#include "builder.hpp"
#include "struct.hpp"
#include "instance.hpp"
#include "utils.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <regex>
#include <string>

// Effector  ////////////////

Effector::Effector(std::string name) : Member(name) {

    striding(true);

}


std::string Effector::body(Builder* builder, std::string prepend) {

    bool found = false;

    for (auto x : builder->definitions) 
        if (x == this) {

            found = true;
            break;
            
    }

    if (found || !size())
        return "";
    
    return "\t"+call(prepend)+"\n";


}


std::string Effector::call(std::string prepend) {

    std::string args;

    for (auto &arg : members) 
        args += prepend +"."+ arg->name()+", "; 

    if (args.length()) 
        args.resize(args.size()-2);

    return name()+"("+args+");";
    
}



// FileEffector  ////////////////


bool FileEffector::setup(Builder* builder) { 

    
    ADD_UNIQUE<::Effector*>(builder->effectors_fragment, this);

    return true; 
    
}

void FileEffector::load() {

     source_v = (&file->data[0]);

    // extract args

    std::vector<std::pair<std::string,std::string>> args;

    std::regex regex; std::smatch match;


    regex = std::regex(R"(\b(\w+)\s*(?:\(\s*\))?\s*\(\s*((?:\w+\s+\w+\s*(?:,\s*)?)*)\))");
    for (std::sregex_iterator it(source_v.begin(), source_v.end(), regex), end; it != end; ++it) {
        std::smatch match = *it;

        std::string nameStr = match[1].str();
        if (strcmp(nameStr.c_str(),file->name().c_str())) 
            continue;
        std::string argsStr = match[2].str();

        std::regex regex(R"(\b(\w+)\s+(\w+)\s*(?:,\s*)?)");
        std::sregex_iterator it_(argsStr.begin(), argsStr.end(), regex);
        std::sregex_iterator end_;
        while (it_ != end_) {
            args.push_back({(*it_)[1].str(),(*it_)[2].str()});
            ++it_;
        }

        break;
    }

    // extract ranges

    std::map<std::string, float[3]> ranges;

    // fetch defaul values a.k.a // xxx(min,max,def)
    int range_count = 0;
    regex = std::regex(R"(//\s*([a-zA-Z]+)\s*\((\s*-?\d*(\.\d*)?\s*(,\s*-?\d*(\.\d*)?\s*(,\s*-?\d*(\.\d*)?\s*)?)?)?\))");
    std::sregex_iterator next(source_v.begin(), source_v.end(), regex);
    std::sregex_iterator end;
    while (next != end) {

        std::smatch match = *next++;
        std::string range;
        int i = 0;
        std::istringstream stream(match[2].str());
        while (std::getline(stream, range, ',')) ranges[match[1]][i++] = std::stof(range);
        range_count++;

    }

    // build Member

    name( file->name() );


    bool samesame = true;
    
    if (args.size() != members.size())
        samesame = false;
    else {
    
        for (int i = 0; i < args.size(); i++) 
            if ( ! (members[i]->name() == args[i].second && members[i]->type_name() == args[i].first) )
                samesame = false;
    }

    if (!samesame) {

        clear();

        for (auto arg : args) {

            if (arg.first == "vec2") add<float, 2>(arg.second.c_str());
            else if (arg.first == "vec3") add<float, 3>(arg.second.c_str());
            else if (arg.first == "vec4") add<float, 4>(arg.second.c_str());

            else if (arg.first == "int") add<int>(arg.second.c_str());

            else add<float>(arg.second.c_str());

            if (ranges.find(arg.second) != ranges.end()) range(ranges[arg.second][0],ranges[arg.second][1],ranges[arg.second][2]);

        }

    }else {
        for (int i = 0; i < args.size(); i++) 
            if (ranges.find(args[i].second) != ranges.end()) 
                members[i]->range(ranges[args[i].second][0],ranges[args[i].second][1],ranges[args[i].second][2]);
        
    }
    // for (auto arg : args) 

    // clean source_v

    size_t pos = 0;
    while ((pos = source_v.find("//", pos)) != std::string::npos) {

        size_t end = source_v.find("\n", pos);
        if (end == std::string::npos) 
            end = source_v.length();
        source_v.erase(pos, end - pos+1);

    }
    boost::trim_right(source_v);
    boost::trim_left(source_v);
    if (source_v.find_last_of("}") == source_v.length()-1)
        source_v.resize(source_v.find_last_of("}")+1);

}

FileEffector::FileEffector(File* file, std::string name) : Effector(file->name().length()?file->name():"fileEffector"), file(file) {
    load();
}



// WrapperEffector  ////////////////

Wrappy::Wrappy(std::vector<Effector*> effectors, int count, std::string name) : Effector(name), Effectable(name) {

    for (auto x : effectors) addEffector(x);

    attrs(count);

};


// static 
std::string Wrappy::header() {

    std::string args;
    for (auto x : Effector::members) args += x->type_name() + " " + x->name() + ", ";

    std::string out = "void " + Effector::name() + " ( " + (args.length()?args.substr(0, args.length() - 2):"") + " ) ";

    out += "{\n\n"; 
    out += "\tint id = int(id_*" + std::to_string(effector_refs.size()) + ");\n\n"; 
    
    int id = 0;
    for (auto x : effector_refs) {

        std::string args;

        for (int i = 0; i < x->effector->members.size(); i++) {
            
            args +=  (i<count?Effector::members[i+1]->name():"0") + ", ";
            
        }
        
        out += "\tif (id=="+std::to_string(id++)+") { " + x.get()->effector->name() + "( " + (args.length()?args.substr(0, args.length() - 2):"") + " ); return; }\n\n";
        
    }

    out += "\n\n}\n\n";
    
    return out;

}


void Wrappy::attrs(int count) {

    this->count = count;

    Effector::clear();

    Effector::add<float>("id_").range(0,effector_refs.size()-1,0);

    for (int i = 0 ; i < count; i++)
        Effector::add<float>("param_"+std::to_string(i));


}

bool Wrappy::setup(Builder* builder) { 

    Effector::members[0]->range(0,effector_refs.size()-1,0);
    
    for (auto x : effector_refs) {
        
        ADD_UNIQUE<Effector*>(builder->effectors_fragment, x->effector);

        x->effector->setup(builder);
    
    }
    
    ADD_UNIQUE<Effector*>(builder->effectors_fragment, this);
    
    return true; }

// Ref  ////////////////
// Ref  ////////////////
// Ref  ////////////////
// Ref  ////////////////

EffectorRef::EffectorRef(std::string name, Effector* effector ) : Member(""), effector(effector) {  
    add( effector ); 
    
};


// Effectable  ////////////////
// Effectable  ////////////////
// Effectable  ////////////////
// Effectable  ////////////////

Effectable::~Effectable() {

     if (dyninst) 
        delete dyninst;

     if (statinst) 
        delete statinst;

}

Effectable::Effectable(std::string name) : Member(name) { 

 }

bool Effectable::removeEffector(EffectorRef* effector) {

     std::erase_if( effector_refs, [&](std::shared_ptr<EffectorRef> e) { 
 
        return e.get() == effector; 

    });
    
    return true;
}

EffectorRef* Effectable::addEffector(Effector* def) {

    auto effector = effector_refs.emplace_back(std::make_shared<EffectorRef>(next_name(def->name()), def)).get();

    add(effector);

    return effector;

}
