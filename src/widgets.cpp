#include "widgets.hpp"

#include "gui.hpp"
#include "member.hpp"
#include "engine.hpp"
#include "effector.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <cstdint>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#undef IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

#include "ImGuiColorTextEdit/TextEditor.h"

#include "tinyexpr/tinyexpr.h"
#include <cmath>

void ImGui::CustomSliderScalarN(){


}


////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

bool ImGui::DimWiget(uint32_t* x, uint32_t* y, std::string append) {

    PushID(GUI::member_count++);
    
    auto iw = (GetContentRegionAvail().x-121)*.5;
    PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(3,4));

    bool value_changed = false;

    SetCursorPosX(3);SetNextItemWidth(iw); value_changed += DragScalar(("##dimw"+append).c_str(), ImGuiDataType_U32,  x,1.0f, &p_min_1);
    SameLine(); SetNextItemWidth(iw); value_changed += DragScalar(("##dimh"+append).c_str(), ImGuiDataType_U32, y,1.0f, &p_min_1);
    
    SameLine(); if (Button("/2", ImVec2(25,0))) { *x=std::max(1.0f,*x/2.0f); *y=std::max(1.0f,*y/2.0f); value_changed = true; }
    SameLine(); if (Button("x2", ImVec2(25,0))) { *x=*x*2; *y=*y*2; value_changed = true; }
    SameLine(); if (Button("P", ImVec2(25,0))) { value_changed = true; }
    SameLine(); if (Button("D", ImVec2(25,0))) { *x = engine.gui_v->window->displays[0].width; *y = engine.gui_v->window->displays[0].height; value_changed = true; }

    PopStyleVar(1);  

    PopID();

    return value_changed;

}

////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////


void ImGui::TextX(std::string label, int offset, int size, int depth, std::vector<float> range) {


    for (int i = 0; i < depth; i++) label = "    "+label;

    if (range.size()==2) label += " (" +std::format("{}",range[0])+", "+std::format("{}", range[1])+")";

    while (strlen(label.c_str()) < 35) label += " ";
    label += std::to_string(offset);
    while (strlen(label.c_str()) < 40) label += " ";
    label += std::to_string(size);

    ImGui::Text(label.c_str());

    if (ImGui::IsItemHovered()) {

        hovered_offset = offset;
        hovered_size = size;
        is_hovered=true;

    }

};


////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

void ImGui::DefWidget(Member *member_, int offset, int depth) {

    auto member = member_->ref();

    std::vector<float> range;

    if (member->isData()) {

        range.resize(2);

        float from = *(float*)member->from() ;
        float to = *(float*)member->to() ;
        size_t size = sizeof(float);

        if (member->type().id == typeid(int)) {

            from = *(int*)member->from();
            to = *(int*)member->to();
            size = sizeof(int);

        }
        if (member->type().id == typeid(char)) {

            from = *(char*)member->from();
            to = *(char*)member->to();
            size = sizeof(char);

        }

        memcpy(&range[0], &from, size);

        memcpy(&range[1], &to, size);

    }

    ImGui::TextX(std::string(!member->isData() ? "struct" : member->type_name()) + " " + member->name(), offset, member->footprint(), depth,range);

    if (member->force_ref)
        member = member->members[0];
    for (auto m : member->members) {

        DefWidget(m, offset, depth+1);

        offset += m->footprint_all();
    }

    if (member->stride()) ImGui::TextX("float stride", offset, member->stride(), depth+1);

}

////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

bool ImGui::VCharSlider(void* c, int label_id) {

        static const char* format = "%d";
        static ImVec2 size = ImVec2(22,30);
        static uint8_t min = 0;
        static uint8_t max = 255;
        static const char* label_ = "##vcs";
        std::string label = label_+std::to_string(label_id);

        using namespace ImGui;


        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label.c_str());

        const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
        const ImRect bb(frame_bb.Min, frame_bb.Max);

        const bool temp_input_allowed = 1;
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(frame_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0)) return false;

        // const bool hovered = ItemHoverable(frame_bb, id);
        // const bool clicked = hovered && IsMouseClicked(0, id);
        // if (clicked || g.NavActivateId == id)
        // {
        //     if (clicked)
        //         SetKeyOwner(ImGuiKey_MouseLeft, id);
        //     SetActiveID(id, window);
        //     SetFocusID(id, window);
        //     FocusWindow(window);
        //     g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
        // }

        const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
        bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
        if (!temp_input_is_active)
        {
            // Tabbing or CTRL-clicking on Slider turns it into an input box
            const bool clicked = hovered && IsMouseClicked(0, ImGuiInputFlags_None, id);
            const bool make_active = ( clicked || g.NavActivateId == id);
            if (make_active && clicked)
                SetKeyOwner(ImGuiKey_MouseLeft, id);
            if (make_active && temp_input_allowed)
                if ( (clicked && g.IO.KeyCtrl) || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
                    temp_input_is_active = true;

            if (make_active && !temp_input_is_active)
            {
                SetActiveID(id, window);
                SetFocusID(id, window);
                FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            }
        }

        if (temp_input_is_active)
        {
            // Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
            const bool is_clamp_input = ( ImGuiSliderFlags_AlwaysClamp) != 0;


            return TempInputScalar(frame_bb, id, label.c_str(), ImGuiDataType_U8, c, format, &min, &max);

        }


        PushStyleVar(ImGuiStyleVar_GrabMinSize, 0);

        PushID(GUI::member_count++);


        // Draw frame
        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

        // Slider behavior
        ImRect grab_bb;
        const bool value_changed = SliderBehavior(frame_bb, id, ImGuiDataType_U8, c, &min, &max, "",  ImGuiSliderFlags_Vertical, &grab_bb);

        grab_bb.Max.y=frame_bb.Max.y-2;
        // grab_bb.Min.y+=style.GrabMinSize;

        if (value_changed)
            MarkItemEdited(id);

        // Render grab
        if (grab_bb.Max.y > grab_bb.Min.y)
            window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

        PushStyleColor(ImGuiCol_Text, ImVec4(.05,.05,.05,1));
        std::string  lid = std::to_string(label_id);
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, &(*lid.begin()), &(*lid.end()), NULL, ImVec2(0.5f, 0.0f));
        PopStyleColor();

        // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        // For the vertical slider we allow centered text to overlap the frame padding
        char value_buf[64];
        const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), ImGuiDataType_U8, c, format);
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y+14), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.0f));


        PopID();
        PopStyleVar(1);

        return value_changed;
    }


////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////

bool ImGui::RawWidget(void *data, size_t size) {

    bool has_changed = false;

    ImGui::NewLine();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2,2));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

    ImGui::SetWindowFontScale(.8);

    auto window_width = ImGui::GetWindowWidth()-15;

    int cell_width = 24;
    int cell_margin = 1;

    int cells_per_line = (float)window_width/(cell_width+cell_margin);

    bool colorized = false;

    // ImGui::BeginDisabled();

         if (size > 1025) size = 1025;

        if (cells_per_line) 
            
            for (int i = 0; i < size; i++) {


                // ImGui::PushID(i);
                if (ImGui::IsWindowHovered() && is_hovered && i == hovered_offset) {

                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(.5,0,0,1));

                    colorized = true;

                }

                if (i && !(i%cells_per_line)) ImGui::NewLine();
                ImGui::SameLine(((i%cells_per_line)*(cell_width+cell_margin)));

                if (VCharSlider((((uint8_t*)data)+i),i)) has_changed = true;

                // ImGui::PopID();
                if (colorized && (i == hovered_offset+hovered_size-1 || i == size-1)) { ImGui::PopStyleColor(); colorized= false; }

            }

    // ImGui::EndDisabled();

    ImGui::SetWindowFontScale(1);

    ImGui::PopStyleVar(5);

    static int all_at = 0;
    static std::vector<char> bkp;
    
    bool has_changed_all = false;
    
    if (Button("ALL @ 63###allat63"))  { all_at = 63; has_changed_all = true; }
    SameLine(); SetNextItemWidth(50);
    if (DragInt("##all_at",&all_at,1,0,255)) has_changed_all = true; 

    if (has_changed_all) {

        if (!bkp.size()){
            
            bkp.resize(size);

            memcpy(bkp.data(), data, size);
        
        }
        
        memset(data, all_at, size);

        has_changed = true;
        
    }

    if (bkp.size()) {
    
        SameLine();

        if (Button("reset##rereresset"))  { 

            memcpy(data, bkp.data(), size>bkp.size()?bkp.size():size);

            bkp.resize(0);

            has_changed = true;

        }

    }

    ImGui::NewLine();

    return has_changed;

}




////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////


static int MyResizeCallback(ImGuiInputTextCallbackData* data) {

    bool change = !strcmp(((std::string*)data->UserData)->c_str(), (const char*)data->Buf);

    *(std::string*)data->UserData = data->Buf;
    
    return change;

}

std::array<bool,2> ImGui::SlidersWidget(Member* buff, Member* member, uint32_t offset) {

    if (!member)
        member = buff; 
    
    struct int_ { int val = 0; };
    static std::map<Member*,int_> elem_currents;
    int &elem_current = elem_currents[member].val;

    if (member->quantity() > 1 && member != buff) {

        SameLine();

        SetNextItemWidth(-FLT_MIN-10);
        if (ImGui::SliderInt(("##"+std::to_string(GUI::member_count++)).c_str(), &elem_current, 0, member->quantity()-1)) { }

        offset += member->footprint()*elem_current;

    }

    member = member->ref();

    bool val_changed = false;
    bool def_changed = false;

    // if (member->ref()) member = member->ref();

    if (member->force_ref)
        member = member->members[0];
    for (auto& m : member->members) {

        if (m->isData()) {

            auto type = ImGuiDataType_Float;

            if (m->type().id == typeid(int)) 
                type = ImGuiDataType_U32;

            std::string name = (m->name());


            static std::string str__;
            static std::string last_value;

            int q = m->quantity();


            // SetCursorPosX(85-CalcTextSize(name.c_str()).x);
            SetCursorPosX(5);

            SetNextItemWidth(-FLT_MIN-6);
            PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            bool dis = false;
            bool blo = false;
            if (*(float*)m->to() == 0){
                ImGui::BeginDisabled();
                blo = true;
            }
            if (*(float*)m->to() == *(float*)m->from()){
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.0f, 0.0f, 0.0f));
                dis = true;
            }

            auto x = ImGui::SliderScalarN(
                ("###"+std::to_string(GUI::member_count++)).c_str(), 
                type, buff->data()+offset, q, m->from(), m->to(),
                NULL,0,ImGuiInputTextFlags_CallbackAlways|ImGuiInputTextFlags_EnterReturnsTrue,MyResizeCallback, &str__); 

            if (x) {

                if (str__.length()) {

                    boost::trim_left(str__);

                    if (str__[0] == '+' || str__[0] == '*' || str__[0] == '/' || str__[0] == '-') {
                        
                        if (m->type().id == typeid(float))
                            str__ = std::to_string(*(float*)(buff->data()+offset))+str__;
                        else if (m->type().id == typeid(uint32_t))
                            str__ = std::to_string(*(uint32_t*)(buff->data()+offset))+str__;
                        else if (m->type().id == typeid(int32_t))
                            str__ = std::to_string(*(int32_t*)(buff->data()+offset))+str__;

                    }
                    
                    static te_parser tep;

                    double r = tep.evaluate(str__);

                    str__.clear();

                    if (!std::isnan(r)){

                        if (type == ImGuiDataType_Float) 
                            *(float*)(buff->data()+offset) = r;
                        else if (type == ImGuiDataType_S16) *(int16_t*)(buff->data()+offset) = r;
                        else if (type == ImGuiDataType_U16) *(uint16_t*)(buff->data()+offset) = r;

                    }
                
                }

                val_changed = true;
            }


            if (dis)
                ImGui::PopStyleColor(1);

            if (blo)
                ImGui::EndDisabled();

            if (!(IsItemActive() && ImGui::TempInputIsActive(ImGui::GetCurrentContext()->LastItemData.ID))){
                
                SameLine(); 
                SetNextItemWidth(85);
                PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, .65f));
                ImGui::SetCursorPosX(11);
                Text(name.c_str());
                PopStyleColor();
                
            }

            PopStyleColor();

            if (ImGui::IsItemClicked()) {

                    if (type == ImGuiDataType_Float) last_value = std::to_string(*(float*)(buff->data()+offset+(int)std::floor( ( ImGui::GetMousePos().x - ImGui::GetWindowPos().x ) / ImGui::GetItemRectSize().x * q )*4));
                    else if (type == ImGuiDataType_S16) last_value = std::to_string(*(int16_t*)(buff->data()+offset+(int)std::floor( ( ImGui::GetMousePos().x - ImGui::GetWindowPos().x ) / ImGui::GetItemRectSize().x * q )*4));
                    else if (type == ImGuiDataType_U16) last_value = std::to_string(*(uint16_t*)(buff->data()+offset+(int)std::floor( ( ImGui::GetMousePos().x - ImGui::GetWindowPos().x ) / ImGui::GetItemRectSize().x * q )*4));

            }


        }else{

            std::string septxt  = m->ref()->name();


            PushStyleColor(ImGuiCol_Text, ImVec4(0.6, 0.6, 0.6, 1.0f));
            ImGui::SeparatorText(septxt.c_str());
            PopStyleColor(1);

            if (m == m->ref() ){

                SameLine();
                SetNextItemWidth(150);

                int qq = m->quantity();
                if (IntButtons(&qq)) { 

                    if (!qq) {
                        
                    // engine.gui_v->deleteNode(node);
                    } 
                    
                    else{
            
                        m->quantity(qq); 
                    
                        def_changed = true; 
                    
                    }

                }
            }

            auto recurse = SlidersWidget(buff, m, offset);
            if (recurse[0]) val_changed = true;
            if (recurse[1]) def_changed = true;

            // ImGui::Text("delete");
            // if(ImGui::IsItemClicked()){
            //     // s->remove(m->name()); // TOdoFIX
            //     has_changed = true;
            // }

        }

        offset+=m->footprint_all();

    }


    return {val_changed,def_changed};

 }




////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////






bool ImGui::IntButtons(int* p_data ) {

        bool value_changed = false;

        int p_step_ = 1;
        int p_step_fast_ = 100;
        void* p_step = (void*)&p_step_;
        void* p_step_fast = (void*)&p_step_fast_;

        const float button_size = GetFrameHeight();
        auto style = ImGui::GetStyle();
        BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
        PushID(engine.gui_v->member_count++);
        SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
        

        auto io = ImGui::GetIO();

        ImGuiInputTextFlags flags;
        
        const ImVec2 backup_frame_padding = style.FramePadding;
        style.FramePadding.x = style.FramePadding.y;
        ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;

        SameLine(0, style.ItemInnerSpacing.x);

        static std::set<int*> deletings;
        bool deleting = deletings.find(p_data)!=deletings.end();
        if (ButtonEx(deleting?"?":"-", ImVec2(button_size, button_size), button_flags)) {
            
            if (*p_data == 1 && !deleting) 
                deletings.insert(p_data);
                    
            else{

                deletings.erase(p_data);
            
                DataTypeApplyOp(ImGuiDataType_S32, '-', p_data, p_data, io.KeyCtrl && p_step_fast ? p_step_fast : p_step);
            
                value_changed = true;
                
            }
            
        }

        
        SameLine(0, style.ItemInnerSpacing.x);
        if (ButtonEx("+", ImVec2(button_size, button_size), button_flags))
        {
            deletings.erase(p_data);
            
            if (!deleting){
                
                DataTypeApplyOp(ImGuiDataType_S32, '+', p_data, p_data, io.KeyCtrl && p_step_fast ? p_step_fast : p_step);
                
                value_changed = true;
            
            }
        }

        style.FramePadding = backup_frame_padding;

        PopID();
        EndGroup();

        return value_changed;

}




////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////



struct ImEditor {

    // static inline std::vector<const char*,bool> fasddsf;

    TextEditor editor;

    const char* src;

    bool has_changed = false;

    ImEditor() {

        editor.SetShowWhitespaces(false);

        editor.SetReadOnly(false);

    }

};









////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////
////////////////////////////////////////////