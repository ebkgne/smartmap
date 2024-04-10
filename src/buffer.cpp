
#include "buffer.hpp"
#include "instance.hpp"
// #include "engine.hpp"


Instance Buffer::operator[](std::string name) { return Instance{this,0,this}[name]; }
Instance Buffer::operator[](int id) { return Instance{this,0,this}[id]; }

Buffer::Buffer(std::string name) : Struct(name) {

    data.reserve(MAX_SIZE);

    memset(data.data(),0,data.size());

}

Buffer::Buffer(const Buffer& other) :

    Struct( other ) ,

    data( other.data )

{



}

void Buffer::upload() { }

void  Buffer::pre_change() {

    if (!data.size()) return;


    bkp = copy();

    PLOGV << "bkp " << name();
    bkp->printData();

}

void  Buffer::post_change() {

    if (!bkp) return;

    PLOGV << "remap " << name();
    printData();

    remap(*bkp);
    printData();

    bkp->hard_delete();

    delete bkp;

    bkp = nullptr;

}

Buffer* Buffer::copy() { return new Buffer(*this); }

void Buffer::update() {

    Struct::update();

    if (data.size() > MAX_SIZE) { PLOGE << "data.size() > MAX_SIZE"; }

    data.resize(footprint_all());

    memset( data.data(), 0, data.size() );


}

void Buffer::printData(int max) {

    if (!max) max = data.size();

    std::string str;

    for (auto i = 0 ; i < max; i++) str += std::to_string(data[i]) + " ";

    PLOGD << str;

}

void Buffer::remap(Buffer& src_buffer, Member* src_member, Member* this_member , int src_offset, int this_offset) {

    if (!src_member) src_member = &src_buffer;

    if (!this_member) this_member = this;

    for (int i = 0 ; i < ( src_member->quantity() < this_member->quantity() ? src_member->quantity() :  this_member->quantity() ); i ++) {

        int src_offset_ = src_offset + src_member->eq(i);

        for (auto src_member_ : src_member->members) {

            Member* found = nullptr;

            auto thiseq = this_member->eq(i);

            int this_offset_ = this_offset + thiseq;

            for (auto this_member_ : this_member->members) {

                if (

                    !strcmp(src_member_->name().c_str(), this_member_->name().c_str())

                    && src_member_->type() == this_member_->type()

                ) { found = this_member_; break; }

                else this_offset_ += this_member_->size();

            }

            if (!found) { src_offset_ += src_member_->footprint(); PLOGV << "couldnt find " << src_member_->name(); continue; }

            remap(src_buffer, src_member_, found, src_offset_, this_offset+this_offset_);

            if (found->typed()) {

                PLOGV  << src_member->name() << "::" << src_member_->name() << "@" << src_offset_ << " -> "  << " " << this_member->name() << "::" << found->name()  << "@" <<  this_offset_<< " - " << src_member_->size() << " : " << *(float*)&src_buffer.data[src_offset_] << " -> " << *(float*)&data[this_offset_];

                memcpy(&data[this_offset_], &src_buffer.data[src_offset_],found->size());

            }

            src_offset_ += src_member_->footprint();

        }

    }

}
