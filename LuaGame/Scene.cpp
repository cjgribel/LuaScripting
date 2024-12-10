
#include <thread>
#include <chrono>

#include <algorithm>
#include <execution>

//#include <nlohmann/json.hpp> // to serialize 

#include "imgui.h"
#include "mat.h"

#include "MetaInspect.hpp"
#include "MetaClone.hpp"
#include "MetaSerialize.hpp"

#include "Scene.hpp"

#include "bond.hpp"
// #include "transform.hpp"
//#include "kbhit.hpp"
#include "AudioManager.hpp"
#include "CoreComponents.hpp"
#include "DebugClass.h"

#include "InspectType.hpp"
// #include "BehaviorScript.hpp"

#include "GuiCommands.hpp" // -> GUI

//#include "meta_aux.h" // enum -> string

#define AUTO_ARG(x) decltype(x), x
using namespace linalg;

namespace LuaBindings
{
    // ...
}

namespace {

    // void inspect_script(const ScriptedBehaviorComponent& script)
    // {
    //     script.self.for_each([](const sol::object& key, const sol::object& value)
    //         { std::cout << key.as<std::string>() << ": "
    //         << sol::type_name(value.lua_state(), value.get_type())
    //         << std::endl; });
    // }

// Can be done genercially for basic types since they have no data fields,
// and must have certain meta function.
// Component types or other class types will have data feilds,
// and may or may not have any meta functions
//
// These give the same value
// std::cout << entt::type_id<int>().hash() << std::endl;
// std::cout << entt::type_hash<int>::value() << std::endl;
//
    template<class T>
    void register_basic_type()
    {
        const auto to_json = [](nlohmann::json& j, const void* ptr)
            {
                j = *static_cast<const T*>(ptr);
            };
        const auto from_json = [](const nlohmann::json& j, void* ptr, entt::entity entity, Editor::Context& context)
            {
                *static_cast<T*>(ptr) = j;
            };
        const auto inspect = [](void* ptr, Editor::InspectorState& inspector) -> bool
            {
                return Editor::inspect_type(*static_cast<T*>(ptr), inspector);
            };

        entt::meta<T>()
            .type(entt::type_hash<T>::value())

            // To/from JSON
            .template func<to_json, entt::as_void_t>(to_json_hs)
            .template func<from_json, entt::as_void_t >(from_json_hs)

            // Inspection
            .template func<inspect>(inspect_hs);

        // To string
        if constexpr (std::is_same_v<T, std::string>)
        {
            entt::meta<T>()
                .template func < [](const void* ptr) -> std::string { return *static_cast<const T*>(ptr); } > (to_string_hs);
        }
        else
        {
            entt::meta<T>()
                .template func < [](const void* ptr) -> std::string { return std::to_string(*static_cast<const T*>(ptr)); } > (to_string_hs);
        }
    }

    void bindAudioManager(auto& lua)
    {
        auto& audioManager = AudioManager::getInstance();

        lua->template new_usertype<AudioManager>("AudioManager",
            // "init", [&audioManager]() {
            //     return audioManager.init();
            // },
            "registerEffect", [&audioManager](AudioManager&, const std::string& name, const std::string& path) {
                return audioManager.registerEffect(name, path);
            },
            "registerMusic", [&audioManager](AudioManager&, const std::string& name, const std::string& path) {
                return audioManager.registerMusic(name, path);
            },
            "playEffect", [&audioManager](AudioManager&, const std::string& name, int loops) {
                audioManager.playEffect(name, loops);
            },
            "playMusic", [&audioManager](AudioManager&, const std::string& name, int loops) {
                audioManager.playMusic(name, loops);
            },
            "pauseMusic", [&audioManager](AudioManager&) {
                audioManager.pauseMusic();
            },
            "resumeMusic", [&audioManager](AudioManager&) {
                audioManager.resumeMusic();
            },
            "fadeInMusic", [&audioManager](AudioManager&, const std::string& name, int loops, int ms) {
                audioManager.fadeInMusic(name, loops, ms);
            },
            "fadeOutMusic", [&audioManager](AudioManager&, int ms) {
                audioManager.fadeOutMusic(ms);
            },
            "isMusicPlaying", [&audioManager](AudioManager&) {
                return audioManager.isMusicPlaying();
            },
            "setEffectVolume", [&audioManager](AudioManager&, const std::string& name, int volume) {
                audioManager.setEffectVolume(name, volume);
            },
            "setMusicVolume", [&audioManager](AudioManager&, const std::string& name, int volume) {
                audioManager.setMusicVolume(name, volume);
            },
            "setMasterVolume", [&audioManager](AudioManager&, int volume) {
                audioManager.setMasterVolume(volume);
            },
            "removeEffect", [&audioManager](AudioManager&, const std::string& name) {
                audioManager.removeEffect(name);
            },
            "removeMusic", [&audioManager](AudioManager&, const std::string& name) {
                audioManager.removeMusic(name);
            },
            "clear", [&audioManager](AudioManager&) {
                audioManager.clear();
            }
            // "destroy", [&audioManager](AudioManager&) {
            //     audioManager.destroy();
            // }
        );

        lua->operator[]("engine")["audio"] = &audioManager;
    }

    void registerQuadComponent(auto& lua)
    {
        lua->template new_usertype<QuadComponent>("QuadComponent",
            "type_id",
            &entt::type_hash<QuadComponent>::value,
            sol::call_constructor,
            sol::factories([](float w, uint32_t color, bool is_visible) {
                return QuadComponent{ w, color, is_visible };
                }),
            "w",
            &QuadComponent::w,
            "color",
            &QuadComponent::color,
            "is_visible",
            &QuadComponent::is_visible,
            sol::meta_function::to_string,
            &QuadComponent::to_string
        );
    }

    void registerCircleColliderComponent(auto& lua)
    {
        lua->template new_usertype<CircleColliderComponent>("CircleColliderComponent",
            "type_id",
            &entt::type_hash<CircleColliderComponent>::value,
            sol::call_constructor,
            sol::factories([](float r, bool is_active) {
                return CircleColliderComponent{ r, is_active };
                }),
            "r",
            &CircleColliderComponent::r,
            "is_active",
            &CircleColliderComponent::is_active,
            sol::meta_function::to_string,
            &CircleColliderComponent::to_string
        );
    }

    // void registerScriptedBehaviorComponent(sol::state& lua)
    // {
    //     lua.new_usertype<ScriptedBehaviorComponent>("ScriptedBehaviorComponent",
    //         "type_id",
    //         &entt::type_hash<ScriptedBehaviorComponent>::value,
    //         sol::call_constructor,
    //         sol::factories([]() {
    //             return ScriptedBehaviorComponent{ };
    //             }),
    //         // "scripts",
    //         // &ScriptedBehaviorComponent::scripts,
    //         // "get_script_by_id",
    //         // &ScriptedBehaviorComponent::get_script_by_id,
    //         sol::meta_function::to_string,
    //         &ScriptedBehaviorComponent::to_string
    //     );
    // }

    // void init_script(entt::registry& registry, entt::entity entity)
    // {
    //     auto& script_comp = registry.get<ScriptedBehaviorComponent>(entity);
    //     for (auto& script : script_comp.scripts)
    //     {
    //         assert(script.self.valid());
    //         script.update = script.self["update"];
    //         assert(script.update.valid());

    //         // -> entityID?
    //         script.self["id"] = sol::readonly_property([entity]
    //             { return entity; });
    //         // -> registry?
    //         script.self["owner"] = std::ref(registry);

    //         if (auto&& f = script.self["init"]; f.valid())
    //             f(script.self);
    //         // inspect_script(script);
    //     }
    // }


    void dump_lua_state(auto& lua, sol::table tbl, const std::string& indent = "")
    {
        for (auto& kv : tbl)
        {
            sol::object key = kv.first;
            sol::object value = kv.second;

            // Assuming the key is a string for simplicity
            std::string key_str = key.as<std::string>();

            std::string type_name = lua_typename(lua->lua_state(), static_cast<int>(value.get_type()));

            std::cout << indent << key_str << " (type: " << type_name << ")\n";

            if (value.get_type() == sol::type::table) {
                dump_lua_state(lua, value.as<sol::table>(), indent + "    ");
            }
            else if (value.get_type() == sol::type::function) {
                std::cout << indent << "    [function]\n";
            }
            else {
                std::cout
                    << indent
                    << "    ["
                    << lua->operator[]("tostring")(value).template get<std::string>()
                    << "]\n";
            }
        }
    }

    void release_script(entt::registry& registry, entt::entity entity)
    {
        auto& script_comp = registry.get<ScriptedBehaviorComponent>(entity);
        for (auto& script : script_comp.scripts)
        {
            // auto& script = registry.get<ScriptedBehaviorComponent>(entity);
            if (auto&& f = script.self["destroy"]; f.valid())
                f(script.self);
            script.self.abandon();
        }
    }

    /// @brief Run update for all scripts
    /// @param registry 
    /// @param delta_time 
    /// Entities inside scripts that need to be destroyed are flagged for 
    /// destruction and then destroyed after this function. Destroying entities
    /// inside a view (i.e. inside scripts) while the view is iterated leads to
    /// undefined behavior: 
    /// https://github.com/skypjack/entt/issues/772#issuecomment-907814984
    void script_system_update(auto& registry, float delta_time)
    {
        //std::cout << "update" << std::endl;
        auto view = registry->template view<ScriptedBehaviorComponent>();
        for (auto entity : view)
        {
            assert(entity != entt::null);
            assert(registry->valid(entity));

            auto& script_comp = view.template get<ScriptedBehaviorComponent>(entity);
            for (auto& script : script_comp.scripts)
            {
                assert(script.self.valid());
                // std::cout << script.identifier << std::endl;
                script.update(script.self, delta_time);
            }
        }
    }

    void update_input_lua(auto& lua, const v4f& axes, vec4<bool> buttons)
    {
        auto input = lua->operator[]("engine")["input"];
        input["axis_left_x"] = axes.x;
        input["axis_left_y"] = axes.y;
        input["axis_right_x"] = axes.z;
        input["axis_right_y"] = axes.w;
        input["button_a"] = buttons.x;
        input["button_b"] = buttons.y;
        input["button_x"] = buttons.z;
        input["button_y"] = buttons.w;

        // lua["engine"]["input"]["axis_left_x"] = axes.x;
        // lua["engine"]["input"]["axis_left_y"] = axes.y;
        // lua["engine"]["input"]["axis_right_x"] = axes.z;
        // lua["engine"]["input"]["axis_right_y"] = axes.w;
        // lua["engine"]["input"]["button_a"] = buttons.x;
        // lua["engine"]["input"]["button_b"] = buttons.y;
        // lua["engine"]["input"]["button_x"] = buttons.z;
        // lua["engine"]["input"]["button_y"] = buttons.w;
    }

    // Called from Lua
    // Adds a behavior script to the ScriptedBehaviorComponent of an entity
    // Adds entity & registry to the script ('id', 'owner')
    // 
    sol::table add_script(
        entt::registry& registry,
        entt::entity entity,
        const sol::table& script_table,
        const std::string& identifier,
        const std::string& script_path)
    {
        std::cout << "add_script " << identifier << " entity " << (uint32_t)entity << std::endl;
        //return;
        assert(script_table.valid());

        BehaviorScript script;

        script.self = script_table;

        script.update = script.self["update"];
        assert(script.update.valid());

        script.on_collision = script.self["on_collision"];
        assert(script.on_collision.valid());

        script.identifier = identifier;
        script.path = script_path;

        // -> entityID?
        script.self["id"] = sol::readonly_property([entity] { return entity; });
        // -> registry?
        script.self["owner"] = std::ref(registry); // &registry also seems to work

        // Run script init()
        // if (auto&& f = script.self["init"]; f.valid())
        //     f(script.self);
        // inspect_script(script);
        assert(script.self["init"].valid());
        script.self["init"](script.self);

        // Add script to the list of scripts
        auto& script_comp = registry.get_or_emplace<ScriptedBehaviorComponent>(entity);
        script_comp.scripts.push_back(script); // Invokes copy-ctor

        // Print the table's contents
#if 0
        std::cout << "Lua table contents:" << std::endl;
        for (auto& pair : script_table) {
            sol::object key = pair.first;
            sol::object value = pair.second;
            std::cout << "Key: " << key.as<std::string>() << ", Value: ";
            if (value.is<std::string>()) {
                std::cout << value.as<std::string>() << std::endl;
            }
            else if (value.is<int>()) {
                std::cout << value.as<int>() << std::endl;
            }
            else {
                std::cout << "Unknown type" << std::endl;
            }
        }
#endif
        // return script.self;
        return script_comp.scripts.back().self;
    }

    sol::table add_script_from_file(
        auto& registry,
        entt::entity entity,
        auto& lua,
        const std::string& script_dir,
        const std::string& script_name
    )
    {
        const std::string script_path = script_dir + script_name + ".lua";
        // sol::load_result behavior_script = lua.load_file(script_file);
        sol::load_result behavior_script = lua->load_file(script_path);

        assert(behavior_script.valid());
        sol::protected_function script_function = behavior_script;
        return add_script(registry, entity, script_function(), script_name, script_path);
    }

    sol::table get_script(entt::registry& registry, entt::entity entity, const std::string& identifier)
    {
        if (!registry.all_of<ScriptedBehaviorComponent>(entity))
            return sol::lua_nil;

        auto& script_comp = registry.get<ScriptedBehaviorComponent>(entity);

        for (auto& script : script_comp.scripts)
        {
            if (script.identifier == identifier)
            {
                return script.self;
            }
        }
        //assert(0);
        return sol::lua_nil;
    }

    void update_IslandFinderComponent(
        IslandFinderComponent& grid_comp,
        const CircleColliderGridComponent& c)
    {
        const int core_x = grid_comp.core_x;
        const int core_y = grid_comp.core_y;
        const int w = c.width;
        const int h = c.element_count / w;
        auto& q = grid_comp.visit_queue;
        auto& v = grid_comp.visited;
        auto& islands = grid_comp.islands;
        //const auto& is_active = colliderset_comp.is_active_flags;

        v.assign(w * h, false);
        islands.clear();

        // Core is inactive => mark all nodes as islands
        // if (!colliderset_comp.is_active_flags[core_y * w + core_x])
        // {
        //     for (int i = 0; i < w * h; i++)
        //     {
        //         if (is_active[i])
        //             islands.push_back(i);
        //     }
        //     return;
        // }
        if (!c.active_indices.contains(core_y * w + core_x))
        {
            for (int di = 0; di < c.active_indices.get_dense_count(); di++)
            {
                // if (c. is_active[i])
                int i = c.active_indices.get_dense(di);
                islands.push_back(i);
            }
            return;
        }

        q.push({ core_x, core_y });
        v[core_y * w + core_x] = true;

        while (!q.empty())
        {
            auto [cx, cy] = q.front();
            q.pop();

            int directions[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };
            for (auto& dir : directions)
            {
                int nx = cx + dir[0];
                int ny = cy + dir[1];

                if (nx >= 0 &&
                    ny >= 0 &&
                    nx < w &&
                    ny < h &&
                    !v[ny * w + nx] &&
                    c.active_indices.contains(ny * w + nx)
                    //is_active[ny * w + nx]
                    )
                {
                    v[ny * w + nx] = true;
                    q.push({ nx, ny });
                }
            }
        }

        // Mark all unvisited active nodes as islands
        // for (int i = 0; i < w * h; i++)
        // {
        //     if (is_active[i] && !v[i])
        //         islands.push_back(i);
        // }
        for (int di = 0; di < c.active_indices.get_dense_count(); di++)
        {
            int i = c.active_indices.get_dense(di);
            if (!v[i])
                islands.push_back(i);
        }
    }

    void IslandFinderSystem(auto& registry, float delta_time)
    {
        auto view = registry->template view<IslandFinderComponent>();
#ifdef EENG_COMPILER_CLANG
        for (auto entity : view)
        {
            auto& grid_comp = view.template get<IslandFinderComponent>(entity);
            auto& colliderset_comp = registry->template get<CircleColliderGridComponent>(entity);
            update_IslandFinderComponent(grid_comp, colliderset_comp);
        }
#else
        std::for_each(std::execution::par,
            view.begin(),
            view.end(),
            [&](auto entity) {
                auto& grid_comp = view.get<IslandFinderComponent>(entity);
                auto& colliderset_comp = registry.get<CircleColliderGridComponent>(entity);
                update_IslandFinderComponent(grid_comp, colliderset_comp);
            });
#endif
    }
}

// ImGui -> Lua integration
namespace {
    void ImGui_Text(const std::string& text)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* customFont = io.Fonts->Fonts[1];
        ImGui::PushFont(customFont);

        ImGui::TextUnformatted(text.c_str());

        ImGui::PopFont();
    }

    void ImGui_SetNextWindowPos(float x, float y) {
        ImVec2 pos(x, y);
        ImGui::SetNextWindowPos(pos);
    }

    void ImGui_Begin(const char* name)
    {
        const auto flags =
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoScrollWithMouse
            | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoBackground
            | ImGuiWindowFlags_NoDecoration;

        ImGui::Begin(name, nullptr, flags);
    }

    void ImGui_End()
    {
        ImGui::End();
    }
}

// Bind the ConditionalObserver to Lua
/*
Example sending Lua event from core to Lua

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    // Create the observer instance
    ConditionalObserver observer;

    // Bind the observer to Lua
    bind_conditional_observer(lua, observer);

    // Load and run a Lua script to register callbacks
    lua.script(R"(
        -- Define a Lua table with functions
        local event_handler = {
            on_event1 = function(data)
                print("Event1 received with data: " .. data.some_key)
            end,

            on_event2 = function(data)
                print("Event2 received with data: " .. data.some_other_key)
            end
        }

        -- Register Lua callbacks
        observer:register_callback(event_handler, "on_event1")
        observer:register_callback(event_handler, "on_event2")
    )");

    // Create a reusable Lua table
    sol::table lua_data = lua.create_table();

    // Set fields and enqueue event1
    lua_data["some_key"] = "value1";
    observer.enqueue_event(LuaEvent{lua_data, "on_event1"});

    // Dispatch events
    observer.dispatch_all_events();

    // Reuse the same table for another event
    lua_data["some_other_key"] = "value2";
    observer.enqueue_event(LuaEvent{lua_data, "on_event2"});

    // Dispatch events
    observer.dispatch_all_events();

    return 0;
}
*/
void bind_conditional_observer(auto& lua, ConditionalObserver& observer)
{
    lua->template new_usertype<ConditionalObserver>("ConditionalObserver",
        "register_callback", [](ConditionalObserver& observer, const sol::table& lua_table, const std::string& event_name) {
            auto lua_callback = [lua_table, event_name](const LuaEvent& luaEvent) {
                if (lua_table.valid())
                {
                    sol::function lua_function = lua_table[event_name];
                    if (lua_function.valid())
                    {
                        lua_function(luaEvent.data);
                    }
                    else
                    {
                        std::cerr << "Error: Lua function " << event_name << " is not valid." << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Error: Lua table is not valid." << std::endl;
                }
                };

            observer.register_callback(lua_callback);
        },
        "enqueue_event", [](ConditionalObserver& observer, const sol::table& lua_data, const std::string& event_name) {
            observer.enqueue_event(LuaEvent{ lua_data, event_name });
        },
        "dispatch_all_events", &ConditionalObserver::dispatch_all_events,
        "clear", &ConditionalObserver::clear
    );

    lua->operator[]("observer") = &observer;
}

namespace Inspector
{
    void inspect_scene_graph_node(
        SceneGraph& scenegraph,
        Editor::InspectorState& inspector,
        size_t index = 0)
    {
        assert(index >= 0 && index < scenegraph.tree.size());

        auto& registry = inspector.context.registry;
        auto [entity, nbr_children, branch_stride, parent_ofs] = scenegraph.tree.get_node_info_at(index);

        std::string label = Editor::get_entity_name(registry, entity, entt::resolve<HeaderComponent>());
        // Add entity nbr to start for clarity
        label = "[entity#" + std::to_string(entt::to_integral(entity)) + "] " + label;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
        if (!nbr_children) flags |= ImGuiTreeNodeFlags_Leaf;
        if (inspector.selected_entity == entity) flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNodeEx(label.c_str(), flags))
        {
            if (ImGui::IsItemClicked())
                inspector.selected_entity = entity;

            // Recursively display each child node
            int child_index = index + 1;
            for (int i = 0; i < nbr_children; ++i)
            {
                inspect_scene_graph_node(scenegraph, inspector, child_index);

                auto [entity, nbr_children, branch_stride, parent_ofs] = scenegraph.tree.get_node_info_at(child_index);
                child_index += branch_stride; //scenegraph.tree[current_index].;
            }

            ImGui::TreePop();
        }
    }

    // Or a "window" function that does SG + buttons for New, Copy etc
    void inspect_scenegraph(
        SceneGraph& scenegraph,
        Editor::InspectorState& inspector,
        ConditionalObserver& observer)
    {
        auto& registry = inspector.context.registry;
        static bool open = true;
        bool* p_open = &open;

        // ImGui window
        ImGui::SetNextWindowBgAlpha(0.35f);
        if (!ImGui::Begin("Scene graph", p_open))
        {
            ImGui::End();
            return;
        }

        // New entity
        if (ImGui::Button("New"))
        {
            Scene::CreateEntityEvent event{ .parent_entity = inspector.selected_entity };
            observer.enqueue_event(event);
        }

        // Destroy selected entity
        if (inspector.selected_entity == entt::null) inspector.begin_disabled();
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            Scene::DestroyEntityEvent event{ .entity = inspector.selected_entity };
            observer.enqueue_event(event);
        }
        if (inspector.selected_entity == entt::null) inspector.end_disabled();

        // Copy selected entity
        if (inspector.selected_entity == entt::null) inspector.begin_disabled();
        ImGui::SameLine();
        if (ImGui::Button("Copy"))
        {
            Scene::CopyEntityEvent event{ .entity = inspector.selected_entity };
            observer.enqueue_event(event);
        }
        if (inspector.selected_entity == entt::null) inspector.end_disabled();

        // TODO: Parent
        inspector.begin_disabled();
        ImGui::SameLine();
        if (ImGui::Button("Parent")) {}

        // TODO: Unparent
        ImGui::SameLine();
        if (ImGui::Button("Unparent")) {}
        inspector.end_disabled();

        // Explicit traverse button
        ImGui::SameLine();
        if (ImGui::Button("Traverse"))
        {
            scenegraph.traverse(registry);
        }

        // Scene graph
        if (scenegraph.size())
        {
            // For all roots ...
            size_t i = 0;
            while (i < scenegraph.tree.size())
            {
                inspect_scene_graph_node(scenegraph, inspector, i);
                i += scenegraph.tree.nodes[i].m_branch_stride;
            }
        }

        ImGui::End(); // Window
    }

    bool inspect_entity(Editor::InspectorState& inspector)
    {
        auto& registry = inspector.context.registry;
        bool mod = false;
        static bool open = true;
        bool* p_open = &open;

        bool selected_entity_valid =
            inspector.selected_entity != entt::null &&
            registry->valid(inspector.selected_entity);

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (!ImGui::Begin("Inspector", p_open))
        {
            ImGui::End();
            return mod;
        }

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode("Components"))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
            const ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
            //        const ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable;
            if (ImGui::BeginTable("InspectorTable", 2, flags))
            {

                // entt::id_type of HeaderComponent, to obtain names for entities with those
                auto header_meta = entt::resolve<HeaderComponent>();

                if (selected_entity_valid)
                {
                    mod |= Editor::inspect_entity(inspector.selected_entity, inspector);
                }
                else
                    ImGui::Text("Selected entity is null or invalid");

                //Editor::inspect_registry(header_meta, inspector);

                // if (inspector_widget.begin_node("comp name"))
                // {
                //     // type_widget(*t, w, scene);
                //     ImGui::Text("Hello!");
                //     inspector_widget.end_node();
                // }


                ImGui::EndTable();
            }
            ImGui::TreePop(); // Entities node
            ImGui::PopStyleVar();
        }
        ImGui::End(); // Window

        return mod;
    }

    void inspect_command_queue(Editor::InspectorState& inspector)
    {
        static bool open = true;
        bool* p_open = &open;

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (!ImGui::Begin("Command Queue", p_open))
        {
            ImGui::End();
            return;
        }

        // Fetch state
        assert(!inspector.cmd_queue.expired());
        auto cmd_queue = inspector.cmd_queue.lock();
        bool can_undo = cmd_queue->can_undo();
        bool can_redo = cmd_queue->commands_pending();
        bool has_commands = cmd_queue->size();

        // Undo button
        if (!can_undo) ImGui::BeginDisabled();
        if (ImGui::Button("Undo")) cmd_queue->undo_last();
        if (!can_undo) ImGui::EndDisabled();

        // Redo button
        ImGui::SameLine();
        if (!can_redo) ImGui::BeginDisabled();
        if (ImGui::Button("Redo")) cmd_queue->execute_next();
        if (!can_redo) ImGui::EndDisabled();

        // Clear button
        ImGui::SameLine();
        if (!has_commands) ImGui::BeginDisabled();
        if (ImGui::Button("Clear")) cmd_queue->clear();
        if (!has_commands) ImGui::EndDisabled();

        // Command count
        ImGui::SameLine();
        ImGui::Text("%zu", cmd_queue->size());

        // Command list
        ImGui::BeginChild("ItemListChild", ImVec2(0, 300), true);
        for (int i = 0; i < cmd_queue->size(); ++i)
        {
            // Auto-scroll
            if (i == cmd_queue->get_current_index() - 1)
                ImGui::SetScrollHereY(0.5f);

            bool is_pending = !cmd_queue->is_executed(i);
            if (is_pending)
                ImGui::TextDisabled("%s", cmd_queue->get_name(i).c_str());
            else
                ImGui::Text("%s", cmd_queue->get_name(i).c_str());
        }
        ImGui::EndChild(); // Command list window

        ImGui::End(); // Window
    }

    void inspect_playstate(const Scene::GamePlayState& play_state, ConditionalObserver& observer)
    {
        static bool open = true;
        bool* p_open = &open;

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (!ImGui::Begin("Play / Stop", p_open))
        {
            ImGui::End();
            return;
        }

        // Fetch state
        bool can_play = play_state != Scene::GamePlayState::Play;
        bool can_stop = play_state != Scene::GamePlayState::Stop;
        bool can_pause = play_state == Scene::GamePlayState::Play;

        // Play button
        if (!can_play) ImGui::BeginDisabled();
        if (ImGui::Button("Play##playpause"))
            observer.enqueue_event(Scene::SetGamePlayStateEvent{ Scene::GamePlayState::Play });
        if (!can_play) ImGui::EndDisabled();

        // Pause button
        ImGui::SameLine();
        if (!can_pause) ImGui::BeginDisabled();
        if (ImGui::Button("Pause##playpause"))
            observer.enqueue_event(Scene::SetGamePlayStateEvent{ Scene::GamePlayState::Pause });
        if (!can_pause) ImGui::EndDisabled();

        // Stop button
        ImGui::SameLine();
        if (!can_stop) ImGui::BeginDisabled();
        if (ImGui::Button("Stop##playpause"))
            observer.enqueue_event(Scene::SetGamePlayStateEvent{ Scene::GamePlayState::Stop });
        if (!can_stop) ImGui::EndDisabled();

        ImGui::End(); // Window
    }

    void inspect_chunkregistry(ChunkRegistry& chunk_registry, ConditionalObserver& observer)
    {
        static bool open = true;
        bool* p_open = &open;

        static std::string selected_chunk_tag;
        static std::string load_json_path = "/Users/ag1498/GitHub/LuaScripting/LuaGame/lua/init.lua"; //

        ImGui::SetNextWindowBgAlpha(0.35f);
        if (!ImGui::Begin("Scene Chunks", p_open))
        {
            ImGui::End();
            return;
        }

        ImGui::InputText("##loadjson", &load_json_path, 0, nullptr, nullptr);

        // Load JSON
        if (ImGui::Button("Load file"))
        {
            observer.enqueue_event(Scene::LoadFileEvent{ load_json_path });
        };

        // Chunk list
        static std::string label;
        ImGui::BeginChild("ChunkList", ImVec2(0, 100), true);
        for (auto& [chunk_tag, entities] : chunk_registry.chunks())
        {
            bool is_selected = (selected_chunk_tag == chunk_tag);
            label = chunk_tag + " (" + std::to_string(entities.size()) + ")";

            if (ImGui::Selectable(label.c_str(), is_selected))
                selected_chunk_tag = chunk_tag;
        }
        ImGui::EndChild(); // Chunk list window

        if (ImGui::Button("Destroy chunk"))
        {
            observer.enqueue_event(Scene::DestroyChunkEvent{ selected_chunk_tag });
        };

        ImGui::End(); // Window
    }
} // Inspector

inline void lua_panic_func(sol::optional<std::string> maybe_msg)
{
    std::cerr << "Lua is in a panic state and will now abort() the application" << std::endl;
    if (maybe_msg) {
        const std::string& msg = maybe_msg.value();
        std::cerr << "\terror message: " << msg << std::endl;
    }
    // When this function exits, Lua will exhibit default behavior and abort()
}

bool Scene::entity_parent_registered(
    entt::entity entity)
{
    assert(registry->all_of<HeaderComponent>(entity));
    auto& header = registry->get<HeaderComponent>(entity);
    auto entity_parent = entt::entity{ header.entity_parent };

    if (entity_parent == entt::null) return true;
    return scenegraph.tree.contains(entity_parent);
}

void Scene::register_entity(
    entt::entity entity)
{
    assert(registry->all_of<HeaderComponent>(entity));

    auto& header = registry->get<HeaderComponent>(entity);
    auto& chunk_tag = header.chunk_tag;
    auto entity_parent = entt::entity{ header.entity_parent };

    chunk_registry.addEntity(header.chunk_tag, entity);

    // std::cout << "Scene::create_entity_and_attach_to_scenegraph " << entt::to_integral(entity) << std::endl; //
    if (entity_parent == entt::null)
    {
        scenegraph.create_node(entity);
    }
    else
    {
        assert(scenegraph.tree.contains(entity_parent));
        scenegraph.create_node(entity, entity_parent);
    }
}

entt::entity Scene::create_entity(
    const std::string& chunk_tag,
    const std::string& name,
    entt::entity entity_parent,
    entt::entity entity_hint)
{
    entt::entity entity;
    if (entity_hint == entt::null)
    {
        entity = registry->create();
    }
    else
    {
        entity = registry->create(entity_hint);
        assert(entity == entity_hint);
    }

    std::string used_name = name.size() ? name : std::to_string(entt::to_integral(entity));
    std::string used_chunk_tag = chunk_tag.size() ? chunk_tag : "default_chunk";
    uint32_t guid = 0;
    registry->emplace<HeaderComponent>(entity, used_name, used_chunk_tag, guid, entt::to_integral(entity_parent));

#if 1
    register_entity(entity);
#else
    chunk_registry.addEntity(used_chunk_tag, entity);

    // std::cout << "Scene::create_entity_and_attach_to_scenegraph " << entt::to_integral(entity) << std::endl; //
    if (entity_parent == entt::null)
    {
        scenegraph.create_node(entity);
    }
    else
    {
        assert(scenegraph.tree.contains(entity_parent));
        scenegraph.create_node(entity, entity_parent);
    }
#endif

    std::cout << "Scene::create_entity " << entt::to_integral(entity) << std::endl;
    return entity;
}

// entt::entity Scene::create_entity_hint(
//     entt::entity hint_entity,
//     entt::entity parent_entity)
// {
//     return create_entity("", "", parent_entity, hint_entity);
// }

// entt::entity Scene::create_entity(
//     entt::entity parent_entity)
// {
//     return create_entity("", "", parent_entity, entt::null);
// }

void Scene::queue_entity_for_destruction(entt::entity entity)
{
    entities_pending_destruction.push_back(entity);
}

void Scene::destroy_pending_entities()
{
    int count = 0;
    while (entities_pending_destruction.size())
    {
        auto entity = entities_pending_destruction.back();
        entities_pending_destruction.pop_back();

        // Remove from chunk registry
        assert(registry->all_of<HeaderComponent>(entity));
        chunk_registry.removeEntity(registry->get<HeaderComponent>(entity).chunk_tag, entity);

        // Remove from scene graph
        if (scenegraph.tree.contains(entity))
            scenegraph.erase_node(entity);

        // Destroy entity. May lead to additional entities being added to the queue.
        registry->destroy(entity);

        count++;
    }

    if (count)
        eeng::Log("%i entities destroyed", count);
}

bool Scene::init(const v2i& windowSize)
{
    assert(!is_initialized);

    std::cout << "Scene::init()" << std::endl;

    std::cout << "sizeof(CircleColliderGridComponent) " << sizeof(CircleColliderGridComponent) << std::endl;
    std::cout << "sizeof(QuadGridComponent) " << sizeof(QuadGridComponent) << std::endl;
    std::cout << "sizeof(DataGridComponent) " << sizeof(DataGridComponent) << std::endl;

    SceneBase::windowSize = windowSize;

    // Register registry entt::meta functions to components
    register_meta_component<Transform>();
    register_meta_component<QuadComponent>();
    register_meta_component<CircleColliderComponent>();
    register_meta_component<ScriptedBehaviorComponent>();
    //
    register_meta_component<CircleColliderGridComponent>();
    register_meta_component<QuadGridComponent>();
    register_meta_component<IslandFinderComponent>();
    register_meta_component<DataGridComponent>();
    register_meta_component<HeaderComponent>();

    // WIP
    register_basic_type<std::string>();
    registerDebugClass();

    // Register meta for Scene related stuff
    entt::meta<GamePlayState>()
        .type("GamePlayState"_hs)
        .prop(display_name_hs, "GamePlayState")
        .prop("underlying_meta_type"_hs, entt::resolve<std::underlying_type_t<GamePlayState>>())

        .data<GamePlayState::Play>("Play"_hs).prop(display_name_hs, "Play")
        .data<GamePlayState::Stop>("Stop"_hs).prop(display_name_hs, "Stop")
        .data<GamePlayState::Pause>("Pause"_hs).prop(display_name_hs, "Pause");

    // Create registry
    registry = std::make_shared<entt::registry>();
    registry->on_destroy<ScriptedBehaviorComponent>().connect<&release_script>();

    cmd_queue = std::make_shared<Editor::CommandQueue>();

    // Hook up Scene events
    observer.register_callback([&](const SetGamePlayStateEvent& event) { this->OnSetGamePlayStateEvent(event); });
    observer.register_callback([&](const DestroyChunkEvent& event) { this->OnDestroyChunkEvent(event); });
    observer.register_callback([&](const LoadFileEvent& event) { this->OnLoadFileEvent(event); });
    observer.register_callback([&](const CreateEntityEvent& event) { this->OnCreateEntityEvent(event); });
    observer.register_callback([&](const DestroyEntityEvent& event) { this->OnDestroyEntityEvent(event); });
    observer.register_callback([&](const CopyEntityEvent& event) { this->OnCopyEntityEvent(event); });

    try
    {

        // Create Lua state
        lua = std::make_shared<sol::state>(sol::c_call<decltype(&lua_panic_func), &lua_panic_func>);
        lua->open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string,
            sol::lib::math,
            sol::lib::os,
            sol::lib::table);

        // - Start of Lua binding -

        lua->create_named_table("engine");

        lua->operator[]("engine")["entity_null"] = entt::entity{ entt::null };
        // lua["engine"]["entity_null"] = entt::entity{ entt::null };

        // Create entity
        //
        lua->operator[]("engine")["create_entity"] = [&](
            const std::string& chunk_tag,
            const std::string& name,
            entt::entity parent_entity) {
                // return create_entity_and_attach_to_scenegraph(parent_entity);
                return create_entity(chunk_tag, name, parent_entity, entt::null);
            };

        // Destroy entity
        //
        lua->operator[]("engine")["destroy_entity"] = [&](entt::entity entity) {
            assert(registry->valid(entity));
            queue_entity_for_destruction(entity);
            };

        // Register to Lua: helper functions for adding & obtaining scripts from entities
        lua->operator[]("engine")["add_script"] = [&](
            entt::registry& registry,
            entt::entity entity,
            const std::string& script_name) {
                return add_script_from_file(registry, entity, lua, script_dir, script_name);
            };
        lua->operator[]("engine")["get_script"] = &get_script;

        // // Try to reserve entity root ...
        // auto entity = registry.create(root_entity);
        // assert(root_entity == entity);
        // registry.emplace<HeaderComponent>(root_entity, HeaderComponent{ "Scene root" });
        // // Add scene graph root node
        // scene_graph.create_node(root_entity, entt::null);

        // {
        //     // Bind scene graph op's
        //     sol::table my_table = lua.create_table();
        //     my_table.set_function("add_entity", [&](sol::table self, entt::entity entity, entt::entity parent_entity) {
        //         bool result = scenegraph.create_node(entity, parent_entity);
        //         assert(result);
        //         // Debug print SG
        //         //scenegraph.dump_to_cout(registry, entt::resolve<HeaderComponent>());
        //         return result;
        //         });
        //     my_table.set_function("add_entity_as_root", [&](sol::table self, entt::entity entity) {
        //         bool result = scenegraph.create_node(entity);
        //         assert(result);
        //         // Debug print SG
        //         //scenegraph.dump_to_cout(registry, entt::resolve<HeaderComponent>());
        //         return result;
        //         });
        //     my_table.set_function("remove_entity", [&](sol::table self, entt::entity entity) {
        //         // scenegraph.dump_to_cout(registry, entt::resolve<HeaderComponent>());
        //         // std::cout << "remove_entity " << entt::to_integral(entity) << std::endl;
        //         // bool result = scenegraph.erase_node(entity);
        //         // assert(result);
        //         // // Debug print SG
        //         // scenegraph.dump_to_cout(registry, entt::resolve<HeaderComponent>());
        //         // return result;
        //         });
        //     // + dump_to_cout
        //     lua["scenegraph"] = my_table;
        // }

        // Entities are destroyed outside the regular update loop
        // lua["flag_entity_for_destruction"] = [&](entt::entity entity)
        //     {
        //         entities_pending_destruction.push_back(entity);
        //     };

        // Logging from Lua
        lua->operator[]("engine")["log"] = [&](const std::string& text)
            {
                eeng::Log("[Lua] %s", text.c_str());
                //eeng::Log::log((std::string("[Lua] ") + text).c_str());
            };

        // Particle emitter functions
        lua->operator[]("engine")["emit_particle"] = [&](
            float x,
            float y,
            float vx,
            float vy,
            uint32_t color)
            {
                particleBuffer.push_point(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, color);
            };
        lua->operator[]("engine")["emit_explosion"] = [&](
            float x,
            float y,
            float vx,
            float vy,
            int nbr_particles,
            uint32_t color)
            {
                particleBuffer.push_explosion(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, nbr_particles, color);
            };
        lua->operator[]("engine")["emit_trail"] = [&](
            float x,
            float y,
            float vx,
            float vy,
            int nbr_particles,
            uint32_t color)
            {
                particleBuffer.push_trail(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, nbr_particles, color);
            };

        // Input v2
        lua->operator[]("engine")["input"] = lua->create_table();
        update_input_lua(lua, linalg::v4f_0000, vec4<bool> {false, false, false, false});

        // Attach registry to Lua state
        lua->require("registry", sol::c_call<AUTO_ARG(&open_registry)>, false);
        lua->operator[]("engine")["registry"] = std::ref(registry);

        // Register AudioManager to Lua
        bindAudioManager(lua);

        // Register core components to Lua
        // register_transform(lua);
        register_meta<Transform>(lua);
        register_meta<HeaderComponent>(lua);
        register_meta<CircleColliderGridComponent>(lua);
        register_meta<IslandFinderComponent>(lua);
        register_meta<QuadGridComponent>(lua);
        register_meta<DataGridComponent>(lua);
        register_meta<ScriptedBehaviorComponent>(lua);
        //
        registerQuadComponent(lua); // not used
        registerCircleColliderComponent(lua); // not used
        // registerScriptedBehaviorComponent(lua);
        //
        // registerCircleColliderGridComponent(lua);
        // registerQuadGridComponent(lua);
        // registerIslandFinderComponent(lua);
        // registeDataGridComponent(lua);
        //
        // HeaderComponent_metaregister(lua); // entt::meta (registry stuff added separately) + sol meta
        // ScriptedBehaviorComponent_metaregister(lua);

        // ImGui -> Lua
        lua->set_function("ImGui_Text", &ImGui_Text);
        lua->set_function("ImGui_Begin", &ImGui_Begin);
        lua->set_function("ImGui_End", &ImGui_End);
        lua->set_function("ImGui_SetNextWindowPos", &ImGui_SetNextWindowPos);
        lua->set_function("ImGui_SetNextWindowWorldPos", [&](float x, float y)
            {
                // Transform from world to screen space
                const v4f pos_ss = (VP * P * V) * v4f{ x, y, 0.0f, 1.0f };
                ImGui_SetNextWindowPos(pos_ss.x / pos_ss.w, SceneBase::windowSize.y - pos_ss.y / pos_ss.w);
            });

        // Bind observer
        bind_conditional_observer(lua, observer);

        // Load & execute init script
        lua->safe_script_file(script_dir + "init.lua");

        // Validate & init the game table
        {
            auto lua_game = lua->operator[]("game");
            assert(lua_game.valid());
            assert(lua_game["init"].valid());
            assert(lua_game["destroy"].valid());
            lua_game["init"](lua_game);
        }
        // lua["game"]["destroy"]();

        // - Lua binding done -

        // SERIALIZATION TEST

        {
            serialization_test(lua);

        }

        // Dump Lua game & engine states
        {
            auto lua_game = lua->operator[]("game");
            auto lua_engine = lua->operator[]("engine");
            std::cout << "Inspect Lua engine state:" << std::endl;
            dump_lua_state(lua, lua_engine, "    ");
            std::cout << "Inspect Lua game state:" << std::endl;
            dump_lua_state(lua, lua_game, "    ");
        }

        // Debugging inspection
        // auto debug_entity = create_entity_and_attach_to_scenegraph();
        auto debug_entity = create_entity("", "", entt::null, entt::null);// registry.create();
        registry->template emplace<DebugClass>(debug_entity);

#if 0
        // Send event C++ <-> C++
        struct MyEvent { float x; } event{ 5.0f };
        observer.register_callback([](const MyEvent& e) { std::cout << "C++: MyEvent: " << e.x << std::endl; });
        observer.enqueue_event(event);
        observer.dispatch_all_events();
        observer.clear();

        // C++ listens to event sent from Lua (ever needed?)
        observer.register_callback([](const LuaEvent& e) { std::cout << "C++: LuaEvent: " << e.event_name << std::endl; });

        // Script with internal events via Observer (can be listened to from C+ as well)
        lua.script(R"(

        print("Hello!")

        -- Define a Lua table with functions
        local event_handler = {
            on_event1 = function(data)
                print("Event1 received with data: " .. data.some_key)
            end,

            on_event2 = function(data)
                print("Event2 received with data: " .. data.some_other_key)
            end
        }

        -- Register Lua callbacks
        observer:register_callback(event_handler, "on_event1")
        observer:register_callback(event_handler, "on_event2")

        -- Enqueue events
        observer:enqueue_event({ some_key = "value1" }, "on_event1")
        observer:enqueue_event({ some_other_key = "value2" }, "on_event2")

        -- Dispatch events
        observer:dispatch_all_events()

        -- Clear events
        observer:clear()
    )");
#endif

        // Run engine-side init code
#if 0
        for (int i = 0; i < 5; ++i)
        {
            auto entity = registry.create();

            registry.emplace<Transform>(entity, Transform{ (float)-i, (float)-i });

            registry.emplace<QuadComponent>(entity, QuadComponent{ 1.0f, 0x80ffffff, true });

            add_script_from_file(registry, entity, lua, "lua/behavior.lua", "test_behavior");
        }
#endif
    }
    // catch (const std::exception& e)
    catch (const sol::error& e)
    {
        std::cerr << "Exception: " << e.what();
        throw std::runtime_error(e.what());
    }

    is_initialized = true;
    return true;
}

void Scene::update(float time_s, float deltaTime_s)
{
    assert(is_initialized);

    // Light position
    lightPos = xyz(m4f::TRS(
        { 1000.0f, 1000.0f, 1000.0f },
        time_s * 0.0f,
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }) * linalg::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    // Camera position
    eyePos = xyz(m4f::TRS(
        { 0.0f, 0.0f, 5.0f },
        0.0f,
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }) * vec4 {
        0.0f, 0.0f, 0.0f, 1.0f
    });

    // Viewport matrix
    VP = mat4f::GL_Viewport(0.0f, SceneBase::windowSize.x, SceneBase::windowSize.y, 0.0f, 0.0f, 1.0f);

    // Projection matrix
    const float aspectRatio = float(SceneBase::windowSize.x) / SceneBase::windowSize.y;
    P = m4f::GL_OrthoProjectionRHS(7.5f * aspectRatio, 7.5f, nearPlane, farPlane);

    // View matrix
    V = m4f::TRS(eyePos, 0.0f, v3f{ 1.0f, 0.0f, 0.0f }, v3f{ 1.0f, 1.0f, 1.0f }).inverse();

    // Input
    update_input_lua(lua, SceneBase::axes, SceneBase::buttons);

    // Update scripts
    script_system_update(registry, deltaTime_s);

    // Destroy pending entities
    // After script update because: scripts may remove entities
    destroy_pending_entities();

    // Update scene graph
    // After script update because: scripts may alter positions
    scenegraph.traverse(registry);

    // Update particles
    particleBuffer.update(deltaTime_s);

    // Debug print
    if (entities_pending_destruction.size())
    {
        std::cout << "Destroying " << (int)entities_pending_destruction.size() << " entities... ";
        for (auto entity : entities_pending_destruction) std::cout << entt::to_integral(entity) << " ";
        std::cout << std::endl;
    }

    // Placeholder collision system
#if 1
    {
        const auto dispatch_collision_event_to_scripts = [&](
            float x,
            float y,
            float nx,
            float ny,
            int collider_index,
            entt::entity entity,
            entt::entity other_entity)
            {
                if (!registry->all_of<ScriptedBehaviorComponent>(entity)) return;
                auto& script_comp = registry->get<ScriptedBehaviorComponent>(entity);
                for (auto& script : script_comp.scripts)
                {
                    assert(script.self.valid());
                    script.on_collision(script.self, x, y, nx, ny, collider_index, other_entity);
                }
            };

        auto view = registry->view<CircleColliderGridComponent>();
        for (auto it1 = view.begin(); it1 != view.end(); ++it1)
        {
            auto entity1 = *it1;
            const auto& transform1 = registry->get<Transform>(entity1);
            const auto& collider1 = view.template get<CircleColliderGridComponent>(entity1);
            if (!collider1.is_active) continue;

            const auto R1 = m2f::rotation(transform1.angle_global);
            for (auto it2 = it1; ++it2 != view.end(); )
            {
                auto entity2 = *it2;
                const auto& transform2 = registry->get<Transform>(entity2);
                const auto& collider2 = view.get<CircleColliderGridComponent>(entity2);

                if (!collider2.is_active) continue;
                // LAYER CHECK
                if (!(collider1.layer_bit & collider2.layer_mask)) continue;

                const auto R2 = m2f::rotation(transform2.angle_global);
                // for (auto i = 0; i < collider1.count; i++)
                for (int di = 0; di < collider1.active_indices.get_dense_count(); di++)
                {
                    const int i = collider1.active_indices.get_dense(di);
                    //if (!collider1.is_active_flags[i]) continue;
                    const auto pos1 = R1 * collider1.circles[i].pos;

                    //for (auto j = 0; j < collider2.count; j++)
                    for (int dj = 0; dj < collider2.active_indices.get_dense_count(); dj++)
                    {
                        const int j = collider2.active_indices.get_dense(dj);
                        //if (!collider2.is_active_flags[j]) continue;
                        const auto pos2 = R2 * collider2.circles[j].pos;

                        const float x1 = transform1.x_global + pos1.x;
                        const float y1 = transform1.y_global + pos1.y;
                        const float x2 = transform2.x_global + pos2.x;
                        const float y2 = transform2.y_global + pos2.y;

                        const float r1 = collider1.circles[i].radius;
                        const float r2 = collider2.circles[j].radius;

                        // Distance between colliders
                        const float dx = x1 - x2;
                        const float dy = y1 - y2;
                        const float distanceSquared = dx * dx + dy * dy;
                        const float radiusSum = r1 + r2;

                        // Check for collision
                        if (distanceSquared < radiusSum * radiusSum)
                        {
                            // Collision detected

                            // Distance
                            float distance = std::sqrt(distanceSquared);

                            // Penetration depth
                            float penetrationDepth = radiusSum - distance;

                            // Contact normal
                            float nx = dx / distance;
                            float ny = dy / distance;
                            // auto n = d * (1.0f / distance);

                            // Point of contact
                            float px = x1 - r1 * nx + r2 * nx;
                            float py = y1 - r1 * ny + r2 * ny;

                            dispatch_collision_event_to_scripts(px, py, -nx, -ny, i, entity1, entity2);
                            dispatch_collision_event_to_scripts(px, py, nx, ny, j, entity2, entity1);
                        }
                    } // j
                } // i
            }
        }
    } // anon
#else
    {
        const auto dispatch_collision_event_to_scripts = [&](
            float x,
            float y,
            float nx,
            float ny,
            entt::entity entity,
            entt::entity other_entity)
            {
                if (!registry.all_of<ScriptedBehaviorComponent>(entity)) return;
                auto& script_comp = registry.get<ScriptedBehaviorComponent>(entity);
                for (auto& script : script_comp.scripts)
                {
                    assert(script.self.valid());
                    script.on_collision(script.self, x, y, nx, ny, other_entity);
                }
            };

        auto view = registry.view<Transform, CircleColliderComponent>();
        for (auto it1 = view.begin(); it1 != view.end(); ++it1)
        {
            auto entity1 = *it1;
            const auto& transform1 = view.get<Transform>(entity1);
            const auto& collider1 = view.get<CircleColliderComponent>(entity1);
            if (!collider1.is_active) continue;

            for (auto it2 = it1; ++it2 != view.end(); )
            {
                auto entity2 = *it2;
                const auto& transform2 = view.get<Transform>(entity2);
                const auto& collider2 = view.get<CircleColliderComponent>(entity2);
                if (!collider2.is_active) continue;

                // Calculate the distance between the two entities
                float dx = transform1.x - transform2.x;
                float dy = transform1.y - transform2.y;
                float distanceSquared = dx * dx + dy * dy;
                float radiusSum = collider1.r + collider2.r;

                // Check for collision
                if (distanceSquared < radiusSum * radiusSum)
                {
                    // Collision detected

                    // Calculate distance
                    float distance = std::sqrt(distanceSquared);

                    // Calculate penetration depth
                    float penetrationDepth = radiusSum - distance;

                    // Calculate contact normal
                    float nx = dx / distance;
                    float ny = dy / distance;

                    // Calculate point of contact
                    float px = transform1.x - collider1.r * nx + collider2.r * nx;
                    float py = transform1.y - collider1.r * ny + collider2.r * ny;

                    // std::cout << "Collision detected between entity " << (uint32_t)entity1
                    //     << " and entity " << (uint32_t)entity2 << std::endl;
                    // std::cout << "Contact Point: (" << px << ", " << py << ")\n";
                    // std::cout << "Contact Normal: (" << nx << ", " << ny << ")\n";
                    // std::cout << "Penetration Depth: " << penetrationDepth << "\n";

                    // (nx, ny) points 2 -> 1
                    dispatch_collision_event_to_scripts(px, py, -nx, -ny, entity1, entity2);
                    dispatch_collision_event_to_scripts(px, py, nx, ny, entity2, entity1);
                }
            }
        }
    } // anon
#endif

    IslandFinderSystem(registry, deltaTime_s);

    observer.dispatch_all_events();
}

void Scene::renderUI()
{
    assert(is_initialized);

    {
        size_t nbr_entities = 0;
        auto view = registry->view<entt::entity>();
        for (auto entity : view) nbr_entities++;
        ImGui::Text("Nbr of active entities %i", (int)nbr_entities);
    }

    ImGui::Text("Drawcall count %i", drawcallCount);

    ImGui::Checkbox("Debug render", &debug_render);

    ImGui::Text("Particles %i/%i", particleBuffer.size(), particleBuffer.capacity());

    // Placeholder serialize
    if (ImGui::Button("Serialize"))
    {
        // Serialize
        nlohmann::json jser = Meta::serialize_registry(registry);
        std::cout << "JSON dump" << std::endl << jser.dump(4) << std::endl;

        // Deserialize
        std::cout << "\nDeserialization" << std::endl;
        // destroy_pending_entities(); this?
        registry->clear();
        entities_pending_destruction.clear();
        // NOTE: SG is left unchanged

        scenegraph.tree.nodes.clear(); // visibility ...

        chunk_registry.clear();

        auto context = create_context();
        Meta::deserialize_entities(jser, context);
        // + clear command buffer
    }

    // Set inspector
    // Note: does not properly track what happens to the selected entity, so check its validity here
    static Editor::InspectorState inspector{};
    inspector.context = create_context();
    inspector.cmd_queue = cmd_queue;
    if (!registry->valid(inspector.selected_entity)) inspector.selected_entity = entt::null;
    // + cmd_queue + CommandBuilder ???

    if (Inspector::inspect_entity(inspector))
    {
        //std::cout << "cmd_queue->size() " << cmd_queue->size() << std::endl;
         //cmd_queue->execute_pending();
    }

    Inspector::inspect_command_queue(inspector);

    // Before inspect_entity ???
    Inspector::inspect_scenegraph(scenegraph, inspector, observer);

    Inspector::inspect_playstate(play_state, observer);

    Inspector::inspect_chunkregistry(chunk_registry, observer);

    observer.dispatch_all_events();
    //std::cout << cmd_queue->has_new() << std::endl;
    //std::cout << cmd_queue->has_new() << " " << cmd_queue->new_commands_pending() << std::endl;
    // if (cmd_queue->has_new())
    if (cmd_queue->new_commands_pending())
        cmd_queue->execute_pending();
    destroy_pending_entities();
}

void Scene::render(float time_s, ShapeRendererPtr renderer)
{
    assert(is_initialized);

    // Background quad - TODO script this
    {
        const auto M = set_translation(m4f::scaling(15.0f, 10.0f, 1.0f), v3f{ 2.5f, 0.0f, 0.0f });
        renderer->push_states(M, Renderer::Color4u{ 0x40ffffff });
        renderer->push_quad();
        renderer->pop_states<m4f, Renderer::Color4u>();
    }

    // Render QuadComponents
    // Todo: Remove Transform from view
    {
        auto view = registry->view<Transform, QuadComponent>();
        float z = 0.0f;
        for (auto entity : view)
        {
            auto& transform_comp = registry->get<Transform>(entity);

            auto& quad_comp = registry->get<QuadComponent>(entity);
            if (!quad_comp.is_visible) continue;

            const auto pos = v3f{ transform_comp.x_global, transform_comp.y_global, 0.0f };
            const auto& size = quad_comp.w;
            const auto& color = quad_comp.color;

            // renderer->push_states(Renderer::Color4u::Blue);
            // renderer->push_states(Renderer::Color4u{ 0x80ff0000 });
            renderer->push_states(Renderer::Color4u{ color });
            // renderer->push_quad(pos, size);
            renderer->pop_states<Renderer::Color4u>();
        }
    }

#if 1
    // Render all QuadGridComponent
    {
        auto view = registry->view<QuadGridComponent>();
        for (auto entity : view)
        {
            auto& quadgrid = view.get<QuadGridComponent>(entity);
            if (!quadgrid.is_active) continue;

            auto& transform_comp = registry->get<Transform>(entity);
            const auto G = m4f::TRS(
                v3f{ transform_comp.x_global, transform_comp.y_global, 0.0f },
                transform_comp.angle_global, v3f_001,
                v3f_111
            );

            for (int i = 0; i < quadgrid.count; i++)
            {
                if (!quadgrid.is_active_flags[i]) continue;

                const auto& pos = xy0(quadgrid.positions[i]);
                const auto& size = quadgrid.sizes[i];
                const auto& color = quadgrid.colors[i];
                const auto M = set_translation(m4f::scaling(size, size, 1.0f), pos);

                renderer->push_states(G * M, Renderer::Color4u{ color });
                renderer->push_quad();
                renderer->pop_states<m4f, Renderer::Color4u>();
            }
        }
    }
#endif

#if 1
    // Render all CircleColliderGridComponent
    if (debug_render)
    {
        auto view = registry->view<CircleColliderGridComponent>();
        for (auto entity : view)
        {
            auto& collidergrid = view.get<CircleColliderGridComponent>(entity);

            auto& transform_comp = registry->get<Transform>(entity);
            const auto G = m4f::TRS(
                v3f{ transform_comp.x_global, transform_comp.y_global, 0.0f },
                transform_comp.angle_global, v3f_001,
                v3f_111
            );

            // Render all circles in the grid & use a different shading for inactive ones
            for (int i = 0; i < collidergrid.element_count; i++)
            {
                // const int i = collidergrid.active_indices.get_dense(di);
                const auto& pos = xy0(collidergrid.circles[i].pos);
                const auto& r = collidergrid.circles[i].radius;
                const auto M = set_translation(m4f::scaling(r, r, 1.0f), pos);
                const bool visible = collidergrid.active_indices.contains(i);;
                const auto color = 0xffffffff * visible + 0xff808080 * (1 - visible);

                renderer->push_states(G * M, Renderer::Color4u{ color });
                renderer->push_circle_ring<8>();
                renderer->pop_states<m4f, Renderer::Color4u>();
            }
            // for (int i = 0; i < collidergrid.count; i++)
            // {
            //     const auto& pos = xy0(collidergrid.pos[i]);
            //     const auto& r = collidergrid.radii[i];
            //     const auto M = set_translation(m4f::scaling(r, r, 1.0f), pos);
            //     const bool visible = collidergrid.is_active_flags[i];
            //     const auto color = 0xffffffff * visible + 0xff808080 * (1 - visible);

            //     renderer->push_states(G * M, Renderer::Color4u{ color });
            //     renderer->push_circle_ring<8>();
            //     renderer->pop_states<m4f, Renderer::Color4u>();
            // }
        }
    }
#endif

    // Add some test particles
#if 0
    const int N = 1;
    for (int i = 0; i < N; i++)
    {
        const float angle = fPI / N * i;
        const float x = std::cos(angle);
        const float y = std::sin(angle);
        particleBuffer.push_point(v3f{ 0.0f, 0.0f, 0.0f }, v3f{ x, y, 0.0f } *4, 0xff0000ff);
    }
#endif

    // Render particles
    particleBuffer.render(renderer);

    // Render shapes
    drawcallCount = renderer->render(P * V);
    renderer->post_render();
}

void Scene::destroy()
{
    std::cout << "Scene::destroy()" << std::endl;
    std::cout << "Entities pending destruction " << entities_pending_destruction.size() << std::endl;

    std::cout << "Destroying game..." << std::endl;
    {
        auto lua_game = lua->operator[]("game");
        lua_game["destroy"](lua_game); // <- entities flagged for destruction ???
    }

    std::cout << "Entities pending destruction " << entities_pending_destruction.size() << std::endl;
    destroy_pending_entities();

    // If the game destroys its entities properly, 
    // the registry should be empty at this point
    // (unless entities are added on the engine side)

    std::cout << "Entities remaining in registry: ";
    {
        auto view = registry->view<entt::entity>();
        for (auto entity : view) std::cout << entt::to_integral(entity) << " ";
        std::cout << std::endl;
    }
    std::cout << "Clearing registry..." << std::endl;
    registry->clear();

    // Check so entities were not queued for destruction by registry.clear()
    assert(!entities_pending_destruction.size());

    std::cout << "Nodes remaining in scene graph:" << std::endl;
    scenegraph.dump_to_cout(registry, entt::resolve<HeaderComponent>());

    // Doesn't need to be explicit
    std::cout << "cmd_queue.use_count() " << cmd_queue.use_count() << std::endl;
    std::cout << "registry.use_count() " << registry.use_count() << std::endl;
    std::cout << "lua.use_count() " << lua.use_count() << std::endl;
    cmd_queue.reset();
    registry.reset();
    lua.reset();

    is_initialized = false;
    std::cout << "Done: Scene::destroy()" << std::endl;
}

// void Scene::destroy_chunk(const std::string& chunk_tag)
// {
//     for (auto& entity : chunk_registry.chunk(chunk_tag))
//     {
//         // Either, 
//         // 1) rely on entt's onDestroy event for calling destroy() on scripts
//         // 2) Destroy scripts explicitly: then BehaviorScript should store destroy()

//         registry->destroy(entity);
//     }
// }

Editor::Context Scene::create_context()
{
    return Editor::Context{
        registry,
        lua,
        // Create entity
        [&](entt::entity entity_parent, entt::entity entity_hint) -> entt::entity {
            return this->create_entity("", "", entity_parent, entity_hint);
        },
        // Destroy_entity
        [&](entt::entity entity) -> void {
            this->queue_entity_for_destruction(entity);
        },
        // Can register entity
        [&](entt::entity entity) -> bool {
            return this->entity_parent_registered(entity);
            },
        // Register entity
        [&](entt::entity entity) -> void {
            this->register_entity(entity); }
    };
}

void Scene::save_chunk(const std::string& chunk_tag)
{
    // NOT A COMMAND
}

void Scene::save_all_chunks()
{
    // NOT A COMMAND
}

void Scene::load_json(const std::string& path)
{

}

void Scene::OnSetGamePlayStateEvent(const SetGamePlayStateEvent& event)
{
    auto j_play_state = Meta::serialize_any(event.play_state);
    assert(!j_play_state.is_null());
    eeng::Log("SetGamePlayStateEvent: %s", j_play_state.dump().c_str());

    // NOT A COMMAND

    // Save open tags

    // Play
    //      scripts: run init()/run()

            //std::cout << "update" << std::endl;
        // auto view = registry->template view<ScriptedBehaviorComponent>();
        // for (auto entity : view)
        // {
        //     assert(entity != entt::null);
        //     assert(registry->valid(entity));

        //     auto& script_comp = view.template get<ScriptedBehaviorComponent>(entity);
        //     for (auto& script : script_comp.scripts)
        //     {
        //         assert(script.self.valid());
        //         // std::cout << script.identifier << std::endl;
        //         script.update(script.self, delta_time);
        //     }
        // }

    // Stop

    // Pause


    play_state = event.play_state;
}

void Scene::OnDestroyChunkEvent(const DestroyChunkEvent& event)
{
    eeng::Log("DestroyChunkEvent: %s", event.chunk_tag.c_str());

    // NOT A COMMAND

    for (auto& entity : chunk_registry.chunk(event.chunk_tag))
    {
        // Either, 
        // 1) rely on entt's onDestroy event for calling destroy() on scripts
        queue_entity_for_destruction(entity);

        // 2) Destroy scripts explicitly: then BehaviorScript should store destroy()
        // ...
    }
}

void Scene::OnLoadFileEvent(const LoadFileEvent& event)
{
    eeng::Log("LoadFileEvent: %s", event.path.c_str());
}

void Scene::OnCreateEntityEvent(const CreateEntityEvent& event)
{
    eeng::Log("CreateEntityEvent");

    // const auto create_entity = [&](entt::entity entity_parent, entt::entity entity_hint) -> entt::entity
    //     {
    //         return this->create_entity("", "", entity_parent, entity_hint);
    //     };
    // const auto destroy_entity = [&](entt::entity entity) -> void
    //     {
    //         this->queue_entity_for_destruction(entity);
    //     };

    using namespace Editor;
    auto command = CreateEntityCommand{ event.parent_entity, create_context() };
    // auto command = CreateEntityCommand{ create_entity, destroy_entity, event.parent_entity };
    cmd_queue->add(CommandFactory::Create<CreateEntityCommand>(command));
}

namespace
{
    auto stack_entity_branch(SceneGraph& sg, entt::entity entity)
    {
        std::stack<entt::entity> stack;
        sg.tree.traverse_breadthfirst(entity, [&](auto& entity, size_t index) {
            stack.push(entity);
            });
        return stack;
    }
}

void Scene::OnDestroyEntityEvent(const DestroyEntityEvent& event)
{
    eeng::Log("DestroyEntityEvent: %s", std::to_string(entt::to_integral(event.entity)).c_str());

    // TODO: Validate event.entity

    // const auto destroy_entity = [&](entt::entity entity) -> void
    //     {
    //         this->queue_entity_for_destruction(entity);
    //     };

    // Traverse scene graph branch and add destroy commands bottom-up
    auto branch_stack = stack_entity_branch(scenegraph, event.entity);
    // std::stack<entt::entity> branch_stack;
    // // const auto& [nbr_children, notused1, notused2] = scenegraph.tree.get_node_info(event.entity);
    // scenegraph.tree.traverse_breadthfirst(event.entity, [&](auto& entity, size_t index) {
    //     branch_stack.push(entity);
    //     });
    using namespace Editor;
    while (branch_stack.size())
    {
        auto command = DestroyEntityCommand{ branch_stack.top(), create_context() };
        // auto command = DestroyEntityCommand{ branch_stack.top(), create_context(), destroy_entity };
        cmd_queue->add(CommandFactory::Create<DestroyEntityCommand>(command));
        branch_stack.pop();
    }

    // Single node destroy
    // using namespace Editor;
    // auto command = DestroyEntityCommand{ event.entity, context, destroy_entity };
    // cmd_queue->add(CommandFactory::Create<DestroyEntityCommand>(command));
}

void Scene::OnCopyEntityEvent(const CopyEntityEvent& event)
{
    eeng::Log("CopyEntityEvent: %s", std::to_string(entt::to_integral(event.entity)).c_str());

    assert(event.entity != entt::null);
    assert(registry->valid(event.entity));

    // + REPARENT CHILDREN

    /*

    - Node1
        - Node2
            - Node3

    NOW ->

    - Node1copy
    - Node1
        - Node2
        - Node2copy
            - Node3
            - Node3copy

    SHOULD BE ??? ->

    - Node1copy
        - Node2copy
            - Node3copy
    - Node1
        - Node2
            - Node3

    */

    auto branch_stack = stack_entity_branch(scenegraph, event.entity);
    using namespace Editor;
    while (branch_stack.size())
    {
        auto command = CopyEntityCommand{ branch_stack.top(), create_context() };
        cmd_queue->add(CommandFactory::Create<CopyEntityCommand>(command));
        branch_stack.pop();
    }
}