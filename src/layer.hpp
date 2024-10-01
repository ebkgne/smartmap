#pragma once

#include <memory>

#include "effector.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"
#include "shader.hpp"
#include "drawcall.hpp"

struct Layer : DrawCall {

    Member stat,models_stat;

    struct Feedback : Effector {

        Layer* layer;

        Texture texture;

        Feedback(Layer* layer);

        bool setup(::Builder* builder) override;

        std::string header() override;

    };

    FrameBuffer fb;

    Feedback* feedback_v = nullptr;
    
    virtual Feedback* feedback();

    Layer(uint16_t width = 0, uint16_t height = 0, std::string name = "layer");

    void draw() override;

    void resize(uint32_t width, uint32_t height);

};

struct UberLayer;

struct UberEffector : Effector {

    UberLayer *ubl_v;

    uint32_t range[2] = {0,0};

    UberEffector(UberLayer* ubl = nullptr);
    
    void ubl(UberLayer* ubl);

    std::string  header() override;

    bool setup(Builder* builder) override;
    
    std::string body(Builder* builder, std::string prepend = "") override;

};

struct UberLayer : Layer {

    UberLayer() ;

    Member uberlayer_m;

    int w ; int h;

    void calc_matrice() ;
    
    ::Builder* builder() override;

    struct Builder : DrawCall::Builder {

        void setup() override;

        UberLayer* ubl;
        
        Builder(UberLayer* ubl);

    };

    UberEffector effector;
    
    Layer::Feedback* feedback() override;

    struct Feedback : Layer::Feedback {

        UberLayer* ubl;
        
        Feedback(UberLayer* ubl);

        std::string  header() override;

    };

    struct VirtualLayer : Effectable {

        VirtualLayer(UberLayer* ubl, int w = 0, int h = 0);

        int w ;
        int h;

        UberLayer* ubl;

        struct Effector : ::Effector {

            Effector(VirtualLayer* vlayer);

            VirtualLayer* vlayer;

            bool setup(::Builder* builder) override;
            
            std::string body(::Builder* builder, std::string prepend = "") override;

        };
        
        Effector effector;

    };

    std::vector<std::shared_ptr<VirtualLayer>> layers;

    VirtualLayer& addLayer(int w , int h) ; // kinda ctor for VLaye

};
