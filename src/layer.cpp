
#include "layer.hpp"
#include "drawcall.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "engine.hpp"
#include "instance.hpp"


#include "effector.hpp"
#include <string>

///////// Layer ////

Layer::Layer(uint16_t width, uint16_t height, std::string name) :

    fb((width?width:engine.window.width), 
    (height?height:engine.window.height)), 
    DrawCall(name),
    stat(name)
    
{

    static bool init = false;

    if (!init) {

        globals.layers.striding(true);

        engine.static_ubo->add(&globals.layers);

        glsl_layers =  &(*new Instance(*engine.static_ubo))[&globals.layers];

        init = true;

    }

    int xxx = glsl_layers->stl.back().m->quantity();

    vbo.layer_id = glsl_layers->stl.back().m->quantity();

    glsl_layers->stl.back().m->quantity(vbo.layer_id+1);

    // glsl_layers->eq(vbo.layer_id).set<std::array<float,2>>({(float)width,(float)height});

    // engine.static_ubo->upload();]
    stat.add(&globals.layer);
    engine.static_ubo->add(&stat);

}

Layer::Feedback* UberLayer::feedback() {  

    if (!feedback_v) 
        feedback_v = new UberLayer::Feedback(this);

    return feedback_v; 
    
}
Layer::Feedback* Layer::feedback() { 

    if (!feedback_v) 
        feedback_v = new Layer::Feedback(this);
    
    return feedback_v; 
    
}

void Layer::draw() {

    fb.clear();

    DrawCall::draw();

}

// FeedbackEffector  ////////////////

bool Layer::Feedback::setup(::Builder* builder) { 

    texture.read(&layer->feedback()->texture);

    builder->addSampler(&layer->feedback()->texture, "feedback");

    ADD_UNIQUE<::Effector*>(builder->effectors_fragment, this);
    
    return true; 
    
}

std::string Layer::Feedback::header() {

    std::string current;

    current += "void feedback(float intensity) {\n";
    current += "\tcolor += ( texture( feedback_pass, UV) - .002 ) * intensity;\n",
    current += "}\n";
    return current;

}

std::string UberLayer::Feedback::header() {

    std::string current;

    current += "void feedback(float intensity) { // washington\n";
    std::string name = lower(ubl->uberlayer_m.name());

    if (ubl->uberlayer_m.quantity()>1) {
    
        name += "[obj]";
    
        current += "int obj = int(OBJ);\n";
    
    }
    
    current += "\tcolor += ( texture( feedback_pass, UV*static_ubo."+name+".uberLayers.size+static_ubo."+name+".uberLayers.norm) - .002 ) * intensity;\n",
    current += "}\n";
    return current;

}


Layer::Feedback::Feedback(Layer* layer) : 

    Effector("feedback"), 
    texture(layer->fb.texture.width,layer->fb.texture.height), 
    layer(layer) 

    {

    add<float>("intensity");
    
}


UberLayer::Feedback::Feedback(UberLayer* ubl) : 

    Layer::Feedback(ubl)

    {

        this->ubl = ubl;
    
}






///////// UBEREFFECTOR //// 

void UberEffector::ubl(UberLayer* ubl) {

    ubl_v = ubl;

    if (!ubl) return;

}

UberEffector::UberEffector(UberLayer* ubl) : Effector(ubl?ubl->name():"ubereffector") {  

    this->ubl(ubl);
    

}

std::string  UberEffector::header() { 

    std::string out;
    std::string name = lower(ubl_v->uberlayer_m.name());
    if (ubl_v->uberlayer_m.quantity()>1) name += "[i]";

    out += "void " + ubl_v->name()+ "_effector(int from, int to) {\n\n";
        
        out +="\tfor (int i = from; i < to; i++) {\n\n";
        
            out +="\t\tvec2 tuv = uv;\n\n";
        
            out +="\t\ttuv *= static_ubo."+name+".uberLayers.size;\n";
        
            out +="\t\ttuv += static_ubo."+name+".uberLayers.norm;\n\n";
        
            out +="\t\tcolor += texture("+ubl_v->fb.texture.sampler_name+", tuv);\n\n";
        
        out +="\t}\n\n";
        
    
    out +="};\n";

    return out;

}

bool UberEffector::setup(::Builder* builder) { 

    builder->addSampler(&ubl_v->fb.texture, ubl_v->name());

    ADD_UNIQUE<::Effector*>(builder->effectors_fragment, this);

    return true; 
    
}

UberLayer::VirtualLayer::Effector::Effector(VirtualLayer* vlayer) : 

    ::Effector(vlayer?vlayer->name():"vlayereffector"), 

    vlayer(vlayer) 
    
{


}

bool UberLayer::VirtualLayer::Effector::setup(::Builder* builder) { 

    vlayer->ubl->effector.setup(builder);

    return true; 
    
}
std::string UberLayer::VirtualLayer::Effector::body(::Builder* builder, std::string prepend) {  

    int offset = 0, to = 0;

    bool broke = false;

    for (auto vlayer : vlayer->ubl->layers) 
        if (vlayer.get() == this->vlayer) {

            broke = true;
            break;
        }else 
            offset += vlayer->quantity();
        
    
    return "\t"+vlayer->ubl->name()+"_effector("+std::to_string(offset)+", "+std::to_string(offset+vlayer->quantity())+");\n";
    
    
}

std::string UberEffector::body(::Builder* builder, std::string prepend) {  
    
    return "\t"+ubl_v->name()+"_effector(0, "+std::to_string(ubl_v->uberlayer_m.quantity())+");\n";
    
}



///////// UBERLAYER ////

UberLayer::UberLayer() : 

    Layer(0,0,"UberLayer"), 
    uberlayer_m(engine.static_ubo->next_name(name())) 

{

    uberlayer_m.quantity(0);

    uberlayer_m.add(&globals.ubl);

    engine.static_ubo->add(&uberlayer_m);

    effector.ubl(this);
    
}

::Builder* UberLayer::builder() {

    if (!builder_v) 
        builder_v = new UberLayer::Builder(this); 

    return builder_v;

}

void UberLayer::calc_matrice() {

    auto layers = this->layers;

    std::sort( layers.begin(), layers.end(), [](auto a,auto b){ return a.get()->h > b.get()->h; } );

    auto first_of_line = layers.begin();

    int max_line_h = 0;
    int last_x = 0;
    int last_y = 0;
    int count = 0;

    std::vector<std::vector<std::array<int,5>>> matrice = {{}};

    for (auto it = layers.begin(); it != layers.end(); it++ ) {

        for (int i = 0 ; i < it->get()->quantity(); i++) {

            matrice.back().emplace_back( std::array<int,5>{it->get()->w, it->get()->h, last_x, last_y, count} );

            if (it->get()->h > max_line_h) max_line_h = it->get()->h;

            GLint gl_max_texture_size;
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);

            if (last_x+it->get()->w > gl_max_texture_size/2) { // beware /2 just for me

                matrice.resize( matrice.size()+1 );
                last_y += max_line_h;
                max_line_h = 0;
                last_x = 0;

            }else{

                last_x += it->get()->w;

            }

            count++;

        }
    }

    if (matrice.size()<1) 
        return;

    if (!matrice.back().size()) 
        matrice.resize(matrice.size()-1);

    float matrice_height = matrice.back()[0][1]+matrice.back()[0][3];
    float matrice_width = matrice[0].back()[0]+matrice[0].back()[2];

    fb.create( matrice_width, matrice_height );

    if (feedback_v)
        feedback_v->texture.create( matrice_width, matrice_height );

    uberlayer_m.quantity(count);

    vbo.vertices.quantity(0);
    vbo.indices.quantity(0);

    int z = 0;

    for (auto &x : matrice) for (auto &y : x) {

        auto w = y[0] / matrice_width;
        auto h = y[1] / matrice_height;
        auto x_ = y[2] / matrice_width;
        auto y_ = y[3] / matrice_height;

        auto glsl_uberlayer= Instance(*engine.static_ubo)[&uberlayer_m];

        // PLOGW << z << " - "  << "[ "<< y[0] << " " << y[1] << " " << y[2] << " " << y[3] << " ] - [" << w << " " << h << " " << x_ << " " << y_ << " ]";

        glsl_uberlayer.eq(z).set<float,8>({w, h,x_*2-1,y_*2-1, x_, y_,(float)y[0],(float)y[1]});

        vbo.addQuad(w, h, x_, y_);

        z++;
    }

    builder()->build();

    engine.static_ubo->upload();

}

UberLayer::VirtualLayer& UberLayer::addLayer(int w , int h) {

    layers.emplace_back(std::make_shared<UberLayer::VirtualLayer>(this, w,h));

    add(layers.back().get());

    return *layers.back().get();

}

UberLayer::VirtualLayer::VirtualLayer(UberLayer* ubl, int w, int h) : 

    ubl(ubl),
    Effectable(ubl->next_name("Vlayer")), 
    w(w?w:engine.window.width), 
    h(h?h:engine.window.height), 
    effector(this) 
        
{ 

}

UberLayer::Builder::Builder(UberLayer* ubl) : DrawCall::Builder(ubl), ubl(ubl) {

}

void UberLayer::Builder::setup() {

    DrawCall::Builder::setup();

    for (auto &model : ubl->layers) 
        for (auto x : model->effector_refs) 
            x.get()->effector->setup(this);

    std::string ar_str = lower(
        ubl->uberlayer_m.name())+std::string(ubl->uberlayer_m.quantity()>1?
        "[obj]":"")+
        ".uBL";

    if (ubl->layers.size() == 1) 
        body_fragment += print_layer( *ubl->layers[0].get(), lower(dc->name()), "obj", ar_str );

    else {

        int last_id = 0;

        for (auto &x : ubl->layers) {

            if (last_id) body_fragment += "\n} else ";

            auto l = last_id;
            last_id += x.get()->quantity();

            body_fragment += "if (obj < "+std::to_string(last_id)+" ){ "+(l?"obj -= "+std::to_string(l)+";":"")+"\n\n" + print_layer( *x.get(), lower(dc->name()), "obj", ar_str );

        }

        if (ubl->layers.size()) body_fragment += "\n}\n";

    }

    body_fragment += "\n";

}

