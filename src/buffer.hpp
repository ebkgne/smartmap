#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstddef>
#include <map>

#include "struct.hpp"




struct Instance;

struct Buffer : Struct {

    std::vector<char> data;
    
    Buffer(std::string name = "buffer_definition") : Struct(name) {

        data.reserve(10000);

        memset(data.data(),0,data.size());

    }

    virtual void upload() { }

    void set(Member& m, void* data) { }

    void update() override {
            
        Struct::update(); 

        data.resize( footprint_all() ); 

    }

    Instance operator[](const char* name);

    Member* copy(Member* x) override { 
        
        if (!x) {x = new Buffer();} 

        Struct::copy(x);

        ((Buffer*)x)->data = data;

        return x; 
        
    }
    
    Buffer& copy() { 
        Buffer* x= (Buffer*)Buffer::copy(nullptr); 
        Buffer& y= *x; 
        Buffer*z= &y; 
        return y;
    }

    void remap(Buffer& src_buffer, Member* src_member = nullptr, Member* this_member = nullptr, int src_offset = 0, int this_offset = 0) {

        if (!src_member) src_member = &src_buffer;

        if (!this_member) this_member = this;

        for (int i = 0 ; i < ( src_member->quantity < this_member->quantity ? src_member->quantity :  this_member->quantity ); i ++) {
        
            int src_offset_ = src_offset + src_member->eq(i);
            
            for (auto src_member_ : src_member->members) {
                
                Member* found = nullptr;

                int this_offset_ = this_offset + this_member->eq(i);

                for (auto this_member_ : this_member->members) {
                
                    if (

                        !strcmp(src_member_->name().c_str(), this_member_->name().c_str())

                        && src_member_->type() == this_member_->type()
                        
                    ) { found = this_member_; break; }
                
                    else this_offset_ += this_member_->size();
                
                }
                
                if (!found) { PLOGV << "couldnt find " << src_member_->name(); continue; }

                remap(src_buffer, src_member_, found, src_offset_, this_offset+this_offset_); 

                if (found->typed()) {
                    
                    PLOGV  << src_offset_ << " -> "  << this_offset_ << " " << src_member->name() << "::" << src_member_->name() << "(" << src_member_->size() << ")";

                    memcpy(&data[src_offset_], &src_buffer.data[this_offset_],found->size());
                                            
                }
            
                src_offset_ += src_member_->footprint();

            }
            
        }

    }

};

