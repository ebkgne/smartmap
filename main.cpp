/*

    SMARTMAP (beta)

                        */

#include "engine.hpp"
#include "log.hpp"
#include "boilerplate.hpp"

#include "drawcall.hpp"


int main() {

    logger.cout(true);

    engine.init();

    auto &dc = *engine.stack->addOwnr<DrawCall>();

    auto m_node = dc.add((*engine.models)[0]);

    // m_node->is_a<Model>()->quantity(2);
    // m_node->add((*engine.effectors)[3]);
    // m_node->add((*engine.effectors)[0]);
    // m_node->add((*engine.effectors)[1]);

    engine.gui->editors.push_back(new EditorWidget());

    // // engine.
    // for (int i = 0 ; i < 100; i++) {

    // auto &bkp = engine.dynamic_ubo.copy();
    // delete &bkp;
    // }
    // PLOGD
    engine.run();

}


// redo copy ?

// finish remap
// then
// on Buffer:: add (member) { set defeualt } // do range // do default // on push
