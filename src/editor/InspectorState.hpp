#ifndef InspectorState_hpp
#define InspectorState_hpp

//#include <cassert>
//#include <array>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h" // ImGui widgets for std::string

namespace Editor {

    // General type inspection template
    template<class T>
    bool inspect_type(T& t, InspectorState& inspector)
    {
        ImGui::Text("Widget not implemented");
        return false;
    }

    // Type inspection specialization for float
    template<>
    inline bool inspect_type<float>(float& t, InspectorState& inspector)
    {
        return ImGui::InputFloat("##label", &t, 1.0f);
    }

    // Type inspection specialization for int
    template<>
    inline bool inspect_type<int>(int& t, InspectorState& inspector)
    {
        return ImGui::InputInt("##label", &t, 1);
    }
    template<>
    inline bool inspect_type<const int>(const int& t, InspectorState& inspector)
    {
        ImGui::TextDisabled("%i", t);
        return false;
    }

    // Type inspection specialization for bool
    template<>
    inline bool inspect_type<bool>(bool& t, InspectorState& inspector)
    {
        return ImGui::Checkbox("##label", &t);
    }

    // Type inspection specialization for std::string
    template<>
    inline bool inspect_type<std::string>(std::string& t, InspectorState& inspector)
    {
        return ImGui::InputText("##label", &t); // label cannot be empty
    }

    // // inspect float
    // template<class T> requires std::is_same_v<T, float>
    // //        requires std::is_floating_point_v<T> // also matches double
    // bool inspect_type(T& t, InspectorState& inspector)
    // {
    //     return ImGui::InputFloat("", &t, 1.0f);
    // }

    /// inspect int
    // template<class T> requires std::is_same_v<T, int>
    // // requires (std::is_integral_v<T> && !std::is_unsigned_v<T>) // also matches e.g. char
    // bool inspect_type(T& t, InspectorState& inspector)
    // {
    //     return ImGui::InputInt("", &t, 1);
    // }

    struct InspectorState
    {
        //entt::entity primary_entity;
        // int id = 0;
        // void push_id()
        // {
        //     ImGui::PushID(id++);
        // }
        // void pop_id()
        // {
        //     ImGui::PopID();
        // }
        void begin_leaf(const char* label)
        {
            row();
            //ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
            ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth);
            next_column();
            // push_id();
            ImGui::PushID(label);
            ImGui::SetNextItemWidth(-FLT_MIN);
        }
        void end_leaf()
        {
            // pop_id();
            ImGui::PopID();
        }
        bool begin_node(const char* label)
        {
            row();
            bool open = ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_SpanFullWidth);
            if (open) {
                next_column();
                // push_id();
                ImGui::SetNextItemWidth(-FLT_MIN);
            }
            return open;
        }
        void end_node()
        {
            ImGui::TreePop();
            // pop_id();
        }
        void row()
        {
            ImGui::TableNextRow();
            next_column();
        }
        void next_column()
        {
            ImGui::TableNextColumn();
        }
    };
} // namespace Editor

#endif // InspectorState_hpp
