#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>

#include <set>
#include <map>
#include <vector>

#include <string>
#include <cstring>

#include <cmath>

#include <dirent.h>
#include <fstream>
#include <sstream>
#include <thread>

#include <functional>

#include <iterator>
#include <regex>

#include <chrono>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <RectangleBinPack/GuillotineBinPack.h> 

#include "file.hpp"

struct vec2 { float x,y; };
struct vec4 { float x,y,z,w; };
struct RectF { vec2 size = {1,1}; vec2 pos = {0,0}; };

static void glInfo(GLenum glenum, std::string name = "Info") {

    int data;

    glGetIntegerv(glenum, &data);

    std::cout << name << ": " << data <<std::endl;

}

#define DEBUG_GL

#ifdef DEBUG_GL
std::map<GLenum,std::string> GL_errors = {

  { GL_NO_ERROR, "no error" },
  { GL_INVALID_ENUM, "invalid enumerant" },
  { GL_INVALID_VALUE, "invalid value" },
  { GL_INVALID_OPERATION, "invalid operation" },
  { GL_STACK_OVERFLOW, "stack overflow" },
  { GL_STACK_UNDERFLOW, "stack underflow" },
  { GL_OUT_OF_MEMORY, "out of memory" },
  { GL_NO_ERROR, "unknown" }

};
void glError(std::string file_name, std::string function_name, GLuint line) {

  GLenum code = glGetError();

  if (code == GL_NO_ERROR) return;

  std::string code_string = "unknown";

  if (GL_errors.find(code) != GL_errors.end()) code_string  = GL_errors[code];

  file_name = file_name.substr(file_name.find_last_of("\\")+1);
  function_name = function_name.substr(function_name.find(" "));
  function_name = function_name.substr(0,function_name.find("("));

  std::cout << file_name << "(" << line << "): " << function_name  <<  "() => " << code_string << '\n';

}

#define GL_ERROR() glError(__FILE__, __PRETTY_FUNCTION__, __LINE__)
#else
#define GL_ERROR()
#endif


#endif