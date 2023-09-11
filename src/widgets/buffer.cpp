#pragma once
#include "gui.hpp"
#include "buffer.hpp"

  struct Widget : GUI::Window {

        Buffer *buffer;
        int buffer_current = 0, object_current = 0, elem_current = 0;
        int add_comp = -1;
        std::string add_buffer, add_object; 

        StringsBuffer object_list,buffer_list;

        std::function<void()> callback = [](){};

        void updateBufferList() {

            std::vector<std::string> names;
            for (auto buffer:Buffer::pool) { names.push_back(buffer->name); }
            buffer_list.create(names);
            
            if (buffer_current < Buffer::pool.size()) updateObjectList(); 
            
        }

        void updateObjectList() {

            std::vector<std::string> names;
            for (auto &obj:Buffer::pool[buffer_current]->objects) { names.push_back(obj.name); }
            object_list.create(names);

        }

        Widget(std::string name = "Buffer") : GUI::Window(name) { 
            
            if (Buffer::pool.size()) buffer = Buffer::pool[0];
            updateBufferList();
            add_buffer.resize(120);
            add_object.resize(120);
            // update();
        
        }

        void draw() override {

            if (!buffer) return;

            ImGui::InputText(("##New"+name).c_str(), &add_buffer[0], add_buffer.size());
            ImGui::SameLine();
            if (ImGui::Button(("Add "+name).c_str()) && strlen(&add_buffer[0])) { 
                
                new Buffer{add_buffer.c_str()};

                buffer_current = pool.size()-1;  
                
                memset(&add_buffer[0],0,add_buffer.size());

                // Buffer::toJSON();

                updateBufferList(); 
            
            }
            ImGui::Spacing();
            if (ImGui::Combo("list##234sdfgsdfg", &buffer_current, buffer_list.buffer)) updateObjectList();

            if (!Buffer::pool.size()) return;

            ImGui::NewLine();

            ImGui::InputText("##NewObject", &add_object[0], add_object.size());
            ImGui::SameLine();
            if (ImGui::Button("Add Object") && strlen(&add_object[0])) { 
                
                buffer->add(add_object); 

                object_current = buffer->objects.size()-1;  
                
                // memset(&add_object[0],0,add_object.size());

                // UBO::toJSON();
            
            }
            ImGui::Spacing();

            if (!buffer->objects.size()) return;

            ImGui::Combo("object", &object_current, object_list.buffer);

            auto &obj = buffer->objects[object_current];
            
            if (ImGui::InputInt("quantity##objectquantity", &obj.quantity)) {
                
                if (!obj.quantity) {
                    
                    buffer->objects.erase(buffer->objects.begin()+object_current);
                    
                    buffer->updateBuffer();        
                    // UBO::toJSON();
                    return;
                }

                buffer->updateBuffer(); 
                // UBO::toJSON();
                
            }

            ImGui::Spacing();
            
            ImGui::Combo("##AddComponent", &add_comp, Component::buffer_string.buffer);
            ImGui::SameLine();
            if (ImGui::Button("Add Component")) {

                obj.add(Component::pool[add_comp]->name); 

                buffer->updateBuffer(); 
                
                // UBO::toJSON();

            }

            ImGui::NewLine();

            ImGui::ShowDemoWindow();


            if (!obj.components.size()) return;

            //////////////// SINGLE VIEW ////////////////////
            if (ImGui::CollapsingHeader("Single view")) {

                ImGui::NewLine();

                if (ImGui::SliderInt("current##uibocurrent", &elem_current,0,obj.quantity-1)) {}//UBO::toJSON();
            
                int uniform_offset = obj.offset;
                for (auto c:obj.components) {
                    
                    // ImGui::Text((std::to_string(elem_current)+" - "+std::to_string(obj.size)).c_str());
                    ImGui::SeparatorText(c->name.c_str());
                    
                    for (auto m:c->members) {

                        float *value = (float*)&buffer->data[uniform_offset+(elem_current*obj.size)];

                        if (ImGui::SliderFloat((m.name+"##"+c->name+m.name+uid+std::to_string(uniform_offset)).c_str(), value, m.range_from, m.range_to)) { 
                            
                            // ubo->update(); 
                        }

                        uniform_offset += m.size; 
                    }
                    
                    
                        // break; 
                }
            }

            ImGui::NewLine();
            
            //////////////// TABLE VIEW ////////////////////
            if (ImGui::CollapsingHeader("List view", ImGuiTreeNodeFlags_DefaultOpen)) {

                ImGui::NewLine();

                int members_size = 0;
                for (auto &c:obj.components) { members_size+= c->members.size(); }

                if (ImGui::BeginTable("items", obj.components.size(),   ImGuiTableFlags_Borders |  ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX )) {

                    for (auto &c:obj.components) {ImGui::TableSetupColumn(c->name.c_str()); }
                    ImGui::TableHeadersRow();

                    int comp_offset = 0;
                    int members_offset = obj.offset;
                    int col_members_offset = 0;

                    for (auto &c:obj.components) {

                        ImGui::TableNextColumn();
                    
                        if (ImGui::BeginTable("items", c->members.size(), ImGuiTableFlags_RowBg  | ImGuiTableFlags_BordersInner |  ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX )) {
                        
                            for (auto &m:c->members){

                                ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);
                                
                                ImGui::TableSetupColumn(m.name.c_str()); 

                                ImGui::PopStyleVar();

                            }

                            ImGui::TableHeadersRow();

                            for (int id = 0; id < obj.quantity; id++) {
    
                                col_members_offset = 0;

                                ImGui::TableNextRow(ImGuiTableRowFlags_None, 20);
                                for (int i = 0; i < c->members.size(); i++) {
                                    
                                    ImGui::TableNextColumn();
                                    ImGui::SetNextItemWidth(-1);
                                    if (ImGui::SliderFloat(("##tableview"+std::to_string(id*obj.size+members_offset+col_members_offset)).c_str(), (float*)&buffer->data[id*obj.size+members_offset+col_members_offset], 0,1 )) {

                                        buffer->callback();
                                        
                                    }

                                    col_members_offset+=c->members[i].size;

                                }

                            }

                            members_offset+=col_members_offset;

                            ImGui::EndTable();
                            
                        }

                        comp_offset += c->size;

                    }

                    ImGui::EndTable();

                }
            }

            ImGui::NewLine();

            if (ImGui::CollapsingHeader("Raw view")) {

                ImGui::NewLine();

                //////////////// RAW VIEW //////////////////////
                
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2,2));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
                ImGui::SetWindowFontScale(.6666);

                auto window_width = ImGui::GetWindowWidth();

                int cells_count = 48, cell_min = 0, cell_max = 255;
                int cell_width = window_width / cells_count - 2;
                for (int i = 0; i < buffer->data.size(); i++) {

                    ImGui::PushID(i);

                    if (!(i%cells_count)) ImGui::NewLine();
                    ImGui::SameLine(((i%cells_count)*20)+8); 

                    ImGuiDataType_ datatype = ImGuiDataType_U8;

                    if (ImGui::VSliderScalar("",  ImVec2(cell_width,30),    datatype, &buffer->data[i],  &cell_min,   &cell_max,   "")) { 
                        
                        // fixtureUBO->update(); 
                        
                    }

                    ImGui::SameLine(0);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - cell_width) ;
                    ImGui::Text(std::to_string(i).c_str());

                    ImGui::PopID();

                }
                ImGui::SetWindowFontScale(1);

                ImGui::PopStyleVar(5);
                
                ImGui::NewLine();
            
            }

        }

    } ;
    
    // static inline Widget widget();