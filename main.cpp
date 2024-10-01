/*

   SMARTMAP ( v 1.3.1.2 beta ) 

                                */


#include "engine.hpp"
#include "layer.hpp"
#include "node.hpp"
#include "editor.hpp"




int main() {


    logger.cout(Sev::warning);

    engine.init();

    engine.open("project.json");  
    engine.window.size(100, 1);
    engine.gui(true);

    auto main_ = engine.tree->find("main")->allow<AnyNode>();

    auto argb_ = engine.tree->find("argb");
    auto rectangle_ = engine.tree->find("rectangle");
    auto quad_ = engine.tree->find("quad");


    main_->add(quad_);
    auto lay_ = main_->childrens.back();
    auto v1_ = lay_->childrens.back();
    v1_->add(argb_);
    v1_->add(rectangle_);
    auto v2_ = lay_->add(quad_);
    v2_->add(argb_);
    v2_->add(rectangle_);
    engine.gui_v->editors[0].get()->selected = lay_;
    engine.gui_v->editors[0].get()->locked = true;



    auto ubl_ = engine.tree->find("main")->addOwnr<UberLayer>();
    auto ubl = ubl_->is_a<UberLayer>();

    auto &vl1 = ubl->addLayer(100, 1);
    vl1.quantity(5);
    auto vl1_ = ubl_->addPtr<UberLayer::VirtualLayer>(&vl1);
    vl1_->active(true);
    vl1_->add(argb_);

    auto &vl2 = ubl->addLayer(100, 1);
    vl2.quantity(2);
    auto vl2_ = ubl_->addPtr<UberLayer::VirtualLayer>(&vl2);
    vl2_->active(true);
    vl2_->add(argb_);

    ubl->calc_matrice();

    engine.gui_v->editors[1].get()->selected = ubl_;
    engine.gui_v->editors[1].get()->locked = true;
    engine.run();

}
