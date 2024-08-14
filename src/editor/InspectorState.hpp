#ifndef InspectorState_hpp
#define InspectorState_hpp

//#include <cassert>
//#include <array>
#include "imgui.h"

namespace Editor {

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
            ImGui::SetNextItemWidth(-FLT_MIN);
        }
        void end_leaf()
        {
            // pop_id();
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
