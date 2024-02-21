#include "editor.hpp"

#include "log.hpp"
#include "node.hpp"
#include "gui.hpp"
#include "buffer.hpp"
#include "directory.hpp"
#include "component.hpp"
#include "struct.hpp"
#include "model.hpp"
#include "file.hpp"
#include "ubo.hpp"
#include "engine.hpp"
#include "drawcall.hpp"
#include "vbo.hpp"
#include "shader.hpp"
#include "effector.hpp"
#include "engine.hpp"
#include "artnet.hpp"

#include <cstring>

static void draw_definition(Object *obj) {

    ImGui::Text(("char[" +std::to_string( obj->buffer->data.size()) + "]").c_str());

    for (auto &o: obj->buffer->objects) {
        
        ImGui::Text(("  "+o.s->name+"[" +std::to_string(o.s->size)+"]"+" * "+std::to_string(o.reserved)).c_str());

        for (auto& c : o.s->comps) {

            ImGui::Text(("    "+c->name+"["+std::to_string(c->size)+"]").c_str());

            for (auto& m : c->members) {
                ImGui::Text(("      "+m.name+"["+std::to_string(m.size)+"]").c_str());                
            }


        }
    }

}

static void draw_raw(void *data, size_t size) {

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2,2));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::SetWindowFontScale(.8);

    auto window_width = ImGui::GetWindowWidth()-15;

    int cell_width = 20;
    int cell_margin = 1;

    int cells_per_line = (float)window_width/(cell_width+cell_margin);

    uint8_t ndata = 0;
    uint8_t cell_min = 0;
    uint8_t cell_max = 255;

    for (int i = 0; i < size; i++) {

        ImGui::PushID(i);

        if (!(i%cells_per_line)) ImGui::NewLine();
        ImGui::SameLine(((i%cells_per_line)*(cell_width+cell_margin))); 

        ImGuiDataType_ datatype = ImGuiDataType_U8;
        
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, ((*(((uint8_t*)data)+i))/255.0f)*30);

        if (ImGui::VSliderScalar("",  ImVec2(cell_width,30),    datatype, &ndata,  &cell_min,   &cell_max,   "")) { 
                    
            
        }
        
        ImGui::PopStyleVar(1);

        ImGui::SameLine(0);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - cell_width) ;
        ImGui::Text(std::to_string(i).c_str());

        ImGui::PopID();

    }
    ImGui::SetWindowFontScale(1);

    ImGui::PopStyleVar(5);

}

static bool draw_object(void*data, Struct* s) {

    int uniform_offset = 0;

    bool has_changed = false;

    for (auto c:s->comps) {
                
        ImGui::SeparatorText(c->name.c_str());
        
        for (auto m:c->members) {

            auto name = (m.name+"##"+c->name+m.name+"uid"+std::to_string(uniform_offset)).c_str();

            float *f = (float*)(((char*)data)+uniform_offset);

            uniform_offset += m.size; 

            if (m.type == Member::Type::VEC2) { ImGui::SliderFloat2(name, f, m.range_from, m.range_to); continue; }
            if (m.type == Member::Type::VEC3) { ImGui::SliderFloat3(name, f, m.range_from, m.range_to); continue; }
            if (m.type == Member::Type::VEC4) { ImGui::SliderFloat4(name, f, m.range_from, m.range_to); continue; }

            auto type = ImGuiDataType_Float;

            if (m.type == Member::Type::UI8) type = ImGuiDataType_U8;
            if (m.type == Member::Type::UI16) type = ImGuiDataType_U16;
            if (m.type == Member::Type::UI32) type = ImGuiDataType_U16;

            if (ImGui::SliderScalar(name, type, data, &m.range_from, &m.range_to))  has_changed = true;
        }
        
    }

    return has_changed;
            
 }


void Editors::init() {

    // ////////// xxx.HPP 

    // Editor<xxx>([](Node* node, xxx *x){ });

    ////////// UBO.HPP 

    Editor<UBO>([](Node* node, UBO *ubo){ 

        ImGui::Text((""+std::to_string(ubo->subscribers.size())+" subs").c_str());

        Editor<Buffer>::cb(node, ubo); 
        
    });

    ////////// VBO.HPP 
    
    Editor<VBO>([](Node*node,VBO*vbo){ Editor<Buffer>::cb(node, vbo); });

    ////////// Artnet.HPP 
    
    Editor<DMX>([](Node*node,DMX* dmx){ 


        // DMX is a univers

        for (auto &r : dmx->remaps) {

             int member_id = 0;

            for (auto c:r.fixture->s->comps) {
                        
                ImGui::SeparatorText(c->name.c_str());
                
                for (auto m:c->members) {

                    ImGui::Text(m.name.c_str());

                    static int e = 0;
                    ImGui::SameLine(); ImGui::RadioButton(("bypass##rbt"+std::to_string(member_id)).c_str(), &r.fixture->attributes[member_id].combining , 0);
                    ImGui::SameLine(); ImGui::RadioButton(("coarse##rbt"+std::to_string(member_id)).c_str(), &r.fixture->attributes[member_id].combining , 1);
                    ImGui::SameLine(); ImGui::RadioButton(("fine##rbt"+std::to_string(member_id)).c_str(), &r.fixture->attributes[member_id].combining , 2);
                    ImGui::SameLine(); ImGui::RadioButton(("ultra##rbt"+std::to_string(member_id)).c_str(), &r.fixture->attributes[member_id].combining , 3);

                    member_id++;

                }
                
            }

        }


    });

    Editor<Artnet>([](Node*node,Artnet* an){ 
            ImGui::Text("yellowp "); 

        for (auto &u : an->universes) {

            ImGui::Text(("u "+std::to_string(u.first)).c_str()); 

            draw_raw(&u.second->data[0], u.second->data.size());

        }


    });

    ////////// VBO.HPP 
    
    Editor<VBO>([](Node*node,VBO*vbo){ Editor<Buffer>::cb(node, vbo); });

    ////////// STRUCT.HPP 

    Editor<Struct>([](Node* node, Struct *s){ 
        
        ImGui::Text((s->name+" " +std::to_string(s->size)).c_str());

        for (auto& c : s->comps) {

            ImGui::Text((" - "+c->name+" "+std::to_string(c->size)).c_str());

            for (auto& m : c->members) {
                ImGui::Text(("  -- "+m.name+" "+std::to_string(m.size)).c_str());                
            }


        }
    
    });

    ////////// SHADER.HPP 

    Editor<ShaderProgram>([](Node* node, ShaderProgram *shader){ 

        static TextEditor frageditor;
        static TextEditor verteditor;

        static bool init = false;
        if (!init){
            frageditor.SetShowWhitespaces(false);
            frageditor.SetReadOnly(false);
            frageditor.SetText(shader->frag.src); 
            init = true;
        }

        static std::chrono::_V2::system_clock::time_point last_change;
        if (last_change != shader->last_change) { 

            frageditor.SetText(shader->frag.src); 
            last_change = shader->last_change;
            
        }
        
	    // auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	    // frageditor.SetLanguageDefinition(lang);

        // frageditor.SetPalette(TextEditor::GetDarkPalette());
        
        frageditor.Render("frageditor");
        // verteditor.Render("frageditor");
        
        if (frageditor.IsTextChanged()) {

            auto x = frageditor.GetText();

            memset(&x[x.size()-2],0,1);

            shader->create(x,shader->vert.src);

        }
   
    });

    ////////// DRAWCALL.HPP 
    
    Editor<DrawCall>([](Node* node, DrawCall *dc){ Editor<ShaderProgram>::cb(node, &dc->shader); });

    ////////// Log.HPP 
    
    Editor<Stack>([](Node* node, Stack *log){ 

        static bool verbose = true;

        ImGui::Checkbox("verbose", &verbose);

        int max_lines = 1000;
        int to = engine.log.appender.list.size()-max_lines;
        if (to<0)to = 0;
        for (int i = engine.log.appender.list.size()-1; i>=to; i-- ) {
        // for (auto &m : engine.log.appender.list ){

            auto &m = engine.log.appender.list[i];

            ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (m.severity == plog::Severity::warning) color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            if (m.severity == plog::Severity::verbose) {
                
                if (!verbose) continue;
                
                color = ImVec4(0.0f, 0.0f, .8f, 1.0f);
                
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text((" "+m.msg).c_str()); 
            ImGui::PopStyleColor();


        }
    });

    ////////// File.HPP 

    Editor<File>([](Node* node, File *file){ 

        char path[512]; 
        memset(path,0,512);
        memcpy(path,file->path.c_str(),file->path.size());

        if (ImGui::InputText("path", path, 512)) {
    
            if (strcmp(path, file->path.c_str())) {
                
                file->read(path);

                if (!file->loaded) node->name = "File";
                else node->name = node->name = file->name+"."+file->extension+"";
                
            }
        
        }
        
        char data[512000]; 
        memset(data,0,512000);
        memcpy(data,file->data.c_str(),file->data.size());

        if(ImGui::InputTextMultiline("src", data, 512000, ImVec2(600,300))) {

            if (strcmp(data, file->data.c_str())) {

                file->write(data); 

                node->update(); 
                
            }

        }

    });

    ////////// Effector.HPP 

    Editor<Effector>([](Node* node, Effector *effector){ Editor<File>::cb(node, effector->file); });

    ////////// MODEL.HPP 

    Editor<Model>([](Node* node, Model *model){ 
        
        if (node->parent()->is_a<DrawCall>()) Editor<Object>::cb(node, model->obj); 
        
        else Editor<File>::cb(node, model->file);
        
    });

    ////////// OBJECT.HPP 

    Editor<Object>([](Node* node, Object *obj){
        
        static int elem_current = 0;
        static std::string uid = "123";

        Buffer* buffer = obj->buffer;

        if (buffer->objects.size()) {

            int max = obj->reserved-1;
            int min = 0;
            if (max<0) min = -1;

            if (ImGui::SliderInt("instance##current", &elem_current, min, max)) { }

            ImGui::SameLine(); if (ImGui::Button("add")) {

                obj->push();

                node->update();

            }

            if (!obj->reserved) return;

            int uniform_offset = obj->offset;

            for (auto c:obj->s->comps) {
                        
                ImGui::SeparatorText(c->name.c_str());
                
                for (auto m:c->members) {

                    auto name = (m.name+"##"+c->name+m.name+uid+std::to_string(uniform_offset)).c_str();

                    auto data = &buffer->data[uniform_offset+(elem_current*obj->s->size)];
                    uniform_offset += m.size; 

                    if (m.type == Member::Type::VEC2) { ImGui::SliderFloat2(name, (float*)data, m.range_from, m.range_to); continue; }
                    if (m.type == Member::Type::VEC3) { ImGui::SliderFloat3(name, (float*)data, m.range_from, m.range_to); continue; }
                    if (m.type == Member::Type::VEC4) { ImGui::SliderFloat4(name, (float*)data, m.range_from, m.range_to); continue; }

                    auto type = ImGuiDataType_Float;

                    if (m.type == Member::Type::UI8) type = ImGuiDataType_U8;
                    if (m.type == Member::Type::UI16) type = ImGuiDataType_U16;
                    if (m.type == Member::Type::UI32) type = ImGuiDataType_U16;

                    if (ImGui::SliderScalar(name, type, data, &m.range_from, &m.range_to))  buffer->update();
                }
                
            }
            
        }

        //// RAW VIEW
        
        // ImGui::Separator();ImGui::Separator(); draw_raw(obj->data(), obj->buffer->data.size());
        
        // ImGui::Separator();ImGui::Separator(); draw_definition(obj);

    });

    ////////// BUFFER.HPP 

    Editor<Buffer>([](Node* node, Buffer *buffer){

        static StringsBuffer object_str;
        static int obj_current = 0;
        std::vector<std::string> obect_strs;
        for (auto &obj : buffer->objects) obect_strs.push_back(obj.s->name);
        object_str.create(obect_strs);
        ImGui::Combo("Buffer##234sdfgsdfg", &obj_current, object_str.buffer);

        Editor<Object>::cb(node, &buffer->objects[obj_current]);

    });

}



/////// EditorWidget.hpp

EditorWidget::EditorWidget() : GUI::Window("Editor")  {  }

void EditorWidget::draw() { 

    if (!selected || !locked) selected = engine.selected;

    std::string name = selected->name;

    if (selected->parent()) { name = selected->parent()->name + "::" + name;  }

    ImGui::Text("%s", name.c_str()); ImGui::SameLine(); ImGui::Checkbox("lock##locked", &locked);

    selected->editor();

}
