#include "editor.hpp"

#include "node.hpp"
#include "gui.hpp"


#include <ctime>
#include <cstring>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "assets/fonts/IconsLucide.h"


/////// EditorWidget.hpp

EditorWidget::EditorWidget(GUI* gui, Node* selected) : GUI::Window("Editor", gui), selected(selected), locked(selected)  { 

 }

EditorWidget::~EditorWidget()  {   }


void EditorWidget::draw() {

    if (ImGui::BeginDragDropTargetCustom(ImRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()),ImGui::GetItemID())) {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENONODE")) {

            auto n = (Node*)(*(uint64_t*)payload->Data);

            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) 
                selected = n;
            else
                if (selected && !selected->add(n)) 
                    for (auto x : selected->childrens)
                        if (x->add(n))
                            break;


        }

        ImGui::EndDragDropTarget();

    }

    if (!selected || !locked) selected = gui->selected;

    if (!selected) 
        return;
        
    bool colored_tab = false;
    if (selected->color != std::array<float,4>{1,1,1,1}) {

        colored_tab = true;   
        ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4{.4,.4,.4,1}*(ImVec4&)selected->color);

    }

    auto w = ImGui::GetWindowWidth(); 
    auto x = 0;ImGui::GetCursorPosX();

    std::string name = selected->nameSTL(1)+ " ("+selected->type_name()+")";

    auto tx = ImGui::CalcTextSize(name.c_str()).x;
    if (ImGui::GetWindowDockNode() && !ImGui::GetWindowDockNode()->IsHiddenTabBar() && ImGui::DockNodeBeginAmendTabBar(ImGui::GetWindowDockNode())) {

        auto x = ImGui::GetCursorPosX();
        auto y = ImGui::GetCursorPosY();

        ImGui::SetCursorPosX(x+w-60);
        ImGui::SetCursorPosY(y-21);  
        auto &io = ImGui::GetIO();
        auto fs = ImGui::GetCurrentWindow()->FontWindowScale;
        ImGui::SetWindowFontScale(fs*1.2);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4,0));
        if (ImGui::Button((std::string(locked?ICON_LC_LOCK_KEYHOLE:ICON_LC_LOCK_KEYHOLE_OPEN)+"##locked"+std::to_string((size_t)this)).c_str())) {
            locked = !locked;
            if (!gui->selected)
                gui->selected = selected;

        }
        ImGui::SetWindowFontScale(fs);
        ImGui::PopStyleVar(1);
        auto offset = x+100;
        ImGui::SetCursorPosX(offset);
        ImGui::SetCursorPosY(y-21);

        ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4&)selected->color);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4(0,0,0,0)));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));

        // set next item width
        if (ImGui::Button(name.c_str(), ImVec2(w-200,0)))
            ImGui::OpenPopup("nodecolorpicker");

        if (ImGui::BeginPopup("nodecolorpicker")) { 
            ImGui::ColorPicker4("#nodecolorpickercolor", &selected->color[0]); 
            ImGui::EndPopup(); 
        }

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(1);

        ImGui::SetCursorPosY(y);

        ImGui::DockNodeEndAmendTabBar();
      }
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    std::string referings;
    for (auto r : selected->referings) if (r) referings += (r->type_name()+"::"+r->name())+", ";
    if (referings.length()) {
        std::string str = "(refering"+std::string(referings.length()>1?"s":"")+": "+referings.substr(0,referings.length()-2)+")";
        ImGui::SameLine(); ImGui::Text(str.c_str()); }
    ImGui::PopStyleColor(1);


    if(EDITOR::ptr.find(selected->stored_type) != EDITOR::ptr.end()) 

        (*(std::function<void(Node*,void*)>*)EDITOR::ptr[selected->stored_type])(selected,selected->void_ptr);

    if (colored_tab)
        ImGui::PopStyleColor(1);

}
