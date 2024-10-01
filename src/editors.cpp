#include "editors.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#undef IMGUI_DEFINE_MATH_OPERATORS 
#include "imgui/imgui_internal.h"
#include "ImGuiColorTextEdit/TextEditor.h"


#include "editor.hpp"
#include "folder.hpp"
#include "output.hpp"
#include "ndi.hpp"
#include "model.hpp"
#include "file.hpp"
#include "ubo.hpp"
#include "engine.hpp"
#include "vbo.hpp"
#include "shader.hpp"
#include "effector.hpp"
#include "artnet.hpp"
#include "texture.hpp"
#include "atlas.hpp"
#include "framebuffer.hpp"
#include "json.hpp"
#include "layer.hpp"
#include "gui.hpp"
#include "widgets.hpp"

#include "log.hpp"
#include <cstdint>
#include <map>

#include <sstream>
#include <string>



using namespace ImGui;


void Editors::init() {

    // ////////// xxx.HPP

    // Editor<xxx>([](Node* node, xxx *x){ });

    ////////// Artnet.HPP



    Editor<NDI::Sender>([](Node*node, NDI::Sender* sender){



        Editor<Output>::cb_typed( node, sender);


            ImGui::NewLine();
            ImGui::NewLine();

        if (sender->pixelcount) RawWidget( &sender->data[0], sender->pixelcount );


    });

    Editor<DMXRemap>([](Node*node, DMXRemap* remap){

        if (!remap->dst || !remap->src)
            return;

        if ( BeginPopupContextWindow()){


            if (Button("reset")) 
                remap->extract(remap->dst->m());

            
            EndPopup();

        }

        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("##chjdshjkers", &remap->chan);
        SameLine();
        ImGui::SetNextItemWidth( -FLT_MIN);
         ImGui::InputText("###puppybbb", &remap->dst->stl_name(1)[0], 10, ImGuiInputTextFlags_ReadOnly);

        if (ImGui::BeginTable("##remapswindow", 7, ImGuiTableFlags_Borders, ImVec2(GetContentRegionAvail().x, 0))) {

            ImGui::TableSetupColumn("##state", ImGuiTableColumnFlags_WidthFixed, 20);
            ImGui::TableSetupColumn("name");
            ImGui::TableSetupColumn("skip", ImGuiTableColumnFlags_WidthFixed, 35);
            ImGui::TableSetupColumn("coarse", ImGuiTableColumnFlags_WidthFixed, 35);
            ImGui::TableSetupColumn("fine", ImGuiTableColumnFlags_WidthFixed, 35);
            ImGui::TableSetupColumn("ultra", ImGuiTableColumnFlags_WidthFixed, 35);
            ImGui::TableSetupColumn("min/max", ImGuiTableColumnFlags_WidthFixed, 106); 
            ImGui::TableHeadersRow();

            int member_id = 0;

            remap->dst->each([&](Instance inst) {

                auto *m = inst.stl.back().m;

                 if (!m->isData()) return;

                    static int e = 0;

                    bool changed = false;

                    ImGui::TableNextRow();

                    // if (ImGui::IsItemHovered()) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(.5,.5,.5,1));

                    ImGui::TableNextColumn();
                    if (ImGui::Checkbox(("##arsbt"+std::to_string(member_id)).c_str(), &remap->attributes[member_id].active)) 
                        changed = true;

                    if (!remap->attributes[member_id].active)
                        BeginDisabled();
                    ImGui::TableNextColumn();
                    // auto textWidth   = 100-ImGui::CalcTextSize(m->name().c_str()).x;
                    // ImGui::SetCursorPosX((textWidth * 0.5f) );

                    std::string name = m->name();
                    if (inst.stl.size()>1) name = inst.stl[inst.stl.size()-2].m->name()+"."+name;

                    ImGui::Text(name.c_str());

                    ImGui::TableNextColumn();
                    if (ImGui::RadioButton(("##arbt"+std::to_string(member_id)).c_str(), &remap->attributes[member_id].combining , 0)) 
                        changed = true;
                    ImGui::TableNextColumn();
                    if (ImGui::RadioButton(("##vrbt"+std::to_string(member_id)).c_str(), &remap->attributes[member_id].combining , 1)) 
                        changed = true;
                    ImGui::TableNextColumn();
                    if (ImGui::RadioButton(("##xrbt"+std::to_string(member_id)).c_str(), &remap->attributes[member_id].combining , 2)) 
                        changed = true;
                    ImGui::TableNextColumn();
                    if (ImGui::RadioButton(("##srbt"+std::to_string(member_id)).c_str(), &remap->attributes[member_id].combining , 3)) 
                        changed = true;
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(100);
                    if (ImGui::DragScalarN(("##minmax"+std::to_string(member_id)).c_str(), ImGuiDataType_Float, &remap->attributes[member_id].min, 2)) 
                        changed = true;

                    if (changed) {
                        for (int i = 1; i < m->quantity(); i++) {
                            remap->attributes[member_id+i].min = remap->attributes[member_id].min;
                            remap->attributes[member_id+i].max = remap->attributes[member_id].max;
                            remap->attributes[member_id+i].combining = remap->attributes[member_id].combining;
                            remap->attributes[member_id+i].active = remap->attributes[member_id].active;
                        }
                    }


                    if (!remap->attributes[member_id].active)
                        EndDisabled();                

                    member_id+=m->quantity();

                    // if (ImGui::IsItemHovered()) ImGui::PopStyleColor();


                

            });

                ImGui::EndTable();
            }

        // std::stringstream deststart;
        // deststart  <<  (void*)remap->dst->data();
        // deststart  << " - " << (void*)remap->dst->stl.front().m->data();
        // deststart  << " - " << remap->dst->offset;
        // ImGui::Text(deststart.str().c_str()) ;


        // ImGui::SameLine(); ImGui::Text(("* "+std::to_string(remap->dst->stl.back().m->quantity())).c_str());

        // engine.
    });

    Editor<Universe>([](Node*node,Universe* uni){ 

        if (RawWidget(uni->instance->data(), 512)) 
            node->update();

    });

    Editor<Artnet>([](Node*node,Artnet* an){

        ImGui::Text(an->name().c_str());

        ImGuiTextBuffer tbuff;
        for (auto x : an->available_ips) tbuff.append(x.c_str(), x.c_str()+x.length()+1);

        if (Combo("device##available_ips", &an->device_id, tbuff.begin())) {

            node->active(false);
            an->connect(an->available_ips[an->device_id]);

        }

        node->each<Universe>([](Node* n, Universe* uni){

            std::string str = "universe "+std::to_string(uni->id);
            
            ImGui::Text(str.c_str());

            ImGui::NewLine();

            Editor<Universe>::cb_typed(n, uni);

        });

    });


    ////////// WINDOW.HPP

    Editor<Window>([](Node* node, Window *window){

        using namespace ImGui;

        Editor<Output>::cb_typed( node, window);


        NewLine();

        int i = 0;

        for (auto &display : Window::displays) {

            if (Button( ("fit display "+std::to_string(i++)).c_str())) {

                window->pos( display.x, display.y );
                window->size( display.width, display.height );

            }
            SameLine();
        }
        NewLine();
        NewLine();


        if (window->shader) Editor<ShaderProgram>::cb_typed( node, window->shader);

    });

    ////////// OUTPUT.HPP

    Editor<Output>([](Node* node, Output *output){

        static std::map<Output*,int> output_currents; 
        static int min0 = 0; 
        static int min1 = 1; 
        ImGuiTextBuffer tbuff;

        for (auto x : engine.tree->find("main")->childrens) tbuff.append(x->name().c_str(), x->name().c_str()+x->name().length()+1);

        if (tbuff.size()) if (ImGui::Combo("source", &output_currents[output], tbuff.begin())) {

            auto main = engine.tree->find("main");

            main->each<Layer>([&](Node* n, Layer* layer){

                if (layer && &layer->fb == output->fb) n->referings.erase(node);

            });
            
            output->fb = &main->childrens[output_currents[output]]->is_a<Layer>()->fb;

            main->childrens[output_currents[output]]->referings.insert(node);
            
        }

        if (ImGui::DragScalarN("position###winpos", ImGuiDataType_S32,  &output->offset_x, 2, 1, &min0)) 
            output->pos( output->offset_x, output->offset_y );

        if (ImGui::DragScalarN("size###winsize", ImGuiDataType_U32,  &output->width, 2, 1, &min1)) 
            output->size( output->width, output->height );

    });

    ////////// SHADER.HPP


    Editor<ShaderProgram>([](Node* node, ShaderProgram *shader){

        if (ImGui::BeginPopupContextItem()){

            if (ImGui::Button("create")) {
                // builder->build();
                // shader.create(builder->frag(), builder->vert());
                node->update();
            }
            if (ImGui::Button("empty")) { 

                static Builder BUILDER;
                Layer* lay = node->is_a<Layer>(); 
                if (lay) 
                    BUILDER.vbo = &lay->vbo; 
                shader->destroy(); 
                BUILDER.shader = shader;
                BUILDER.build();
                shader->create(BUILDER.frag(), BUILDER.vert()); 
            }
            
            EndPopup();
        
        }

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        static std::map<EditorWidget*,int> active_tab;

        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {

            if (ImGui::BeginTabItem("fragment")) {

;
                if (active_tab[EditorWidget::current] != 1 || EDITOR::triglist.find(node) != EDITOR::triglist.end()) {
                    
                    EditorWidget::current->editor.SetText(shader->frag.src);
                    EditorWidget::current->editor.SetErrorMarkers(shader->frag.errors);
                    node->error = shader->frag.errors.size();

                    EDITOR::triglist.erase(node);
                    active_tab[EditorWidget::current] = 1;

                }

                
                EditorWidget::current->editor.Render("fragment");

                if (EditorWidget::current->editor.IsTextChanged()) {
                    
                    std::string x = EditorWidget::current->editor.GetText();

                    memset(&x[x.length()-1],0,1); // against new line everytime dafuk
                    shader->destroy(); 
                    shader->create(x,shader->vert.src);

                    EditorWidget::current->editor.SetErrorMarkers(shader->frag.errors);
                    node->error = shader->frag.errors.size();

                    

                }
                

                ImGui::EndTabItem();

            }
            if (ImGui::BeginTabItem("vertex")) {

                if (active_tab[EditorWidget::current] != 2 || EDITOR::triglist.find(node) != EDITOR::triglist.end()) {
                
                    EditorWidget::current->editor.SetText(shader->vert.src);
                    EditorWidget::current->editor.SetErrorMarkers(shader->vert.errors);
                    node->error = shader->vert.errors.size();
              
                    EDITOR::triglist.erase(node);

                    active_tab[EditorWidget::current] = 2;

                }

                EditorWidget::current->editor.Render("vertex");

                if (EditorWidget::current->editor.IsTextChanged()) {

                    std::string x = EditorWidget::current->editor.GetText();

                    memset(&x[x.length()-1],0,1); // against new line everytime dafuk
                    shader->destroy(); 
                    shader->create(shader->frag.src,x);

                    EditorWidget::current->editor.SetErrorMarkers(shader->vert.errors);
                    node->error = shader->vert.errors.size();
                    
                }


                ImGui::EndTabItem();

            }

            ImGui::EndTabBar();

        }


    });



    ////////// File.HPP

    Editor<File>([](Node* node, File *file){

        if (file->extension == "obj") {

            if (ImGui::Button("FBX")) Model::convert(file, "fbx"); ImGui::SameLine();
            if (ImGui::Button("3DS")) Model::convert(file, "3ds"); ImGui::SameLine();
            if (ImGui::Button("DXF")) Model::convert(file, "dxf"); ImGui::SameLine();

        }

        char path[512];
        memset(path,0,512);
        memcpy(path,file->path_v.c_str(),file->path_v.size());

        if (ImGui::InputText("path", path, 512)) {

            if (strcmp(path, file->path_v.c_str())) {

                file->read(path); // only if not local file

                node->name(file->name());

            }

        }

        char data[512000];
        memset(data,0,512000);
        memcpy(data,&file->data[0],file->data.size());

        static TextEditor codeeditor;
        static File *curr = nullptr;
        if (curr != file) {

            codeeditor.SetShowWhitespaces(false);
            codeeditor.SetReadOnly(false);
            codeeditor.SetText(file->data.data());
            curr = file;
        }

        codeeditor.Render("codeeditor");

        if (codeeditor.IsTextChanged()) {

            if (!file->owned) file->write(codeeditor.GetText().c_str());

            else file->loadString(codeeditor.GetText().c_str());

            node->update();

        }

    });


    ////////// Texture.HPP

    Editor<Texture>([](Node* node, Texture *texture){
        static std::map<Node*,uint32_t[2]> dims;

        if (dims[node][0] != texture->width)
            dims[node][0] = texture->width;
        if (dims[node][1] != texture->height)
            dims[node][1] = texture->height;

        if (ImGui::DimWiget(&dims[node][0], &dims[node][1], node->name())) {

            auto layer = node->is_a<Layer>();

            if (layer && dims[node][0] && dims[node][1])
                layer->fb.create(dims[node][0], dims[node][1]);

        } 

        auto w = GetContentRegionAvail().x;
        auto h = w*((texture->height)/(float)texture->width);
        if (h > 400) {
            auto nw = (400/h)*w;
            SetCursorPosX((w-nw)/2);
            h = 400;
            w=nw;
        }

        ImGui::Image((void*)(uintptr_t)texture->id, ImVec2(w, h)); 
    });

    ////////// HPP

    Editor<Member>([](Node* node, Member *m){


        // std::stringstream ss;
        // ss << m;
        // ImGui::Text(( ss.str()).c_str());
         if (ImGui::BeginTabBar("def", ImGuiTabBarFlags_None)) {


            if (ImGui::BeginTabItem("programmer")) {
                
                auto widget = SlidersWidget(m);
                if (m->buffering() && widget[0]) {
            
                    m->upload();

                    // engine.stack->each<UberLayer>([](Node*n, UberLayer* ubl){ ubl ->fb.clear();});
                    // engine.stack->each<Layer>([](Node*n, Layer* layer){ layer ->fb.clear();});

                    // engine.stack->each([](Node* node){ if (node->type().id == typeid(UberLayer) || node->type().id == typeid(Layer)) ((Layer*)node->ptr)->fb.clear(); });

                }

                ImGui::EndTabItem();

            }
            if (ImGui::BeginTabItem("definition")) {
                

                DefWidget(m);

                ImGui::Separator();

                if (m->buffering() && RawWidget(m->data(),m->footprint())) m->upload();

                ImGui::Separator();

                ImGui::EndTabItem();

            }
                
            ImGui::EndTabBar();

        }



    });


    ////////// UBO.HPP

    Editor<UBO>([](Node* node, UBO *ubo){

        // std::string subs_str = "data[" + std::to_string(ubo->data.size())+"]";
        // std::string subs_str = std::to_string(ubo->subscribers.size())+" sub(s):";
        // for (auto s: ubo->subscribers) subs_str += " #"+std::to_string(s->id);
        // ImGui::Text(subs_str.c_str());

        Editor<Member>::cb_typed( node, ubo);

    });

    ////////// MODEL.HPP

    Editor<Model>([&](Node* node, Model *model){

        Editor<Effectable>::cb_typed( node, model);


    });

    Editor<Instance>([&](Node* node, Instance *instance){ 

        auto widget = SlidersWidget(instance->buff(), node->is_a<Member>(), instance->offset);
        if (widget[0])
            instance->buff()->upload();
        if (widget[1])
            node->update ();

        auto ubl_ = node->is_a_nowarning<UberLayer>();
        if (ubl_ && widget[1]) {

            ubl_->calc_matrice();
            ubl_->update_pv();
            ubl_->upload();
        }
        

    });

    Editor<Effectable>([&](Node* node, Effectable *effectable){

        ImGui::SeparatorText(effectable->name().c_str());

        if (effectable->instance)
            Editor<Instance>::cb_typed( node, effectable->instance);

    });

    ////////// Artnet.HPP
    ////////// Artnet.HPP

    // Editor<Universe>([](Node*node,Universe* dmx){



    //     for (auto &r : dmx->remaps) Editor<Universe::Remap>::cb( node, &r);

    // });

    ////////// VBO.HPP

    Editor<VBO>([](Node*node,VBO*vbo){


        if (ImGui::BeginPopupContextItem()){

            std::ostringstream address;
            address << (void const *)vbo;
            ImGui::Text(("VBO " + std::to_string(vbo->vbo) + " - IBO " + std::to_string(vbo->ibo) + " - VAO " + std::to_string(vbo->vao) + " - " +  address.str()).c_str());

            if (ImGui::Button("destroy")) vbo->deleteData();
            if (ImGui::Button("create")) vbo->create();
            if (ImGui::Button("reset")) vbo->reset();
            if (ImGui::Button("addQuad")) vbo->addQuad();
            if (ImGui::Button("glDeleteVertexArrays")) glDeleteVertexArrays(1, &vbo->vao);
            
            EndPopup();
        
        }

        Editor<Member>::cb_typed( node, vbo);

    });

  /////////////////////////////////


    ////////// Atlas.HPP

    Editor<Atlas>([](Node* node, Atlas *atlas){

        if (atlas) Editor<Texture>::cb_typed( node, atlas->texture);
        else PLOGE << "NONONON";

        // Editor<Object>::cb( node, atlas->m);  // tofix


    });

    ////////// Component.HPP

    // Editor<Component>([](Node* node, Component *comp){

    //     for (auto &m : comp->members) {

    //         std::string str = std::string(m.type_name()) + " " + m.name+ "; ";
    //         ImGui::Text(str.c_str());
    //     }


    // });
    ////////// JSON.HPP

    Editor<JSON>([](Node* node, JSON *json){

        JSON::if_obj_in("models",json->document, [&](auto &m) { ImGui::Text(m.name.GetString());ImGui::SameLine();ImGui::Text(m.value.GetString()); });
        JSON::if_obj_in("effectors",json->document, [&](auto &m) { ImGui::Text(m.name.GetString());ImGui::SameLine();ImGui::Text(m.value.GetString()); });

    });
    ////////// FRAMEHPP

    Editor<FrameBuffer>([](Node* node, FrameBuffer *fb ){

        // ImGui::Text(("attachment "+std::to_string(fb->attachments)).c_str());

        Editor<Texture>::cb_typed( node, &fb->texture);

    });

      ////////// Engine.HPP
    
    Editor<Stack>([](Node*node,Stack* stack){


        ImVec2 btn_size = {100,50};
        if (ImGui::Button("layer", btn_size)) {
            node->addOwnr<Layer>();
            
        }
        ImGui::SameLine();
        if (ImGui::Button("uberlayer", btn_size)) {

            node->addOwnr<UberLayer>();

        }

    });

    Editor<Debug>([](Node* node, Debug *debug){

        // Separator();

        // static std::vector<GLenum> enums = {GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_SRC_ALPHA_SATURATE, GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, GL_SRC1_ALPHA, GL_ONE_MINUS_SRC1_ALPHA};
        // static int blendin = 1, blendout = 3;
        // if (ImGui::SliderInt2( "blend", &blendin, 0, enums.size()-1)) glBlendFunc(enums[blendin],enums[blendout]);


        Editor<Log>::cb_typed( node, &logger);

    });
        ////////// Log.HPP

        using namespace ImGui;

    Editor<Log>([&](Node* node, Log *log_n){


        static bool is_verbose = false;

        static ImVec4 info = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
        static ImVec4 debug = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
        static ImVec4 warning = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
        static ImVec4 error = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        static ImVec4 verbose = ImVec4(0.4f, 0.4f, 1.0f, 1.0f);

        static float *curr = &info.x;

        auto curpos = GetCursorPos();
        auto winsize = GetContentRegionAvail();
        auto start = curpos  - ImVec2(255, -5);
        start.x += winsize.x;
        start.y += GetScrollY();


        int max_lines = 200;
        int count = 0;

        static int last_id = 0;

        for (int member_count = log_n->appender.list.size()-1; member_count>=0; member_count-- ) {

            auto &m = log_n->appender.list[log_n->appender.list.size()-1-member_count];

            ImVec4 color = info;

            if (m.severity == Sev::debug) color = debug;
            if (m.severity == Sev::warning) color = warning;
            if (m.severity == Sev::error) color = error;
            if (m.severity == Sev::verbose) {color = verbose; if (!is_verbose) { continue;} }

            count++;
            if (count == max_lines) break;

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            std::string str;
            tm t;
            plog::util::localtime_s(&t, &m.time.time);
            plog::util::nostringstream ss;
            ss <<std::setfill('0') << std::setw(4) << static_cast<int> (m.id) << ":"<< std::setfill('0') << std::setw(2)<<t.tm_sec<<"."<< std::setfill('0') << std::setw(3) << static_cast<int> (m.time.millitm);

            str+="[";
            str+=ss.str();
            str+="] ";

            str += m.msg;

            int x = 130;
            ImGui::Text(str.c_str(), str.c_str()+(str.length()>x?x:str.length())); if (str.length()>x) { ImGui::SameLine(); ImGui::Text("..."); }
            ImGui::PopStyleColor();

            if (ImGui::BeginPopupContextItem(("##dsfgsdf"+std::to_string(count)).c_str())) {

                if (ImGui::Button("copy" )) { ImGui::SetClipboardText(str.c_str()); ImGui::CloseCurrentPopup(); }

                ImGui::EndPopup();

            }
        }

        if (log_n->appender.list.size() && last_id != log_n->appender.list.back().id) {
            SetScrollY(GetScrollMaxY()+((log_n->appender.list.size())*GetCurrentContext()->FontBaseSize*2));
            last_id = log_n->appender.list.back().id;
            
        }
        ImGui::SetCursorPos(start); if (ImGui::Button("clear")) { 
            log_n->appender.list.clear(); 
            }start.x+=50;
        ImGui::SetCursorPos(start); if(ImGui::ColorButton("info", info)){ curr = &info.x; ImGui::OpenPopup("picker");  }start.x+=25;
        ImGui::SetCursorPos(start);if (ImGui::ColorButton("debug", debug)) { curr = &debug.x; ImGui::OpenPopup("picker"); }start.x+=25;
        ImGui::SetCursorPos(start);if (ImGui::ColorButton("warning", warning)) { curr = &warning.x; ImGui::OpenPopup("picker"); }start.x+=25;
        ImGui::SetCursorPos(start);if (ImGui::ColorButton("error", error)) { curr = &error.x; ImGui::OpenPopup("picker"); }start.x+=25;
        ImGui::SetCursorPos(start);if (ImGui::ColorButton("verbose##vcolop", verbose)) { curr = &verbose.x; ImGui::OpenPopup("picker"); }start.x+=25;
        ImGui::SetCursorPos(start); if (ImGui::Checkbox("verbose", &is_verbose)) { SetScrollY(GetScrollMaxY()); } start.x+=25;
        ImGui::SetCursorPos(start); if (ImGui::BeginPopup("picker")) { ImGui::ColorPicker4("#dfsdinfo", curr); ImGui::EndPopup(); }
        ImGui::SetCursorPos(curpos);
    });



    ////////// Effector.HPP

    // Editor<EffectorRef>([](Node* node, EffectorRef *effector){

    //     if (InputInt("wrap", &effector->wrap)) node->update();

    //     for (auto def : effector->definitions) Editor<Effector>::cb( node, def);

    // });

    Editor<Wrappy>([](Node* node, Wrappy *wrap){
        

        for (auto x : wrap->effector_refs) 
            Text(x->effector->name().c_str());

    });
    Editor<Effector>([](Node* node, Effector *def){
        
        auto wrap_ = dynamic_cast<Wrappy*>(def);
        if (wrap_) {
            
            for (auto x : wrap_->effector_refs) ImGui::Text(x->name().c_str());

            return;
            
        }

        static std::map<Effector*,TextEditor> codeeditors;
        auto file_ = dynamic_cast<FileEffector*>(def);
        if (file_) {

            if (codeeditors.find(def) == codeeditors.end()) {

                codeeditors[def].SetShowWhitespaces(false);
                codeeditors[def].SetReadOnly(false);
                codeeditors[def].SetText(file_->file->data.data());
            }

            auto &codeeditor = codeeditors[def];

            codeeditor.Render("codeeditor");

            if (codeeditor.IsTextChanged()) {

                file_->file->data = codeeditor.GetText().c_str();
                file_->load();

                node->update(); // do I need bkp here ? is even the fx useful

            }
            
            return;
            
        }

        DefWidget(def);

    });

    Editor<FileEffector>([](Node* node, FileEffector *e){ Editor<Effector>::cb_typed( node, e); });


    // REMAP.HPP

    // Editor<Universe::Remap>([](Node*node, Universe::Remap* remap){ Editor<Remap>::cb( node, remap); });





    ////////// DRAWCALL.HPP

    Editor<DrawCall>([](Node* node, DrawCall *dc){


        if (ImGui::BeginTabBar("dctqb", ImGuiTabBarFlags_None)) {

            if (ImGui::BeginTabItem("Attribtues")) {

                if (BeginPopupContextItem()) {
                    
                    static std::vector<std::string> BLEND_NAMES = {
                        "GL_ZERO",
                        "GL_ONE",
                        "GL_SRC_COLOR",
                        "GL_ONE_MINUS_SRC_COLOR",
                        "GL_DST_COLOR",
                        "GL_ONE_MINUS_DST_COLOR",
                        "GL_SRC_ALPHA",
                        "GL_ONE_MINUS_SRC_ALPHA",
                        "GL_DST_ALPHA",
                        "GL_ONE_MINUS_DST_ALPHA",
                        "GL_CONSTANT_COLOR",
                        "GL_ONE_MINUS_CONSTANT_COLOR",
                        "GL_CONSTANT_ALPHA",
                        "GL_ONE_MINUS_CONSTANT_ALPHA",
                        "GL_SRC_ALPHA_SATURATE",
                        "GL_SRC1_COLOR",
                        "GL_ONE_MINUS_SRC1_COLOR",
                        "GL_SRC1_ALPHA",
                        "GL_ONE_MINUS_SRC1_ALPHA"

                    };


                    SliderInt(("IN: "+BLEND_NAMES[dc->GL_BLEND_MODE_IN]+"###"+std::to_string(engine.gui_v->member_count++)).c_str(), &dc->GL_BLEND_MODE_IN, 0, BLEND_NAMES.size()-1);
                    SliderInt(("OUT: "+BLEND_NAMES[dc->GL_BLEND_MODE_OUT]+"###"+std::to_string(engine.gui_v->member_count++)).c_str(), &dc->GL_BLEND_MODE_OUT, 0, BLEND_NAMES.size()-1);

                    Text(("instances: " + std::to_string((dc->models.size() == 1 ? dc->models[0]->quantity():1))).c_str());

                    EndPopup();
                }
                
                Editor<Effectable>::cb_typed(node, dc);

                ImGui::EndTabItem();

            }

            if (ImGui::BeginTabItem("ShaderProgram")) {

                Editor<ShaderProgram>::cb_typed( node, &dc->shader);

                ImGui::EndTabItem();

            }

            if (ImGui::BeginTabItem("VBO")) {

                Editor<VBO>::cb_typed( node, &dc->vbo);

                ImGui::EndTabItem();

            }
            ImGui::EndTabBar();
        }


    });


    Editor<UberLayer>([](Node* node, UberLayer *ubl){

        // static uint32_t min = 0, max = 10;;
        // for (auto tex : Texture::pool) 
        //     if (ImGui::SliderScalar(("unit"+std::to_string(tex->id)+" "+tex->sampler_name).c_str(), ImGuiDataType_U32, &tex->unit, &min, &max)) 
        //         tex->bind();
            
        // ImVec2 btn_size = {100,50};
        // if (ImGui::Button("smartlayer", btn_size)) {
        //     node->addPtr<UberLayer::VirtualLayer>(&ubl->addLayer(engine.window.width,engine.window.height));
        //     ubl->calc_matrice();
        //     engine.stack->trig(Node::CHANGE);
        //     ubl->update();
        // }
            
        
        Editor<Layer>::cb_typed( node, ubl);

    });

    Editor<Layer>([](Node* node, Layer *layer){

        if (ImGui::BeginTabBar("laytab", ImGuiTabBarFlags_None)) {

            if (ImGui::BeginTabItem(node->name().c_str())) {
                            
                if (ImGui::BeginPopupContextItem()){
                        
                    ImGui::Text(("FB "+std::to_string(layer->fb.id)).c_str());

                    ImGui::SliderFloat4("clear_color", &layer->fb.clear_color[0], 0, 1);
                    
                    EndPopup();
                
                }

                Editor<FrameBuffer>::cb_typed( node, &layer->fb);

                ImGui::EndTabItem();

            }

            for (auto x : layer->builder()->samplers) {
                
                if (ImGui::BeginTabItem(x.second->sampler_name.c_str())) {

                    Editor<Texture>::cb_typed( node, x.second);

                    ImGui::EndTabItem();

                }

            }

            ImGui::EndTabBar();

        }


        Editor<DrawCall>::cb_typed( node, layer);

    });


}