/*

   SMARTMAP ( v 1.3.1.2 beta ) 

                                */



#include "engine.hpp"
#include "gui.hpp"
#include "editor.hpp"
#include "instance.hpp"

int main() {

    // logger.cout(Sev::warning);

    engine.init();
    
    engine.open("project.json");  

    auto rect1 = engine.tree->find("layer2::rect1")->is_a<Member>();

    auto loc_test = rect1->instance;

    engine.run();

}
