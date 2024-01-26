#pragma once

#include "pch.hpp"

#include "window.hpp"
#include "ubo.hpp"
#include "framebuffer.hpp"
#include "stack.hpp"
#include "gui.hpp"
#include "model.hpp"
#include "shaderFX.hpp"
#include "drawcall.hpp"

#include "node.hpp"
#include "atlas.hpp"

#include "widgets/textures.hpp"


#include "tree.hpp"
#include "editor.hpp"

struct Engine {

    Window window;

    // UBO dynamic_ubo, static_ubo;

    int sequid = 1000;

    Atlas *atlas;

    // Stack stack;

    Tree tree;
    
    Node* stack = tree.add(new Node{"Stack"});
    
    GUI gui;

    std::array<char,512> blank{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    void init();

    static Engine& getInstance() { static Engine instance;  return instance; }

    void run();

    EditorWidget editorw;

private:

    Engine(uint16_t width = 400, uint16_t height = 300);

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    ~Engine();

};
