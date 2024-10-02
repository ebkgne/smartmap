/*

   SMARTMAP ( v 1.3.1.2 beta ) 

                                */


#include "engine.hpp"
#include "imgui.h"
#include "instance.hpp"
#include "layer.hpp"
#include "node.hpp"
#include "editor.hpp"
#include "gui.hpp"
#include "widgets.hpp"

int main() {

    logger.cout(Sev::verbose);

    Member buff("buff");

    // buff.buffering(true);

    auto mint = new Member("int1", TYPE<int>());
    // buff.members.push_back(mint);
    // buff.size_v += mint->footprint_all();
    // buff.calc_size();

    // for (auto a : Member::structs) 
    //     for (auto &m : a->members) 
    //         if (m == &buff) 
    //             a->update_pv();

    // buff.buffer_v.resize(buff.footprint_all());
    // memset( buff.buffer_v.data(), 0, buff.buffer_v.size() );

    mint->quantity(1500);

    buff.add(mint);
    delete mint;
    // buff.add<int,10000>("val1");

}
