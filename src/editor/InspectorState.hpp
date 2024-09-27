#ifndef InspectorState_hpp
#define InspectorState_hpp

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h" // ImGui widgets for std::string

#include <sol/forward.hpp>
// namespace sol {
//     class state;
// }

namespace Editor {

    struct InspectorState;

    /// General type inspection template
    template<class T>
    bool inspect_type(T& t, InspectorState& inspector)
    {
        ImGui::Text("Widget not implemented");
        return false;
    }

    // TODO
    // unsigned char
    // unsigned int

    /// Inspect float
    template<>
    inline bool inspect_type<float>(float& t, InspectorState& inspector)
    {
        return ImGui::InputFloat("##label", &t, 1.0f);
    }

    /// Inspect const float
    template<>
    inline bool inspect_type<const float>(const float& t, InspectorState& inspector)
    {
        ImGui::TextDisabled("%.2f", t);
        return false;
    }

    /// Inspect double
    template<>
    inline bool inspect_type<double>(double& t, InspectorState& inspector)
    {
        return ImGui::InputDouble("##label", &t, 1.0f);
    }

    /// Inspect const double
    template<>
    inline bool inspect_type<const double>(const double& t, InspectorState& inspector)
    {
        ImGui::TextDisabled("%.2f", t);
        return false;
    }

    /// Inspect int
    template<>
    inline bool inspect_type<int>(int& t, InspectorState& inspector)
    {
        return ImGui::InputInt("##label", &t, 1);
    }

    /// Inspect const int
    template<>
    inline bool inspect_type<const int>(const int& t, InspectorState& inspector)
    {
        ImGui::TextDisabled("%i", t);
        return false;
    }

    /// Inspect bool
    template<>
    inline bool inspect_type<bool>(bool& t, InspectorState& inspector)
    {
        return ImGui::Checkbox("##label", &t);
    }

    /// Inspect const bool
    template<>
    inline bool inspect_type<const bool>(const bool& t, InspectorState& inspector)
    {
        ImGui::BeginDisabled();
        bool b = t;
        return ImGui::Checkbox("##label", &b);
        ImGui::EndDisabled();
        return false;
    }

    /// Inspect std::string
    template<>
    inline bool inspect_type<std::string>(std::string& t, InspectorState& inspector)
    {
        return ImGui::InputText("##label", &t); // label cannot be empty
    }

    /// Inspect const std::string
    template<>
    inline bool inspect_type<const std::string>(const std::string& t, InspectorState& inspector)
    {
        ImGui::TextDisabled("%s", t.c_str());
        return false;
    }

    /// @brief 
    struct InspectorState
    {
        sol::state* lua;
        entt::registry* registry;

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
