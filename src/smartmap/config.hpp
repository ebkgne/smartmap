#pragma oncreate

#include "../dmx.hpp"
#include "../json.hpp"
#include "output.hpp"

namespace SmartMap {

struct Config {
    
    File file;

    rapidjson::Document json;

    DMX::Fixture basic_fixture;

    std::string ip;
    
    unsigned int subnet;

    std::map<std::string, Output*> outputs;

    Config();
    
    void import(std::string filepath); 

};

};
