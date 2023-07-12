
#include "smartmap.hpp"

#include "imgui/imgui.h"

// #define SM_DEBUG
#ifdef SM_DEBUG
#include <windows.h>
#include <ctime>
#include <cstdint>

static inline std::map<int,int> filechecks;
static inline int survey_count = 0;
static inline void survey(const char* path, std::function<void()> cb = [](){}) {

    WIN32_FILE_ATTRIBUTE_DATA fileInfo; GetFileAttributesExA(path, GetFileExInfoStandard, &fileInfo);
    SYSTEMTIME st; FileTimeToSystemTime(&fileInfo.ftLastWriteTime, &st);
    auto last = st.wMilliseconds;

    if (filechecks[survey_count] != last) { filechecks[survey_count] = last;  cb(); }
    survey_count++;

}

#endif



SmartMap::SmartMap() {

    // order matters for some
    artnet = new Artnet("2.0.0.222");
    window = new Window(false,400,300,1540);
    window->setPos(2560,1440-1080);
    window->setSize(1920,1080);
    MAT_X = 8; 
    MAT_Y = 2;
    float scale = 1;
    FW = window->width*MAT_X*scale;
    FH = window->height*MAT_Y*scale;
    gui = new GUI(window->window);
    quadC = new VBO("quad.obj",0);
    quadA = new VBO("quad.obj",2);
    quadB = new VBO("quad.obj",1);
    passBuf = new Texture(nullptr,FW,FH);
    passBuf->format = GL_RGBA8;
    outBuf = new Texture(nullptr, FW,FH); 
    outBuf->format = GL_RGBA8;
    outFB = new FrameBuffer(outBuf); 
    // outBlur = new Texture(nullptr, FW*.5,FH*.5); 
    // outBlur->format = GL_RGBA8;

    shader = new ShaderProgram({"smartmap.frag", "smartmap.vert"});
    blur_x = new ShaderProgram({"blur_x.comp"});
    blur_y = new ShaderProgram({"blur_y.comp"});
    basic = new ShaderProgram({"test.frag", "basic.vert"});

    
    shader->use();
    atlas = new Atlas("assets/media/");
    atlas->link(shader);


    winFB = new FrameBuffer(0,window->width,window->height); 
}

void SmartMap::createLayer(GLuint chan, GLuint uni, Fixture *fixture, int count, float mode) {

    std::vector<std::array<float, 4>> mat = matrice(MAT_X,MAT_Y);
    matriceUBO = new UBO("MatriceUBO", mat.size()*32, {shader->id}); 
    matriceUBO->update(&mat[0][0],mat.size()*32); 
    shader->sendUniform("MatriceUBOSize", MAT_X*MAT_Y);

    fixtureUBO = new UBO("FixtureUBO", 24*16, {shader->id}); 
    new UBO("FixtureUBO2", 24*16, {shader->id}); 

    artnet->universes[uni].output = &fixtureUBO->data[0];
    artnet->universes[uni].remap_specs = *fixture;
    artnet->universes[uni].quantity = count;
    artnet->universes[uni].mode = mode;
    
}


static int  cell_min = 0, cell_max = 255, cells_count = 48;
 
void SmartMap::render() {

    while(true) window->render([&]() {

        for (int i = 0; i < 10; i++) shader->sendUniform("debug"+std::to_string(i), debuguniforms[i]);

        artnet->run(); 

        fixtureUBO->update();

        winFB->clear(); 
        outFB->clear(); // thus bind

        passBuf->bind();

        shader->sendUniform("mode", debuguniforms[artnet->universes[0].mode]);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_SRC_COLOR);
        quadA->draw(artnet->universes[0].quantity); 
        glBlendFunc(GL_BLEND_MODES[GL_BLEND_MODE_IN], GL_BLEND_MODES[GL_BLEND_MODE_OUT]);
        quadB->draw(artnet->universes[0].quantity);

        passBuf->read(outBuf);

        // glBindImageTexture(0, *outBlur, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        // glBindImageTexture(1, *outBuf, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
        // blur_x->use(FW*.5/16,FH*.5/16);
        // blur_y->use(FW*.5/16,FH*.5/16);
        // glMemoryBarrier( GL_ALL_BARRIER_BITS ); 

        
        winFB->bind(); 
        outBuf->bind();
        shader->use();
        quadC->draw();



        //////////////////////////////////////////////
        //////////////////////////////////////////////
        //////////////////////////////////////////////
        // GUI STUFF
        //////////////////////////////////////////////
        //////////////////////////////////////////////

        gui->newframe();  

        ///// BUFFERS

        ImGui::Begin("KTRL");
        
            // ImGui::SetWindowFontScale(1.5);

            ImGui::Checkbox("Debug mode", &debug);
            
            uint16_t min = 1, max = 2560;

            if (ImGui::DragScalarN("winsize", ImGuiDataType_U16,  &window->width,  2, 1, &min,&max)) window->updateSize();
            if (ImGui::DragScalarN("winpos", ImGuiDataType_U16,  &window->offset_x,  2, 1, &min,&max)) window->updatePos();

            ImGui::Separator();
            
            for (auto fps:FPS::pool) if (fps->fps > 1) ImGui::Text((fps->name+": "+std::to_string((GLuint)(fps->fps))+" FPS").c_str());
            
            ImGui::Separator();
            
            for (int i = 0; i < 10; i++) ImGui::SliderFloat(("debug "+std::to_string(i)).c_str(), &debuguniforms[i], 0, 1);

            ImGui::Separator();

            ImGui::SliderInt("GL_BLEND_MODE_IN",&GL_BLEND_MODE_IN,0,GL_BLEND_MODES.size());
            ImGui::SliderInt("GL_B2LEND_MODE_OUT",&GL_BLEND_MODE_OUT,0,GL_BLEND_MODES.size()); 


            // if (ImGui::InputText(" tex", (char*)&tex->path[0], IM_ARRAYSIZE((char*)&tex->path[0]))) tex->reset();
            // if (ImGui::InputText(" frag", (char*)&basic->paths[1][0], IM_ARRAYSIZE((char*)&basic->paths[1][0]))) basic->reset();
            


        ///// ARTNET

        for (auto dmx : artnet->universes) {

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2,2));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

            ImGui::Begin(std::string("Artnet Universe "+std::to_string(dmx.first)).c_str());
                
                auto window_width = ImGui::GetWindowWidth();

                int cell_width = window_width / cells_count - 2;


                for (int i = 0; i < 512; i++) {

                    ImGui::PushID(i);

                    ImGui::VSliderScalar("",  ImVec2(cell_width,30),    ImGuiDataType_U8, &dmx.second.raw[i],  &cell_min,   &cell_max,   "");

                    if ((i + 1) % cells_count != 0) ImGui::SameLine(0);


                    ImGui::PopID();

                }


            ImGui::End();

            ImGui::PopStyleVar(5);

        ImGui::End();

        ///// UBO
        
        ImGui::Begin("FixtureUBO");
        // for (int i = 0; i < 20; i++) ImGui::SliderScalar(std::to_string(i).c_str(), ImGuiDataType_U8, (uint8_t*)(&artnet->universes[0].raw[i]),  &min,   &max,   "");
        for (int i = 0; i < 32; i++) ImGui::SliderFloat(("uniform "+std::to_string(i)).c_str(), &fixtureUBO->data[i], 0, 1);
        ImGui::End();

        ///// TEXTURES

        ImGui::Begin("VIEW");

        for (int i = Texture::pool.size()-1; i != 0; i--) {

            ImGui::Text(("width: " + std::to_string(Texture::pool[i]->width)).c_str());
            ImGui::Text(("height: " + std::to_string(Texture::pool[i]->height)).c_str());
            
            float ratio = Texture::pool[i]->height/(float)Texture::pool[i]->width;
            auto nw = std::min(Texture::pool[i]->width,(GLuint)512);

            ImGui::Image((void*)(intptr_t)(ImTextureID)(uintptr_t)Texture::pool[i]->id, ImVec2(nw,nw*ratio));

            ImGui::PushID(i+100);

            ImGui::PopID();
            ImGui::Separator();
        }
        
        ImGui::End(); 

        gui->render();  

            break;

        } 





        //////////////////////////////////////////////
        //////////////////////////////////////////////
        //////////////////////////////////////////////
        // DEBUG STUFF
        //////////////////////////////////////////////
        //////////////////////////////////////////////

        if (debug) { 

            survey_count = 0;
            std::vector<std::array<float, 4>> mat = matrice(MAT_X,MAT_Y);

            survey(("C:/msys64/home/SysErr/old/smartmap/assets/shader/"+std::string(shader->paths[0])).c_str(), [&](){ shader->reset(); atlas->link(shader); matriceUBO->update(&mat[0][0],mat.size()*16); shader->sendUniform("MatriceUBOSize", MAT_X*MAT_Y); }); 
            survey(("C:/msys64/home/SysErr/old/smartmap/assets/shader/"+std::string(shader->paths[1])).c_str(), [&](){ shader->reset(); atlas->link(shader); matriceUBO->update(&mat[0][0],mat.size()*16); shader->sendUniform("MatriceUBOSize", MAT_X*MAT_Y); }); 

        }
        
    }); 

}

SmartMap& SmartMap::getInstance() { static SmartMap instance;  return instance; }
