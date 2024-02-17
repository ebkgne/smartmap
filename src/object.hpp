#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct Struct;
struct Buffer;
struct Entry;

struct Object { 

    Struct* s; 

    int reserved; 

    Buffer* buffer;
    size_t offset;

    char *data(size_t id = 0);
    int  eq(size_t id = 0);

    size_t size();

    std::vector<Entry*> entrys;
    
    Entry &push();

    Entry &push(void* data);

    void update(Buffer bkp);
    void update2(Buffer bkp);

    void addComp(std::vector<std::string> components);

};