#include "atlas.hpp"  

#include "directory.hpp"  
#include "engine.hpp"  
#include "shader.hpp"  
#include "texture.hpp"
#include "ubo.hpp"
#include "object.hpp"
#include "struct.hpp"

#include "image.hpp"


Atlas::Atlas(int width, int height, std::string path)  : binpack(width,height,0)  {

    if (!buffer) buffer = engine.static_ubo->addObj(new Struct("mediasCoords", {"Size","Position", }));

    texture = new Texture(width,height,1,1);

    texture->mipmaps = 10;

    if(path.length()) fromDir(path);

}

void Atlas::clear() {

    PLOGD << "clear";

    normalized_list.resize(0);

    binpack.Init(texture->width,texture->height);

}

void Atlas::fromDir(std::string path) {

    texture->clear();

    if (!Directory::exist(path)) return;

    this->path = path;

    clear();

    Directory dir(path);

    for (auto &file:dir.list) {
        
        Image img(file);

        if (!img.loaded) continue;
        
        auto r = binpack.Insert(img.width, img.height, rbp::MaxRectsBinPack::RectBestShortSideFit);
        if (!r.width) {PLOGW << "needniouatlas"; continue;} 

        normalized_list.push_back({r.width/(float)this->texture->width, r.height/(float)this->texture->height, r.x/(float)this->texture->width, r.y/(float)this->texture->height});

        buffer->push(&normalized_list[0][0]);

        texture->write(&img.data[0],r.width,r.height,r.x,r.y,1,1);

    }

}

void Atlas::link(ShaderProgram* shader) {

    engine.static_ubo->subscribers.push_back(shader);
    engine.static_ubo->update();

    engine.static_ubo->upload();
    
    shader->sendUniform("mediasAtlas", 1);
    texture->bind();

}
 