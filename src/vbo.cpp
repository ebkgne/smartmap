#include "vbo.hpp"

#include "member.hpp"
#include "file.hpp"
#include "instance.hpp"
#include "utils.hpp"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "globals.hpp"

VBO::VBO(File* file) : VBO() { addFile(file); }

VBO::VBO() : Member("VBO"), vertices("Vertices"), indices("Indices") {

    buffering(true);

    vertices.add(&globals.vertice);
    vertices.quantity(0);
    add(&vertices);

    indices.add(&globals.indice);
    indices.quantity(0);
    add(&indices);

    create();

    ADD_UNIQUE<VBO*>(pool, this);

}

void VBO::destroy() {

    init = false;

    if (vao) {

        glBindVertexArray(vao);

        for (int i = 0 ; i < enabled_attrs; i++) glDisableVertexAttribArray(i);

        glDeleteVertexArrays(1, &vao);

    }

    glDeleteBuffers(1, &vbo);

    glDeleteBuffers(1, &ibo);

}

void VBO::create() {

    destroy();

    glGenBuffers(1, &vbo); glGenBuffers(1, &ibo);

    init = true;

}

VBO::~VBO()  {

    destroy();

    removeHard(indices);
    removeHard(vertices);

    REMOVE<VBO*>(pool, this);

}

void VBO::clear() {

    vertices.quantity(0);

    indices.quantity(0);
    


}

void VBO::reset() {

    clear();

    upload();

}

void VBO::update() { Member::update_pv(); }

void VBO::upload() {

    auto v_size = vertices.footprint_all();

    auto i_size = indices.footprint_all();

    if (!v_size || !i_size) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER,  v_size, data(), GL_STATIC_DRAW );

    if (!vao) {

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        int offset = 0;
        enabled_attrs = 0;
        for (auto & m : globals.vertice.members) {

            auto type = GL_FLOAT;
            if (m->type().id == typeid(int)) type = GL_INT;
            if (m->type().id == typeid(uint32_t)) type = GL_UNSIGNED_INT;

            glVertexAttribPointer(enabled_attrs, m->quantity(), type, GL_TRUE, globals.vertice.footprint_all(), (const void*)(size_t)offset);

            glEnableVertexAttribArray(enabled_attrs++);

            offset+= m->footprint_all();

        }

    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size, data() + v_size , GL_STATIC_DRAW );

    // std::string str;
    // for (int i = 0 ; i < size(); i++) str+= std::to_string(*(((uint8_t*)data())+i)) + " ";
    // PLOGW << name() << ": " << footprint_all() << " - " << vbo << " - " << str;

}

void VBO::draw(int count) {

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glDrawElementsInstanced(GL_TRIANGLES, indices.footprint_all()/sizeof(int), GL_UNSIGNED_INT, 0, count);

}

void VBO::addQuad(float w, float h, float x, float y, int id) {

    int o = vertices.quantity();

    if (o) id = Instance(*this)[&vertices].eq(o-1)[&globals.vertice]["OBJ"].get<float>()+1;

    auto x_ = x;
    auto y_ = y;
    auto w_ = (x+w);
    auto h_ = (y+h);

    vertices.quantity(vertices.quantity()+4);
    auto v = Instance(*this)[&vertices].eq(vertices.quantity()-4);
    v.set<float,32>(
        x_*2-1 , y_*2-1 , 0.0f, 0.0f, x_ , y_ ,  (float)id, (float)layer_id,
        w_*2-1 , y_*2-1 , 1.0f, 0.0f, w_ , y_ ,  (float)id, (float)layer_id,
        x_*2-1 , h_*2-1 , 0.0f, 1.0f, x_ , h_ ,  (float)id, (float)layer_id,
        w_*2-1 , h_*2-1 , 1.0f, 1.0f, w_ , h_ ,  (float)id, (float)layer_id
    );


    indices.quantity(indices.quantity()+2);
    auto i = Instance(*this)[&indices].eq(indices.quantity()-2);
    i.set<int, 6>(
        o+0,o+1,o+2,
        o+1,o+2,o+3
    );

    upload();

}

bool VBO::addFile(File* file, int id, int w , int h, int x, int y) { if (addFile_noupload(file,id,w,h,x,y)) { upload(); return true; } return false; }

bool VBO::addFile_noupload(File* file, int id, int w, int h, int x, int y) {

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFileFromMemory(&file->data[0], file->data.size(), aiProcess_CalcTangentSpace       |
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { PLOGE << "Failed to load OBJ file: " << file->path_v << " . " << importer.GetErrorString(); return false; }

    auto mesh = scene->mMeshes[0];

    int next_indice =  vertices.quantity();

    for (int i = 0; i < mesh->mNumVertices; i++) {

        const aiVector3D& vertex = mesh->mVertices[i];

        vertices.quantity(vertices.quantity()+1);

        auto v = Instance(*this)[&vertices].eq(vertices.quantity()-1)[&globals.vertice];

        Instance(v)["POSITION"].set<float, 2>({ vertex.x*w+x, vertex.y*h+y });
        Instance(v)["UV"].set<float, 2>({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
        Instance(v)["NORMALIZED"].set<float, 2>({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
        Instance(v)["OBJ"].set<float>(id);
        Instance(v)["LAYER"].set<float>(layer_id);

    }

    indices.quantity(indices.quantity()+mesh->mNumFaces);

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {

        const aiFace& face = mesh->mFaces[i];

        auto indice = Instance(*this)[&indices].eq(indices.quantity()-mesh->mNumFaces+i);

        indice.set<int, 3>(next_indice+(int)face.mIndices[0], next_indice+(int)face.mIndices[1], next_indice+(int)face.mIndices[2]);

    }

    return true;

}
