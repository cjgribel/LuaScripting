#ifndef InspectorState_hpp
#define InspectorState_hpp

// #include <entt/entt.hpp>
// #include <entt/fwd.hpp>
// #include <sol/forward.hpp>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h" // ImGui widgets for std::string

#include "Context.hpp"
#include "CommandQueue.hpp"

namespace Editor {

    /// @brief 
    struct InspectorState
    {
        Context context;
        std::weak_ptr<CommandQueue> cmd_queue;

        bool imgui_disabled = false;
        entt::entity selected_entity = entt::null;

        // int id = 0;
        // void push_id()
        // {
        //     ImGui::PushID(id++);
        // }
        // void pop_id()
        // {
        //     ImGui::PopID();
        // }

        void begin_disabled()
        {
            if (!imgui_disabled)
            {
                ImGui::BeginDisabled();
                imgui_disabled = true;
            }
        }

        void end_disabled()
        {
            if (imgui_disabled)
            {
                ImGui::EndDisabled();
                imgui_disabled = false;
            }
        }

        bool is_disabled() const { return imgui_disabled; }

        void begin_leaf(const char* label, bool selected = false)
        {
            row();
            //ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
            //ImGui::TreeNodeEx()
            ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_Leaf | /*ImGuiTreeNodeFlags_Bullet | */ImGuiTreeNodeFlags_NoTreePushOnOpen * 0 | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Selected * selected);
            next_column();
            // push_id();
            // ImGui::PushID(label);
            ImGui::SetNextItemWidth(-FLT_MIN);
        }
        void end_leaf()
        {
            ImGui::TreePop(); // if no ImGuiTreeNodeFlags_NoTreePushOnOpen
            // pop_id();
            // ImGui::PopID();
        }
        bool begin_node(const char* label)
        {
            row();
            bool open = ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_SpanFullWidth /*| ImGuiTreeNodeFlags_Selected*/);
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
