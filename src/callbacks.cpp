#include "callbacks.hpp"

#include "engine.hpp"
#include "member.hpp"
#include "node.hpp"
#include "file.hpp"
#include "remap.hpp"
#include "shader.hpp"
#include "ubo.hpp"
#include "model.hpp"
#include "effector.hpp"
#include "folder.hpp"
#include "drawcall.hpp"
#include "artnet.hpp"
#include "editor.hpp"
#include "ndi.hpp"
#include "atlas.hpp"
#include "json.hpp"
#include "window.hpp"
#include "framebuffer.hpp"
#include "layer.hpp"
#include "texture.hpp"
#include "gui.hpp"


#include "engine.hpp"

void Callbacks::init() {

    // ////////// xxx.HPP

    // NODE<xxx>::on(Node::CREATE, [](Node* node, xxx *x){ });
    // NODE<xxx>::on(Node::CHANGE, [](Node* node, xxx *x){ });
    // NODE<xxx>::on(Node::RUN, [](Node* node, xxx *x){ });

    // NODE<xxx>::onadd<yyy>([](Node*_this,Node*node){

    //     auto x = _this->is_a<xxx>();
    //     auto y = node->is_a<yyy>();

    //     return _this->Node::add(node);

    // });
    
    ////////// FILE.HPP

    NODE<File>::on(Node::CHANGE, [](Node* node, File *file){ 

        file->name(node->name());

        file->path_v = (file->name())+"."+file->extension;
        
        node->eachBreak<FileEffector>([node](Node* e_, FileEffector* e){ 
            // e_->name(node->name());
            e_->name_v = (node->name());

            e->Effector::name(e->file->name());

            e->load();

            for (auto refering : e_->referings) 
                refering->name(e->file->name());

            return Node::Break;
            
        });

    });

    

    NODE<File>::on(Node::CREATE, [](Node* node, File *file){ node->name(file->name()); });

    NODE<File>::on(Node::RUN, [](Node* node, File *file){

        // if (file->last_modified != file->getTimeModified()) file->reload();
        if (file->hasChanged()) {
            file->reload();
            node->update();


            }

    });

    ////////// Member.HPP

    NODE<AnyNode>::on(Node::DESTROY, [](Node* node, AnyNode *n){ 

        if (engine.gui_v) 
            engine.gui_v->deselect(node);
        

    });

    NODE<Member>::on(Node::DESTROY, [](Node* node, Member *m){ 

        // detach all remaps (couldnt it be per refering isntead  ?)
        for (auto inst : m->instances) { 

            for (auto remap : inst->remaps) {

                if (remap->src->m() == m) 
                remap->src = nullptr;
                if (remap->dst->m() == m) {
                    // remap->dst->m().r
                    remap->dst = nullptr;
                }

            }
        }
    });

    NODE<Member>::on(Node::CREATE, [](Node* node, Member *m){ 
        node->name_v = (m->ref()->name()); 
    });

    NODE<Member>::on(Node::CHANGE, [&](Node*node, Member* m){
        if (m->name() != node->name() && m == m->ref())
            m->name(node->name()); 
    });
    
    NODE<Member>::on(Node::RUN, [](Node* node, Member *m){ m->upload(); });

    NODE<UBO>::is_a<Member>();

    ////////// DRAWCALL

    
    NODE<Layer>::on(Node::CHANGE, [](Node* node, Layer *layer){ 

        layer->glsl_layers->eq(layer->vbo.layer_id).set<std::array<float,2>>({(float)layer->fb.texture.width,(float)layer->fb.texture.height});

        NODE<Member>::on_cb[Node::CHANGE](node, &layer->stat);
    
    });

    NODE<Layer>::on(Node::DESTROY, [](Node* node, Layer *layer){ 

        for (auto x : node->referings) {

            auto o = x->is_a_nowarning<Output>();

            if (o && o->fb == &layer->fb) o->fb = nullptr;
 
        } 
        
    }); 

    NODE<Model>::on(Node::CHANGE, [](Node* node, Model *model) {

        NODE<Member>::on_cb[Node::CHANGE](node, &model->stat);

    });

    NODE<Model>::on(Node::DESTROY, [](Node* node, Model *model) {

        auto modelable = node->parent()->is_a_nowarning<Modelable>();

        if (modelable) 
            modelable->removeModel(model); 

        else
            PLOGE << "error distroying " << node->nameSTL();
 
    });

    NODE<UberLayer>::allow<UberLayer::VirtualLayer>();
    NODE<Effectable>::allow<EffectorRef>();
    NODE<Modelable>::allow<Model>();
    NODE<Artnet>::allow<Universe>();
    NODE<Universe>::allow<DMXRemap>();
    
    NODE<UberLayer>::is_a<Layer>();
    NODE<Layer>::is_a<DrawCall>();
    NODE<Model>::is_a<Effectable>();
    NODE<UberLayer::VirtualLayer>::is_a<Effectable>();
    NODE<Effectable>::is_a<Member>();
    NODE<Effector>::is_a<Member>();

    NODE<Modelable>::is_a<Effectable>();

    NODE<DrawCall>::is_a<Modelable>();

    NODE<DrawCall>::on(Node::RUN, [](Node* node, DrawCall *dc) { 

        dc->draw(); 
        
    });

    NODE<Layer::Feedback>::on(Node::CHANGE, [](Node* node, Layer::Feedback *feedback) {


        feedback->texture.create( feedback->layer->fb.texture.width, feedback->layer->fb.texture.height );


    });

    NODE<Window>::on(Node::DEACT, [](Node* node, Window *win) {
        win->visibility(false);
    });
    NODE<Window>::on(Node::ACT, [](Node* node, Window *win) {
        win->visibility(true);
    });
    NODE<Debug>::on(Node::DEACT, [](Node* node, Debug *win) {
        logger.cout(Sev::warning);
    });
    NODE<Debug>::on(Node::ACT, [](Node* node, Debug *win) {
        logger.cout(Sev::verbose);
    });

    NODE<DrawCall>::on(Node::CHANGE, [](Node* node, DrawCall *dc) {

        node->parent()->each<DrawCall>([](Node*n, DrawCall* dc){ 
            
            dc->builder()->build();

            EDITOR::triglist.insert(n);


        });

        dc->vbo.upload();

    }); 

    NODE<File>::allow<Effector>();

    NODE<Effectable>::onadd<File>([](Node*_this,Node*file_){

        auto file = file_->is_a<File>();
        if (file->extension != "glsl") {

            if (_this->is_a_nowarning<Modelable>())
                return Node::no_worry;

            else
                return Node::Null;

        }


        Node* effector = nullptr;

        bool found = false;

        file_->eachBreak<FileEffector>(([&](Node* n, FileEffector* effector_){
            
            effector = n;
        
            return Node::Break;

        }));

        if (!effector)
            effector = file_->addOwnr<FileEffector>(file)->hide();

        _this->add(effector);

        return effector;

    });

    NODE<Modelable>::onadd<File>([](Node*_this,Node*node){

        auto file = node->is_a<File>();
        if (file->extension != "obj") 
            return Node::Null;

        return _this->addPtr<Model>(_this->is_a<Modelable>()->addModel( node->is_a<File>() ));

    });

    NODE<Effectable>::onadd<Effector>([](Node*_this,Node*node){ 

        auto effectable =  _this->is_a<Effectable>();
        auto effector =  node->is_a<Effector>();
        auto x = effectable->addEffector( effector );
        auto n = _this->addPtr<EffectorRef>(x);
        
        return n;
        
    });

    NODE<Effectable>::onadd<UberLayer>([](Node*_this,Node*node){ 
        
        return _this->addPtr<EffectorRef>( _this->is_a<Effectable>()->addEffector(&node->is_a<UberLayer>()->effector));
        
    });
    NODE<Effectable>::onadd<UberLayer::VirtualLayer>([](Node*_this,Node*node){ 
        
        auto out = _this->addPtr<EffectorRef>( _this->is_a<Effectable>()->addEffector(&node->is_a<UberLayer::VirtualLayer>()->effector));

        out->name(node->name());
        
        return out;
        
    });

    ////////// Effector.HPP

    NODE<FileEffector>::is_a<Effector>();
    NODE<Wrappy>::is_a<Effector>();
    NODE<Wrappy>::is_a<Effectable>();
    NODE<Layer::Feedback>::is_a<Effector>();
    NODE<UberLayer::Feedback>::is_a<Layer::Feedback>();

    NODE<Wrappy>::onadd<Effector>([](Node* _this, Node *node) {

        _this->is_a<Wrappy>()->addEffector(node->is_a<Effector>());
        
        return _this;

    });

    NODE<EffectorRef>::on(Node::CHANGE, [&](Node*node, EffectorRef* ref){ node->name_v = (ref->effector->name());  });

    NODE<FileEffector>::on(Node::CHANGE, [&](Node*node, FileEffector* effector){ effector->load();  });

    NODE<UberLayer>::on(Node::CHANGE, [&](Node*node, UberLayer* ubl){ NODE<Member>::on_cb[Node::CHANGE](node, &ubl->effector);   });
    NODE<EffectorRef>::is_a<Member>();

    NODE<Layer::Feedback>::on(Node::CHANGE, [](Node* node, Layer::Feedback *fb) {

        node->name_v = "feedback";

    });

    NODE<EffectorRef>::on(Node::DESTROY, [](Node* node, EffectorRef *reffector) {

        auto effectable = node->parent()->is_a_nowarning<Effectable>();

        if (effectable) {

            effectable->removeEffector(reffector); 

            auto lay_ = node->parent()->is_a_nowarning<Layer>();
            if (lay_ && (void*)reffector->effector == (void*)lay_->feedback_v){
                delete lay_->feedback_v;
                lay_->feedback_v = nullptr;
            }

        }else

            PLOGE << "no found";

    });

    ////////// Atlas.HPP

    NODE<Atlas>::on(Node::CREATE, [](Node* node, Atlas *atlas) { 
        node->name(atlas->path);
        });
    NODE<Atlas>::on(Node::CHANGE, [](Node* node, Atlas *atlas) { 
        atlas->fromDir(atlas->path); 
        });

    ////////// Artnet.HPP


    // NODE<Artnet>::on(Node::CHANGE, [](Node* node, Artnet *an){ 


        
    // });

    NODE<Artnet>::on(Node::RUN, [](Node* node, Artnet *an){ 
        
        an->run();

        static int size = 0;

        if (an->universes.size() != size) {

            std::vector<Universe*> missing;

            for (auto uni : an->universes) {

                bool found = false;
                for (auto n : node->childrens) {
                    auto uni_ = n->is_a_nowarning<Universe>();
                    if (uni_ == uni.second.get()) {
                        found = true;
                        break;
                    }

                }

                if (!found) 
                    missing.push_back(&an->universe(uni.first));

            }

            if (missing.size())
                for (auto x : missing)
                    node->addPtr<Universe>(x)->name("universe "+std::to_string(x->id));
            
        
            missing.clear();

            size = an->universes.size();
        }

    });
    
    // NODE<Artnet>::is_a<Member>();

    //////// Remap.HPP

    NODE<DMXRemap>::on(Node::CHANGE, [](Node* node, DMXRemap *remap) { 
        remap->extract(remap->dst->m()); 
    });

    NODE<Remap>::on(Node::RUN, [](Node* node, Remap *remap) { 

        if (remap->src && remap->dst)
            remap->update(); 

    });

    NODE<DMXRemap>::is_a<Remap>();

    NODE<Artnet>::onadd<Effectable>([](Node*_this,Node*node){ 
        
        auto &artnet = *_this->is_a<Artnet>();

        if (!artnet.universes.size()) {
            artnet.universe(0);
            _this->trig(Node::RUN);
        }
        _this->eachBreak<Universe>([&](Node* n, Universe* uni_){

            n->add(node);;

            return Node::Break;

        });

        return Node::no_worry; 

    });
    NODE<Universe>::onadd<Effectable>([](Node*_this,Node*node){ 
        
        auto &uni = *_this->is_a<Universe>();
        auto &effectable = *node->is_a<Effectable>();

        auto remap = _this->addOwnr<DMXRemap>(uni.instance, effectable.instance);

        remap->add(node); 
    
        return remap ; 

    });

    NODE<DMXRemap>::onadd<Effectable>([](Node*_this,Node*node){ 
    
        auto &remap = *_this->is_a<DMXRemap>();
        auto &effectable = *node->is_a<Effectable>();

        engine.tree->each<Effectable>([&](Node* node, Effectable* effectable){ 
            
            if (effectable->instance == remap.src) 
                node->referings.erase(_this);
            else if (effectable->instance == remap.dst) 
                node->referings.erase(_this);
            
        });

        node->referings.insert(_this);
        
        remap.dst = effectable.instance;
        
        // also add to Universe na ?

        return Node::no_worry; 
    
    });


    //////// FrameBuffer.HPP

    NODE<FrameBuffer>::on(Node::CHANGE, [](Node* node, FrameBuffer *fb) { if (fb->texture.width != fb->texture.width || fb->texture.height != fb->texture.height) { fb->create(fb->texture.width, fb->texture.height); } });


    ////////// Folder.HPP

    NODE<Folder>::on(Node::CREATE, [](Node* node, Folder *dir){ node->name(dir->path); });

    ////////// Output

    NODE<Output>::on(Node::RUN, [](Node* node, Output *output){ output->draw(); });


    NODE<NDI::Sender>::on(Node::CHANGE, [](Node* node, Output *output){

        if (!output->fb) return;

        if (output->fb->texture.width != output->width || output->fb->texture.height != output->height) { 

            PLOGE << "resize " << node->name() << " from " << output->width << "x" << output->height << " to " << output->fb->texture.width << "x" << output->fb->texture.height;

            output->size(output->fb->texture.width, output->fb->texture.height); 
        }

      });
    
    NODE<Output>::onadd<Layer>([](Node* _this, Node *node) {

        auto output = _this->is_a<Output>();

        if (output->fb) {

            Node* found = nullptr;

            for (auto r : _this->referings) {

                auto layer = r->is_a<Layer>();
                
                if (layer && &layer->fb == output->fb) {

                    found = node;

                    break;

                }

            }

            if (found)
                _this->referings.erase(found);
            else
                PLOGW << "unknown fb owner";
        
        }
       
        output->fb = &node->is_a<Layer>()->fb;
        
        _this->referings.insert(node);

        return Node::no_worry;

    });

    NODE<Window>::is_a<Output>();

    NODE<NDI::Sender>::is_a<Output>();

    ////////// NDI

    NODE<NDI::Sender>::on(Node::CREATE, [](Node* node, NDI::Sender *sender){ node->name(sender->name);  });

    ////////// JSON.HPP

    NODE<JSON>::on(Node::CREATE, [](Node* node, JSON *json){ if (json->file) node->name(json->file->name()); });

}
