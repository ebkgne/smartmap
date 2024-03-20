#include "vbo.hpp"  

#include "model.hpp"  
#include "file.hpp"  
#include "instance.hpp"  
#include "engine.hpp"  
#include "log.hpp"  

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

VBO::VBO() : Buffer("VBO"), vertice_array("Vertice_arr", 0), indice_array("Indice_arr", 0) {

    vertice_array.add(vertice);
    vertice_array.striding(true);
    add(vertice_array);

    indice_array.add(indice);
    indice_array.striding(true);
    add(indice_array);


    vertices = (*this)[0];
    indices = (*this)[1];

    create();

}

void VBO::destroy() {

    init = false;

    if (vbo<0) return;

    glDisableVertexAttribArray(0); 
    glDisableVertexAttribArray(1); 
    glDisableVertexAttribArray(2); 
    glDisableVertexAttribArray(3); 

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);

    for (auto m : models) delete m;

}

void VBO::create() {

    destroy();

    glGenBuffers(1, &vbo); glGenBuffers(1, &ibo); glGenVertexArrays(1, &vao);

    init = true;

}

VBO::~VBO()  { destroy(); }

void VBO::update() { Buffer::update(); if (init) upload(); }

void VBO::upload() {

    // tofix

    static std::vector<float> backup_quad = {

        -1,-1, 0,0, 0,
        1,-1, 1,0, 0,
        -1,1, 0,1, 0,
        1,1, 1,1, 0,

    };

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER,  vertices.size(), vertices.data(), GL_STATIC_DRAW );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), indices.data(), GL_STATIC_DRAW );

    int offset = 0;
    int i = 0;
    for (auto & m : indice.members) {

        auto type = GL_FLOAT;
        if (m->type() == typeid(float)) type = GL_FLOAT;

        auto count = 1; // m->count(); does it
        if (m->type() == typeid(glm::vec2)) count = 2;
        else if (m->type() == typeid(glm::vec3)) count = 3;
        else if (m->type() == typeid(glm::vec4)) count = 4;

        glVertexAttribPointer(i, count, type, GL_TRUE, m->size(), (const void*)offset);

        glEnableVertexAttribArray(i++);

        offset+= m->size();

    }

}

void VBO::draw(int count) {

    glBindVertexArray(vao); 

    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, count);

}
	

int VBO::import(File *file) {    

    // return 0;  // tofix

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFileFromMemory(&file->data[0], file->data.size(), aiProcess_CalcTangentSpace       | 
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) PLOGW << "Failed to load OBJ file: " << importer.GetErrorString();


    auto vertices = (*this)[0][0];
    auto indices = (*this)[1][0];

    auto mesh = scene->mMeshes[0];

    int next_indice =  vertices.member->members.size();

    for (int i = 0; i < mesh->mNumVertices; i++) {

        const aiVector3D& vertex = mesh->mVertices[i];
        
        auto v = vertices.push();

        v.member->print();
        PLOGD << v.member->name();

        v["Position"].set<glm::vec2>({ vertex.x, vertex.y });
        v["UV"].set<glm::vec2>({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
        v["ID"].set<uint32_t>(0);


    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      
        const aiFace& face = mesh->mFaces[i];
        auto indice = indices.push();

        indice[0].set<uint32_t>(next_indice+face.mIndices[0]);
        indice[1].set<uint32_t>(next_indice+face.mIndices[1]);
        indice[2].set<uint32_t>(next_indice+face.mIndices[2]);


    }

    upload();

    // models.push_back(new Model(file, models.size()));

    // return models.size()-1;

    return 0;
    
}