#ifndef TEXTURE_H
#define TEXTURE_H	
	
#include "globals.hpp"


struct Texture {

    static inline std::vector<Texture*> pool;

    GLuint id = 0, width, height, mipmaps = 1, unit = 0;

    GLenum format = GL_RGB8;

    std::string path;

    Texture();

    Texture(void* data, GLuint width, GLuint height, GLuint offset_x = 0, GLuint offset_y = 0, GLuint unit = 0, GLenum format = GL_RGB8);

    Texture(std::string path);
    
    ~Texture();

    void create(void* data, GLuint width, GLuint height, GLuint offset_x, GLuint offset_y, GLuint unit, GLenum format);

    void read(const Texture *texture, GLuint offset_x = 0, GLuint offset_y = 0);
    void write(void* data, GLuint width, GLuint height, GLuint offset_x = 0, GLuint offset_y = 0);

    void bind(int unit);
    void bind();
    
    void destroy();
    
    operator GLuint();

};



#endif