
#include <thread>
#include <chrono>
#include <filesystem> // get_files_in_folder
#include <algorithm>
#include <execution>

//#include <nlohmann/json.hpp> // to serialize 

#include "imgui.h"

#include "Scene.hpp"
#include "mat.h"

#include "FileManager.hpp" // JSON IO
#include "MetaInspect.hpp"
#include "MetaClone.hpp"
#include "MetaSerialize.hpp"


#include "bond.hpp"
// #include "transform.hpp"
//#include "kbhit.hpp"
#include "AudioManager.hpp"
#include "CoreComponents.hpp"
#include "DebugClass.h"

#include "InspectType.hpp"
// #include "BehaviorScript.hpp"

#include "GuiCommands.hpp" // -> GUI

#include "meta_aux.h" // meta_type_name

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

        const auto from_json = [](const nlohmann::json& j, void* ptr, const Entity& entity, Editor::Context& context)
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

    // void release_all_scripts(entt::registry& registry)
    // {
    //     auto view = registry.view<ScriptedBehaviorComponent>();
    //     for (auto entity : view)
    //     {
    //         release_script(registry, entity);
    //     }
    // }

    /// @brief Call run for all scripts
    /// @param registry 
    void run_scripts(entt::registry& registry)
    {
        //std::cout << "update" << std::endl;
        auto view = registry.template view<ScriptedBehaviorComponent>();
        for (auto entity : view)
        {
            auto& script_comp = view.template get<ScriptedBehaviorComponent>(entity);
            for (auto& script : script_comp.scripts)
            {
                assert(script.self.valid());
                if (script.run) script.run->operator()(script.self);
            }
        }
    }

    /// @brief Call run for all scripts
/// @param registry 
    void stop_scripts(entt::registry& registry)
    {
        //std::cout << "update" << std::endl;
        auto view = registry.template view<ScriptedBehaviorComponent>();
        for (auto entity : view)
        {
            auto& script_comp = view.template get<ScriptedBehaviorComponent>(entity);
            for (auto& script : script_comp.scripts)
            {
                assert(script.self.valid());
                if (script.stop) script.stop->operator()(script.self);
            }
        }
    }

    /// @brief Call update for all scripts
    /// @param registry 
    /// @param delta_time 
    /// Entities inside scripts that need to be destroyed are flagged for 
    /// destruction and then destroyed after this function. Destroying entities
    /// inside a view (i.e. inside scripts) while the view is iterated leads to
    /// undefined behavior: 
    /// https://github.com/skypjack/entt/issues/772#issuecomment-907814984
    void update_scripts(entt::registry& registry, float delta_time)
    {
        //std::cout << "update" << std::endl;
        auto view = registry.template view<ScriptedBehaviorComponent>();
        for (auto entity : view)
        {
            auto& script_comp = view.template get<ScriptedBehaviorComponent>(entity);
            for (auto& script : script_comp.scripts)
            {
                assert(script.self.valid());
                if (script.update) script.update->operator()(script.self, delta_time);

                // if (script.run) script.run->operator()(script.self);
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

    // Called from Lua or after adding a script in Editor mode
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
        std::cout << "Add_script " << identifier << " entity "
            << entt::to_integral(entity) << std::endl;
        assert(script_table.valid());

        BehaviorScript script =
            BehaviorScriptFactory::create_from_lua_object(registry, entity, script_table, identifier, script_path);

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

    sol::table get_script_by_entity(entt::registry& registry, const std::string& identifier, entt::entity entity)
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

// Bind the EventDispatcher to Lua
/*
Example sending Lua event from core to Lua

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    // Create the dispatcher instance
    EventDispatcher dispatcher;

    // Bind the dispatcher to Lua
    bind_conditional_observer(lua, dispatcher);

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
        dispatcher:register_callback(event_handler, "on_event1")
        dispatcher:register_callback(event_handler, "on_event2")
    )");

    // Create a reusable Lua table
    sol::table lua_data = lua.create_table();

    // Set fields and enqueue event1
    lua_data["some_key"] = "value1";
    dispatcher.enqueue_event(LuaEvent{lua_data, "on_event1"});

    // Dispatch events
    dispatcher.dispatch_all_events();

    // Reuse the same table for another event
    lua_data["some_other_key"] = "value2";
    dispatcher.enqueue_event(LuaEvent{lua_data, "on_event2"});

    // Dispatch events
    dispatcher.dispatch_all_events();

    return 0;
}
*/
void bind_conditional_observer(auto& lua, std::shared_ptr<EventDispatcher> dispatcher)
{
    lua->template new_usertype<EventDispatcher>("EventDispatcher",
        "register_callback", [](EventDispatcher& dispatcher, const sol::table& lua_table, const std::string& event_name) {
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

            dispatcher.register_callback(lua_callback);
        },
        "enqueue_event", [](EventDispatcher& dispatcher, const sol::table& lua_data, const std::string& event_name) {
            dispatcher.enqueue_event(LuaEvent{ lua_data, event_name });
        },
        "dispatch_all_events", &EventDispatcher::dispatch_all_events,
        "clear", &EventDispatcher::clear
    );

    // Ties ownership to Lua, which can be better fpr insight purposes
    lua->operator[]("dispatcher") = dispatcher;

    // Share raw pointer
    // lua->operator[]("dispatcher") = dispatcher.get();
}

// Bind BatchLoader to Lua
namespace
{
    // Function to queue assets for loading
    BatchLoader::BatchID queue_assets(BatchLoader& loader, const std::vector<std::string>& asset_names)
    {
        std::vector<std::function<bool()>> tasks;

        for (const auto& asset_name : asset_names)
        {
            // PLACEHOLDER TASK: Simulate loading an asset
            tasks.push_back([asset_name]()
                {
                    // Simulate loading success or failure
                    std::this_thread::sleep_for(std::chrono::seconds(1 + (rand() % 3)));

                    // Randomly decide success or failure
                    bool success = true; // rand() % 2 == 0;
                    if (success)
                    {
                        std::cout << "Successfully loaded asset: " << asset_name << std::endl;
                    }
                    else
                    {
                        std::cerr << "Failed to load asset: " << asset_name << std::endl;
                    }
                    return success;
                });
        }

        // Enqueue the batch and return the batch ID
        return loader.queue_batch(tasks);
    }

    // Function to check if a batch is complete
    bool is_loading_complete(BatchLoader& loader, BatchLoader::BatchID batch_id)
    {
        return loader.is_batch_complete(batch_id);
    }

    // Function to get the loading progress of a batch
    float get_loading_progress(BatchLoader& loader, BatchLoader::BatchID batch_id)
    {
        return loader.get_batch_progress(batch_id);
    }

    // Function to check if a batch succeeded
    bool did_loading_succeed(BatchLoader& loader, BatchLoader::BatchID batch_id)
    {
        return loader.did_batch_succeed(batch_id);
    }
}

void BindBatchLoader(sol::state& lua, BatchLoader& batch_loader)
{
    // Ensure the "engine" table exists
    sol::table engine = lua["engine"];
    assert(engine.valid());
    // if (!engine.valid())
    // {
    //     engine = lua.create_named_table("engine");
    // }

    // queue_assets: Manually handle Lua table to std::vector conversion
    engine.set_function("queue_assets", [&batch_loader](sol::table asset_table)
        {
            std::vector<std::string> asset_names;

            // Convert Lua table to std::vector<std::string>
            for (size_t i = 1; i <= asset_table.size(); ++i) // Lua tables are 1-indexed
            {
                sol::object value = asset_table[i];
                if (value.is<std::string>()) // Ensure the value is a string
                {
                    asset_names.push_back(value.as<std::string>());
                }
            }

            // Call the queue_assets function with the converted vector
            return queue_assets(batch_loader, asset_names);
        });

    engine.set_function("is_loading_complete", [&batch_loader](BatchLoader::BatchID batch_id)
        {
            return is_loading_complete(batch_loader, batch_id);
        });

    engine.set_function("get_loading_progress", [&batch_loader](BatchLoader::BatchID batch_id)
        {
            return get_loading_progress(batch_loader, batch_id);
        });

    engine.set_function("did_loading_succeed", [&batch_loader](BatchLoader::BatchID batch_id)
        {
            return did_loading_succeed(batch_loader, batch_id);
        });
}

namespace Inspector
{

    struct EntityInspectorVisitor
    {
        Editor::InspectorState& inspector;
        int current_level = -1;
        int closed_index = -1;

        EntityInspectorVisitor(Editor::InspectorState& inspector) :
            inspector(inspector)
        {
        }

        //void operator()(const entt::entity& entity, size_t index, size_t level)
        void visit(const Entity& entity, int level)
        {
            while (current_level >= level) { ImGui::TreePop(); current_level--; }
            if (closed_index != -1 && level > closed_index) return;

            auto& registry = inspector.context.registry;
            auto scenegraph = inspector.context.scenegraph.lock();
            auto& entity_selection = inspector.entity_selection;

            const std::string entity_name = Editor::get_entity_name(registry, entity, entt::resolve<HeaderComponent>());
            const std::string label = "[entity#" + std::to_string(entity.to_integral()) + "] " + entity_name;

            bool is_selected = entity_selection.contains(entity);
            bool is_leaf = scenegraph->get_nbr_children(entity) == 0;

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
            if (is_leaf) flags |= ImGuiTreeNodeFlags_Leaf;
            if (is_selected) flags |= ImGuiTreeNodeFlags_Selected;

            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx(label.c_str(), flags))
            {
                current_level = level;
                closed_index = -1;

                if (ImGui::IsItemClicked())
                {
                    if (bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_ModCtrl); ctrl_pressed)
                    {
                        // Multi-selection with Ctrl: toggle selection state
                        is_selected ? entity_selection.remove(entity) : entity_selection.add(entity);
                    }
                    else
                    {
                        // Single selection: clear previous selections and select this entity
                        entity_selection.clear();
                        entity_selection.add(entity);
                    }
                }
            }
            else
            {
                closed_index = level;
            }
        }

        void operator()(const Entity& entity, size_t index, size_t level)
        {
            visit(entity, (int)level);
        }

        ~EntityInspectorVisitor()
        {
            while (current_level >= 0) {
                ImGui::TreePop();
                current_level--;
            }
        }
    };

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
#if 0
        label = "[entity#" + std::to_string(entity.to_integral()) + "] " + label;
#else
        label = "entity#" + std::to_string(entity.to_integral())
            + " (index: " + std::to_string(index) + ", children: " + std::to_string(nbr_children) +
            ", branch_stride: " + std::to_string(branch_stride) + ", parent_ofs: " + std::to_string(parent_ofs) + ") "
            + label;
#endif

        //bool is_selected = inspector.selected_entity == entity;
        auto& entity_selection = inspector.entity_selection;
        bool is_selected = entity_selection.contains(entity);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
        if (!nbr_children) flags |= ImGuiTreeNodeFlags_Leaf;
        if (is_selected) flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNodeEx(label.c_str(), flags))
        {
            // if (ImGui::IsItemClicked())
            //     inspector.selected_entity = entity;

            if (ImGui::IsItemClicked())
            {
                if (bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_ModCtrl); ctrl_pressed)
                {
                    // Multi-selection with Ctrl: toggle selection state
                    if (is_selected)
                        // Deselect
                        entity_selection.remove(entity);
                    else
                        // Add to selection
                        entity_selection.add(entity);
                }
                else {
                    // Single selection: clear previous selections and select this entity
                    entity_selection.clear();
                    entity_selection.add(entity);
                }
            }

            // Recursively display each child node
            int child_index = index + 1;
            for (int i = 0; i < nbr_children; ++i)
            {
                inspect_scene_graph_node(scenegraph, inspector, child_index);

                auto [entity, nbr_children, branch_stride, parent_ofs] = scenegraph.tree.get_node_info_at(child_index);
                child_index += branch_stride;
            }

            ImGui::TreePop();
        }
    }

    // Or a "window" function that does SG + buttons for New, Copy etc
    void inspect_scenegraph(
        SceneGraph& scenegraph,
        Editor::InspectorState& inspector,
        EventDispatcher& dispatcher)
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

        bool has_selection = !inspector.entity_selection.empty();
        bool has_multi_selection = inspector.entity_selection.size() > 1;

        // New entity
        if (ImGui::Button("New"))
        {
            Entity entity_parent;
            if (has_selection) entity_parent = inspector.entity_selection.last();
            CreateEntityEvent event{ .parent_entity = entity_parent };
            dispatcher.enqueue_event(event);
        }

        // Destroy selected entities
        ImGui::SameLine();
        if (!has_selection) inspector.begin_disabled();
        if (ImGui::Button("Delete"))
        {
            DestroyEntitySelectionEvent event{ .entity_selection = inspector.entity_selection };
            dispatcher.enqueue_event(event);
            inspector.entity_selection.clear();
        }
        if (!has_selection) inspector.end_disabled();

        // Copy selected entities
        ImGui::SameLine();
        if (!has_selection) inspector.begin_disabled();
        if (ImGui::Button("Copy"))
        {
            // Scene::CopyEntityEvent event{ .entity = inspector.entity_selection.last() }; // last
            CopyEntitySelectionEvent event{ inspector.entity_selection };
            dispatcher.enqueue_event(event);
        }
        if (!has_selection) inspector.end_disabled();

        // Reparent selected entities
        ImGui::SameLine();
        if (!has_multi_selection) inspector.begin_disabled();
        if (ImGui::Button("Parent"))
        {
            SetParentEntitySelectionEvent event{ .entity_selection = inspector.entity_selection };
            dispatcher.enqueue_event(event);
        }
        if (!has_multi_selection) inspector.end_disabled();

        // Unparent selected entities (set them as roots)
        ImGui::SameLine();
        if (!has_selection) inspector.begin_disabled();
        if (ImGui::Button("Unparent"))
        {
            UnparentEntitySelectionEvent event{ .entity_selection = inspector.entity_selection };
            dispatcher.enqueue_event(event);
        }
        if (!has_selection) inspector.end_disabled();

#if 0
        // Explicit traverse
        ImGui::SameLine();
        if (ImGui::Button("Traverse"))
        {
            scenegraph.traverse(registry);
        }
#endif

        // Scene graph
        if (scenegraph.size())
        {
#if 1
            scenegraph.tree.traverse_depthfirst(EntityInspectorVisitor(inspector));
#else
            // For all roots ...
            size_t i = 0;
            while (i < scenegraph.tree.size())
            {
                inspect_scene_graph_node(scenegraph, inspector, i);
                i += scenegraph.tree.nodes[i].m_branch_stride;
            }
#endif
        }

        // Debug print selected
        ImGui::Separator();
        if (inspector.entity_selection.size())
        {
            ImGui::Text("Selected Entities (in order):");
            std::stringstream ss;
            for (auto entity : inspector.entity_selection.get_all())
                //ss << std::to_string(entt::to_integral(entity)) << " ";
                //ss << Editor::get_entity_name(registry, entity, entt::resolve<HeaderComponent>()) << " ";
                ImGui::Text("Entity %u", entity.to_integral());
            // ImGui::Text("%s", ss.str().c_str());
        }

        ImGui::End(); // Window

        // TEMP
        // if (!inspector.entity_selection.empty())
        //     inspector.selected_entity = inspector.entity_selection.last();
    }

    bool inspect_entity(
        Editor::InspectorState& inspector,
        EventDispatcher& dispatcher)
    {
        auto& registry = inspector.context.registry;
        bool mod = false;
        static bool open = true;
        bool* p_open = &open;

        Entity selected_entity;
        if (!inspector.entity_selection.empty()) selected_entity = inspector.entity_selection.first();
        bool selected_entity_valid =
            !selected_entity.is_null() &&
            registry->valid(selected_entity);

        ImGui::SetNextWindowBgAlpha(0.35f);
        if (!ImGui::Begin("Entity Inspector", p_open))
        {
            ImGui::End();
            return mod;
        }

        ImGui::Text("Add/Remove Component");

        // Component combo
        static entt::id_type selected_comp_id = 0;
        {
            const auto get_comp_name = [](entt::id_type comp_id) -> std::string
                {
                    return meta_type_name(entt::resolve(comp_id));
                };
            std::string selected_entry_label = "";
            if (selected_comp_id) selected_entry_label = get_comp_name(selected_comp_id);

            if (ImGui::BeginCombo("##addcompcombo", selected_entry_label.c_str()))
            {
                // For all meta types
                for (auto&& [id_, type] : entt::resolve())
                {
                    // Skip non-component types
                    if (auto is_component = get_meta_type_prop<bool, false>(type, "is_component"_hs); !is_component)
                        continue;

                    // Note: 
                    // id_ is a hash of the mangled (fully qualified) type name
                    // type.id() is a hash of the name hash given by entt::meta<T>.type(...)
                    // If these mismatch, entt::resolve will give a correct meta type only for the latter
                    auto id = type.id();

                    bool is_selected = (id == selected_comp_id);
                    std::string label = get_comp_name(id); // Display name, if present, or mangled name

                    if (ImGui::Selectable(label.c_str(), is_selected))
                        selected_comp_id = id;

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        // Add Component button
        ImGui::SameLine();
        if (ImGui::Button("Add##addcomponent") && selected_comp_id)
        {
            AddComponentToEntitySelectionEvent event{ selected_comp_id, inspector.entity_selection };
            dispatcher.enqueue_event(event);
        }

        // Remove Component button
        ImGui::SameLine();
        if (ImGui::Button("Remove##removecomponent") && selected_comp_id)
        {
            RemoveComponentFromEntitySelectionEvent event{ selected_comp_id, inspector.entity_selection };
            dispatcher.enqueue_event(event);
        }

        ImGui::Text("Add/Remove Behavior");

        // Scripts combo
        auto all_scripts = FileManager::GetFilesInFolder(script_dir, "lua");
        static std::string selected_script_path = "";
        if (ImGui::BeginCombo("##addscriptcombo", selected_script_path.c_str()))
        {
            for (auto& script_path : all_scripts)
            {
                bool is_selected = (script_path == selected_script_path);

                if (ImGui::Selectable(script_path.c_str(), is_selected))
                    selected_script_path = script_path;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Add script
        ImGui::SameLine();
        if (ImGui::Button("Add##addscript") && selected_script_path.size())
        {
            AddScriptToEntitySelectionEvent event{ selected_script_path, inspector.entity_selection };
            dispatcher.enqueue_event(event);
        };

        // Remove script
        ImGui::SameLine();
        ImGui::BeginDisabled();
        if (ImGui::Button("Remove##removescript") && selected_script_path.size())
        {
            RemoveScriptFromEntitySelectionEvent event{ selected_script_path, inspector.entity_selection };
            dispatcher.enqueue_event(event);
        };
        ImGui::EndDisabled();

        // Component inspector
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
                    mod |= Editor::inspect_entity(selected_entity, inspector);
                }
                else
                    ImGui::Text("Selected entity is null or invalid");

                ImGui::EndTable();
            }
            ImGui::TreePop(); // Entities node
            ImGui::PopStyleVar();
        }
        ImGui::End(); // Window

        return mod;
    }

    void inspect_command_queue(
        Editor::InspectorState& inspector)
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

    void inspect_playstate(
        const GamePlayState& play_state,
        EventDispatcher& dispatcher)
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
        bool can_play = play_state != GamePlayState::Play;
        bool can_stop = play_state != GamePlayState::Stop;
        bool can_pause = play_state == GamePlayState::Play;

        // Play button
        if (!can_play) ImGui::BeginDisabled();
        if (ImGui::Button("Play##playpause"))
            dispatcher.enqueue_event(SetGamePlayStateEvent{ GamePlayState::Play });
        if (!can_play) ImGui::EndDisabled();

        // Pause button
        ImGui::SameLine();
        if (!can_pause) ImGui::BeginDisabled();
        if (ImGui::Button("Pause##playpause"))
            dispatcher.enqueue_event(SetGamePlayStateEvent{ GamePlayState::Pause });
        if (!can_pause) ImGui::EndDisabled();

        // Stop button
        ImGui::SameLine();
        if (!can_stop) ImGui::BeginDisabled();
        if (ImGui::Button("Stop##playpause"))
            dispatcher.enqueue_event(SetGamePlayStateEvent{ GamePlayState::Stop });
        if (!can_stop) ImGui::EndDisabled();

        ImGui::End(); // Window
    }

    void inspect_script_launcher(
        EventDispatcher& dispatcher)
    {
        static bool open = true;
        bool* p_open = &open;

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (!ImGui::Begin("Run Script", p_open))
        {
            ImGui::End();
            return;
        }

        // Scripts combo
        auto all_scripts = FileManager::GetFilesInFolder(script_dir, "lua");
        static std::string selected_script_path = "";
        if (ImGui::BeginCombo("##runscriptcombo", selected_script_path.c_str()))
        {
            for (auto& script_path : all_scripts)
            {
                bool is_selected = (script_path == selected_script_path);

                if (ImGui::Selectable(script_path.c_str(), is_selected))
                    selected_script_path = script_path;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Run button
        bool can_run = selected_script_path.size();
        ImGui::SameLine();
        if (!can_run) ImGui::BeginDisabled();
        if (ImGui::Button("Run##runscript"))
            dispatcher.enqueue_event(RunScriptEvent{ selected_script_path });
        if (!can_run) ImGui::EndDisabled();

        ImGui::End(); // Window
    }

    void inspect_chunkregistry(
        ChunkRegistry& chunk_registry,
        EventDispatcher& dispatcher)
    {
        static bool open = true;
        bool* p_open = &open;

        static std::string selected_chunk_tag;
        // static std::string load_json_path = "..."; //

        ImGui::SetNextWindowBgAlpha(0.35f);
        if (!ImGui::Begin("Scene Chunks", p_open))
        {
            ImGui::End();
            return;
        }

        // Existing files combo
        //ImGui::InputText("##loadjson", &load_json_path, 0, nullptr, nullptr);
        auto all_files = FileManager::GetFilesInFolder(save_dir, "json");
        static std::string selected_file_path = "";
        if (ImGui::BeginCombo("##chunkcombo", selected_file_path.c_str()))
        {
            for (auto& file_path : all_files)
            {
                bool is_selected = (file_path == selected_file_path);

                if (ImGui::Selectable(file_path.c_str(), is_selected))
                    selected_file_path = file_path;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Load JSON
        ImGui::SameLine();
        if (ImGui::Button("Load") && selected_file_path.size())
        {
            dispatcher.enqueue_event(LoadChunkFromFileEvent{ selected_file_path });
        };

        // Chunk list
        static std::string label;
        ImGui::Text("Loaded Chunks");
        ImGui::BeginChild("ChunkList", ImVec2(0, 100), true);
        for (auto& [chunk_tag, entities] : chunk_registry.chunks())
        {
            bool is_selected = (selected_chunk_tag == chunk_tag);
            label = chunk_tag + " (" + std::to_string(entities.size()) + ")";

            if (ImGui::Selectable(label.c_str(), is_selected))
                selected_chunk_tag = chunk_tag;
        }
        ImGui::EndChild(); // Chunk list window

        if (ImGui::Button("Save") && selected_chunk_tag.size())
        {
            dispatcher.enqueue_event(SaveChunkToFileEvent{ selected_chunk_tag });
        };

        ImGui::SameLine();
        if (ImGui::Button("Save All"))
        {
            dispatcher.enqueue_event(SaveAllChunksToFileEvent{ });
        };

        ImGui::SameLine();
        if (ImGui::Button("Unload") && selected_chunk_tag.size())
        {
            dispatcher.enqueue_event(UnloadChunkEvent{ selected_chunk_tag });
        };

        ImGui::SameLine();
        if (ImGui::Button("Unload All"))
        {
            dispatcher.enqueue_event(UnloadAllChunksEvent{ });
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

// TODO -> SG ???
// entt::entity Scene::get_entity_parent(
//     entt::entity entity)
// {
//     assert(registry->all_of<HeaderComponent>(entity));
//     auto& header = registry->get<HeaderComponent>(entity);
//     auto parent_entity = entt::entity{ header.entity_parent };

//     // SH might not be synched yet - this function is used by operations (copy) that register manually
//     // if (parent_entity == entt::null) { assert(scenegraph->is_root(entity)); }
//     // else { assert(parent_entity == scenegraph->get_parent(entity)); }

//     return parent_entity;
// }

bool Scene::entity_valid(const Entity& entity)
{
    return registry->valid(entity);
}

bool Scene::entity_parent_registered(const Entity& entity)
{
    assert(registry->all_of<HeaderComponent>(entity));
    auto& header = registry->get<HeaderComponent>(entity);
    auto entity_parent = Entity{ entt::entity{header.entity_parent} };
    // auto entity_parent = get_entity_parent(entity);

    if (entity_parent.is_null()) return true;
    return scenegraph->tree.contains(entity_parent);
}

void Scene::reparent_entity(const Entity& entity, const Entity& parent_entity)
{
    assert(registry->all_of<HeaderComponent>(entity));
    registry->get<HeaderComponent>(entity).entity_parent = parent_entity;

    scenegraph->reparent(entity, parent_entity);
}

void Scene::set_entity_header_parent(const Entity& entity, const Entity& entity_parent)
{
    assert(registry->all_of<HeaderComponent>(entity));
    registry->get<HeaderComponent>(entity).entity_parent = entity_parent;

    // register_entity(entity);
}

void Scene::register_entity(const Entity& entity)
{
    assert(registry->all_of<HeaderComponent>(entity));

    auto& header = registry->get<HeaderComponent>(entity);
    auto& chunk_tag = header.chunk_tag;
    auto entity_parent = Entity{ entt::entity{header.entity_parent} };

    chunk_registry.add_entity(header.chunk_tag, entity);

    if (entity_parent.is_null())
    {
        scenegraph->insert_node(entity);
    }
    else
    {
        assert(scenegraph->tree.contains(entity_parent));
        scenegraph->insert_node(entity, entity_parent);
    }
}

Entity Scene::create_empty_entity(const Entity& entity_hint)
{
    if (entity_hint.is_null())
        return Entity{ registry->create() };

    Entity entity = Entity{ registry->create(entity_hint) };
    assert(entity == entity_hint);
    return entity;
}

Entity Scene::create_entity(
    const std::string& chunk_tag,
    const std::string& name,
    const Entity& entity_parent,
    const Entity& entity_hint)
{
    Entity entity = create_empty_entity(entity_hint);

    std::string used_name = name.size() ? name : std::to_string(entity.to_integral());
    std::string used_chunk_tag = chunk_tag.size() ? chunk_tag : "default_chunk";
    uint32_t guid = 0;
    registry->emplace<HeaderComponent>(entity, used_name, used_chunk_tag, guid, entity_parent);

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

    std::cout << "Scene::create_entity " << entity.to_integral() << std::endl;
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

void Scene::queue_entity_for_destruction(const Entity& entity)
{
    // entities_pending_destruction.push_back(entity);
    entities_pending_destruction.push_back(entity);
}

void Scene::destroy_pending_entities()
{
    int count = 0;
    while (entities_pending_destruction.size())
    {
        auto entity = entities_pending_destruction.front();
        entities_pending_destruction.pop_front();

        // Remove from chunk registry
        assert(registry->valid(entity));
        assert(registry->all_of<HeaderComponent>(entity));
        chunk_registry.remove_entity(registry->get<HeaderComponent>(entity).chunk_tag, entity);

        // Remove from scene graph
        if (scenegraph->tree.contains(entity))
            scenegraph->erase_node(entity);

        // Destroy entity. May lead to additional entities being added to the queue.
        // registry->destroy(entity);
        registry->destroy(entity, 0); // TODO: This is a fix to keep generations of entities equal

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

    // Register Entity
    {
        const auto from_json = [](const nlohmann::json& j, void* ptr, const Entity& container_entity, Editor::Context& context)
            {
                // Deserialize entity
                auto entity = Entity{ j["id"].get<Entity::entity_type>() };

                // Do remap if another entity is available
                if (context.entity_remap.contains(entity))
                    entity = context.entity_remap.at(entity);

                // Assign entity to object
                *static_cast<Entity*>(ptr) = entity;
            };

        entt::meta<Entity>()
            .type("Entity"_hs).prop(display_name_hs, "Entity")

            .data<&Entity::set_id, &Entity::get_id>("id"_hs)
            .prop(display_name_hs, "id")

            // To/from JSON
            // .template func<to_json, entt::as_void_t>(to_json_hs)
            .template func<from_json, entt::as_void_t >(from_json_hs)

            // Inspection
            //.template func<inspect>(inspect_hs) // <- combo maybe
            ;
    }

    // Register GamePlayState
    {
        entt::meta<GamePlayState>()
            .type("GamePlayState"_hs)
            .prop(display_name_hs, "GamePlayState")
            .prop("underlying_meta_type"_hs, entt::resolve<std::underlying_type_t<GamePlayState>>())

            .data<GamePlayState::Play>("Play"_hs).prop(display_name_hs, "Play")
            .data<GamePlayState::Stop>("Stop"_hs).prop(display_name_hs, "Stop")
            .data<GamePlayState::Pause>("Pause"_hs).prop(display_name_hs, "Pause");
    }

    // Create registry
    registry = std::make_shared<entt::registry>();
    registry->on_destroy<ScriptedBehaviorComponent>().connect<&release_script>();

    scenegraph = std::make_shared<SceneGraph>();
    cmd_queue = std::make_shared<Editor::CommandQueue>();
    dispatcher = std::make_shared<EventDispatcher>();
    // loader = std::make_shared<BatchLoader>(thread_pool);

    // Hook up Scene events
    dispatcher->register_callback([&](const SaveChunkToFileEvent& event) { this->OnSaveChunkToFileEvent(event); });
    dispatcher->register_callback([&](const SaveAllChunksToFileEvent& event) { this->OnSaveAllChunksToFileEvent(event); });

    dispatcher->register_callback([&](const SetGamePlayStateEvent& event) { this->OnSetGamePlayStateEvent(event); });
    dispatcher->register_callback([&](const UnloadChunkEvent& event) { this->OnUnloadChunkEvent(event); });
    dispatcher->register_callback([&](const UnloadAllChunksEvent& event) { this->OnUnloadAllChunksEvent(event); });

    dispatcher->register_callback([&](const LoadChunkFromFileEvent& event) { this->OnLoadChunkFromFileEvent(event); });
    dispatcher->register_callback([&](const CreateEntityEvent& event) { this->OnCreateEntityEvent(event); });
    dispatcher->register_callback([&](const DestroyEntitySelectionEvent& event) { this->OnDestroyEntitySelectionEvent(event); });
    // dispatcher.register_callback([&](const CopyEntityEvent& event) { this->OnCopyEntityEvent(event); });
    dispatcher->register_callback([&](const CopyEntitySelectionEvent& event) { this->OnCopyEntitySelectionEvent(event); });

    dispatcher->register_callback([&](const SetParentEntitySelectionEvent& event) { this->OnSetParentEntitySelectionEvent(event); });
    dispatcher->register_callback([&](const UnparentEntitySelectionEvent& event) { this->OnUnparentEntitySelectionEvent(event); });

    dispatcher->register_callback([&](const AddComponentToEntitySelectionEvent& event) { this->OnAddComponentToEntitySelectionEvent(event); });
    dispatcher->register_callback([&](const RemoveComponentFromEntitySelectionEvent& event) { this->OnRemoveComponentFromEntitySelectionEvent(event); });

    dispatcher->register_callback([&](const AddScriptToEntitySelectionEvent& event) { this->OnAddScriptToEntitySelectionEvent(event); });
    dispatcher->register_callback([&](const RemoveScriptFromEntitySelectionEvent& event) { this->OnRemoveScriptFromEntitySelectionEvent(event); });

    dispatcher->register_callback([&](const ChunkModifiedEvent& event) { this->OnChunkModifiedEvent(event); });

    dispatcher->register_callback([&](const RunScriptEvent& event) { this->OnRunScriptEvent(event); });

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
            entt::entity parent_entity) -> entt::entity {
                // return create_entity_and_attach_to_scenegraph(parent_entity);
                return entt::entity{ create_entity(chunk_tag, name, Entity{ parent_entity }, Entity{}) };
            };

        // Destroy entity
        //
        lua->operator[]("engine")["destroy_entity"] = [&](entt::entity entity) {
            assert(registry->valid(entity));
            queue_entity_for_destruction(Entity{ entity });
            };

        // Register to Lua: helper functions for adding & obtaining scripts from entities
        lua->operator[]("engine")["add_script"] = [&](
            entt::registry& registry,
            entt::entity entity,
            const std::string& script_name) {
                return add_script_from_file(registry, entity, lua, script_dir, script_name);
            };

        lua->operator[]("engine")["get_script_by_entity"] = &get_script_by_entity;

        lua->operator[]("engine")["get_script_by_entity_name"] = [&](
            const std::string& script_name,
            const std::string& entity_name) -> sol::table
            {
                auto entity = chunk_registry.get_entity_by_name(entity_name, [&](
                    const Entity& entity) -> std::string
                    {
                        assert(registry->all_of<HeaderComponent>(entity));
                        return registry->get<HeaderComponent>(entity).name;
                    });
                return get_script_by_entity(*registry, script_name, entity);
            };

#if 0
        // Register get_entity_by_name function
        lua->operator[]("engine")["get_entity_by_name"] = [&](const std::string& name) -> entt::entity
            {
                return chunk_registry.get_entity_by_name(name, [&](const Entity& entity) -> std::string
                    {
                        assert(registry->all_of<HeaderComponent>(entity));
                        return registry->get<HeaderComponent>(entity).name;
                    });
            };
#endif

        lua->operator[]("engine")["load_chunk"] = [&](const std::string& chunk_tag)
            {
                OnLoadChunkFromFileEvent(LoadChunkFromFileEvent{ chunk_tag });
            };

        lua->operator[]("engine")["unload_chunk"] = [&](const std::string& chunk_tag)
            {
                OnUnloadChunkEvent(UnloadChunkEvent{ chunk_tag });
            };

        // lua->operator[]("engine")["save_chunk"] = [&](const std::string& chunk_tag)
        //     {
        //         OnSaveChunkToFileEvent(SaveChunkToFileEvent{ chunk_tag });
        //     };

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
        auto context = create_context();
        register_meta<Transform>(context);
        register_meta<HeaderComponent>(context);
        register_meta<CircleColliderGridComponent>(context);
        register_meta<IslandFinderComponent>(context);
        register_meta<QuadGridComponent>(context);
        register_meta<DataGridComponent>(context);
        register_meta<ScriptedBehaviorComponent>(context);
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

        // Bind dispatcher
        bind_conditional_observer(lua, dispatcher);

        // Bind asset loading stuff
        BindBatchLoader(*lua, loader);

        // Load & execute init script
        lua->safe_script_file(script_dir + "init.lua");

        // Validate & init the game table
        {
#if 0
            auto lua_game = lua->operator[]("game");
            assert(lua_game.valid());
            assert(lua_game["init"].valid());
            assert(lua_game["destroy"].valid());
            lua_game["init"](lua_game);
#endif
        }
        // lua["game"]["destroy"]();

        // - Lua binding done -

        // SERIALIZATION TEST

        {
            serialization_test(lua);

        }

        // Dump Lua game & engine states
        {
#if 0
            auto lua_game = lua->operator[]("game");
            std::cout << "Inspect Lua game state:" << std::endl;
            dump_lua_state(lua, lua_game, "    ");
#endif

            auto lua_engine = lua->operator[]("engine");
            std::cout << "Inspect Lua engine state:" << std::endl;
            dump_lua_state(lua, lua_engine, "    ");
        }

        // Debugging inspection
        // auto debug_entity = create_entity_and_attach_to_scenegraph();
        auto debug_entity = create_entity("", "", Entity{}, Entity{});// registry.create();
        registry->template emplace<DebugClass>(debug_entity);

#if 0
        // Send event C++ <-> C++
        struct MyEvent { float x; } event{ 5.0f };
        dispatcher.register_callback([](const MyEvent& e) { std::cout << "C++: MyEvent: " << e.x << std::endl; });
        dispatcher.enqueue_event(event);
        dispatcher.dispatch_all_events();
        dispatcher.clear();

        // C++ listens to event sent from Lua (ever needed?)
        dispatcher.register_callback([](const LuaEvent& e) { std::cout << "C++: LuaEvent: " << e.event_name << std::endl; });

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
        dispatcher:register_callback(event_handler, "on_event1")
        dispatcher:register_callback(event_handler, "on_event2")

        -- Enqueue events
        dispatcher:enqueue_event({ some_key = "value1" }, "on_event1")
        dispatcher:enqueue_event({ some_other_key = "value2" }, "on_event2")

        -- Dispatch events
        dispatcher:dispatch_all_events()

        -- Clear events
        dispatcher:clear()
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
        eeng::LogError("%s", e.what());
        // std::cerr << "Exception: " << e.what();
        //throw std::runtime_error(e.what());
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
#if 0
    update_scripts(*registry, deltaTime_s);
#else
    if (play_state == GamePlayState::Play)
        update_scripts(*registry, deltaTime_s);
#endif


    // Destroy pending entities
    // After script update because: scripts may remove entities
    destroy_pending_entities();

    // Update scene graph
    // After script update because: scripts may alter positions
    scenegraph->traverse(registry);

    // Update particles
    particleBuffer.update(deltaTime_s);

    // Debug print
    if (entities_pending_destruction.size())
    {
        std::cout << "Destroying " << (int)entities_pending_destruction.size() << " entities... ";
        for (auto entity : entities_pending_destruction) std::cout << entity.to_integral() << " ";
        std::cout << std::endl;
    }

    // Placeholder collision system
#if 1
    if (play_state == GamePlayState::Play)
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
                    if (script.on_collision)
                        script.on_collision->operator()(script.self, x, y, nx, ny, collider_index, other_entity);
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

                //std::cout << "(" << entt::to_integral(entity1) << "," << entt::to_integral(entity2) << ") ";
                // std::cout << collider1.active_indices.get_dense_count() << "-" << collider2.active_indices.get_dense_count() << ",";

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

    if (play_state == GamePlayState::Play)
        IslandFinderSystem(registry, deltaTime_s);

    dispatcher->dispatch_all_events();
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

        scenegraph->tree.clear();

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
    inspector.entity_selection.remove_invalid([&](const Entity& entity)
        {
            return !entity.is_null() && registry->valid(entity);
        });

    // Begin inspection
    // May issue events

    if (Inspector::inspect_entity(inspector, *dispatcher)) {}

    Inspector::inspect_command_queue(inspector);

    // Before inspect_entity ???
    Inspector::inspect_scenegraph(*scenegraph, inspector, *dispatcher);

    Inspector::inspect_playstate(play_state, *dispatcher);

    Inspector::inspect_script_launcher(*dispatcher);

    Inspector::inspect_chunkregistry(chunk_registry, *dispatcher);

    // Event & command loop
    // TODO: Move to start of update?

    event_loop();
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

    // std::cout << "Entities pending destruction " << entities_pending_destruction.size() << std::endl;
    // std::cout << "Destroying game..." << std::endl;
    // {
    //     auto lua_game = lua->operator[]("game");
    //     lua_game["destroy"](lua_game); // <- entities flagged for destruction ???
    // }
    unload_all_chunks();
    event_loop();

    // std::cout << "Command queue size " << cmd_queue->execute_pending << std::endl;

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
    scenegraph->dump_to_cout(registry, entt::resolve<HeaderComponent>());

    // Doesn't need to be explicit
    std::cout << "cmd_queue.use_count() " << cmd_queue.use_count() << std::endl;
    std::cout << "registry.use_count() " << registry.use_count() << std::endl;
    std::cout << "lua.use_count() " << lua.use_count() << std::endl;
    std::cout << "dispatcher.use_count() " << dispatcher.use_count() << std::endl;
    cmd_queue.reset();
    scenegraph.reset();
    registry.reset();
    dispatcher.reset();
    lua.reset();

    is_initialized = false;
    std::cout << "Done: Scene::destroy()" << std::endl;
}

void Scene::event_loop()
{
    int cycles = 0;
    const int max_cycles = 5;

    while ((dispatcher->has_pending_events() ||
        cmd_queue->has_new_commands_pending() ||
        dispatcher->has_pending_events())
        && cycles++ <= max_cycles)
    {
        // Dispatch events. May lead to commands being issued.
        dispatcher->dispatch_all_events();

        // Execute commands. May lead to entities being queued for destruction
        // and new events being issued.
        if (cmd_queue->has_new_commands_pending())
            cmd_queue->execute_pending();

        // Destroy entities flagged for destruction.
        // May lead to additional entities being flagged for destruction.
        destroy_pending_entities();
    }
    if (cycles > 1) std::cout << "Event loop cycles " << cycles << std::endl;
    assert(cycles <= max_cycles);
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
        scenegraph,
        dispatcher,
        // Create entity
        [&](const Entity& entity_parent, const Entity& entity_hint) -> Entity {
            return this->create_entity("", "", entity_parent, entity_hint);
        },
        // Create empty entity
        [&](const Entity& entity_hint) -> Entity {
            return this->create_empty_entity(entity_hint);
        },
        // Destroy_entity
        [&](const Entity& entity) -> void {
            this->queue_entity_for_destruction(entity);
        },
        // Can register entity
        [&](const Entity& entity) -> bool {
            return this->entity_parent_registered(entity);
            },
        // Register entity
        [&](const Entity& entity) -> void {
            this->register_entity(entity);
            },
        // Reparent entity
        [&](const Entity& entity, const Entity& parent_entity) -> void {
            this->reparent_entity(entity, parent_entity);
        },
        // Set entity header parent
        [&](const Entity& entity, const Entity& entity_parent) -> void {
             this->set_entity_header_parent(entity, entity_parent);
        },
        // Entity validity
        [&](const Entity& entity) -> bool {
            return this->entity_valid(entity);
        }
    };
}

void Scene::save_chunk(const std::string& chunk_id)
{
    // Extract chunk entities
    auto chunk_it = chunk_registry.chunk(chunk_id);
    std::vector<Entity> entities(chunk_it.begin(), chunk_it.end());

    // Serialize entities
    nlohmann::json entities_json =
        Meta::serialize_entities(entities.data(), entities.size(), registry);

    // Create JSON object
    nlohmann::json chunk_json;
    chunk_json["chunk"] = chunk_id;
    chunk_json["entities"] = entities_json;

    // Save JSON to file
    std::string file_path = save_dir + chunk_id + ".json";
    eeng::Log("Saving %s...", file_path.c_str());
    FileManager::SaveToFile(file_path, chunk_json.dump(4));
}

void Scene::save_all_chunks()
{
    for (auto& chunk : chunk_registry.chunks()) save_chunk(chunk.first);
}

// TODO: Command based. If used in e.g. destroy, must run execute + destroy_pending_entities()
void Scene::unload_chunk(const std::string& chunk_id)
{
    DestroyEntitySelectionEvent event;

    for (auto& entity : chunk_registry.chunk(chunk_id))
    {
        event.entity_selection.add(entity);

        // Either, 
        // 1) rely on entt's onDestroy event for calling destroy() on scripts
        // queue_entity_for_destruction(entity);

        // 2) Destroy scripts explicitly <- then BehaviorScript should store destroy()
        // ...
    }

    OnDestroyEntitySelectionEvent(event);
    // cmd_queue->execute_pending();
    // destroy_pending_entities();
}

void Scene::unload_all_chunks()
{
    for (auto& chunk : chunk_registry.chunks()) unload_chunk(chunk.first);

    // Can't do this until entities are actually unloaded
    // chunk_registry.clear();
}

void Scene::load_chunk(const std::string& path)
{
    // Load the file content into a string
    std::string fileContent;
    if (!FileManager::LoadFromFile(path, fileContent))
    {
        eeng::LogError("Failed to load file %s", path.c_str());
        return;
    }

    // Parse JSON
    nlohmann::json file_json;
    try
    {
        // Parse the string content into a nlohmann::json object
        file_json = nlohmann::json::parse(fileContent);
        //std::cout << "JSON data loaded successfully:\n" << jsonData.dump(4) << std::endl;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        eeng::LogError("Failed to parse JSON %s", e.what());
        return;
    }

    // Fetch elements
    assert(file_json.contains("entities"));
    assert(file_json.contains("chunk"));
    auto chunk_id = file_json["chunk"].get<std::string>();
    assert(chunk_id.size());
    nlohmann::json entities_json = file_json["entities"];

    // Deserialize entities
    // if (!chunk_registry.chunk_exists(chunk_id)) chunk_registry.create_chunk(chunk_id);
    auto context = create_context();
    Meta::deserialize_entities(entities_json, context);

    // Quick fix to keep command state persistent
    // cmd_queue->remove_pending();
    cmd_queue->clear();

    eeng::Log("Loaded chunk %s from file %s", chunk_id.c_str(), path.c_str());
}

void Scene::OnSaveChunkToFileEvent(const SaveChunkToFileEvent& event)
{
    eeng::Log("SaveChunkToFileEvent: %s", event.chunk_tag.c_str());

    save_chunk(event.chunk_tag);
}

void Scene::OnSaveAllChunksToFileEvent(const SaveAllChunksToFileEvent& event)
{
    eeng::Log("SaveAllChunksToFileEvent");

    save_all_chunks();
}

void Scene::OnUnloadChunkEvent(const UnloadChunkEvent& event)
{
    eeng::Log("DestroyChunkEvent: %s", event.chunk_tag.c_str());

    // COMMAND ?

    unload_chunk(event.chunk_tag);
}

void Scene::OnUnloadAllChunksEvent(const UnloadAllChunksEvent& event)
{
    // COMMAND ?

    unload_all_chunks();
}

void Scene::OnLoadChunkFromFileEvent(const LoadChunkFromFileEvent& event)
{
    eeng::Log("LoadFileEvent: %s", event.path.c_str());

    // COMMAND ?

    load_chunk(event.path);
}

void Scene::OnSetGamePlayStateEvent(const SetGamePlayStateEvent& event)
{
    // COMMAND ?

    auto j_play_state = Meta::serialize_any(event.play_state);
    assert(!j_play_state.is_null());
    eeng::Log("SetGamePlayStateEvent: %s", j_play_state.dump().c_str());

    auto new_playstate = event.play_state;
    if (new_playstate == GamePlayState::Play)
    {
        // save_all_chunks();

        run_scripts(*registry);
    }
    else if (new_playstate == GamePlayState::Stop)
    {
        // unload_all_chunks();

        stop_scripts(*registry);
    }

    play_state = event.play_state;


    // (Editor -> Play)
    // 1. save_all_chunks()
    // 2. Somehow keep track of which chunks (JSONs) are loaded (wip stuff, editor chunks ...)
    //      - Push & Pop in ChunkRegistry?
    // 3. unload_all_chunks()
    // (We are here if the editor is skipped)
    // 4. Load whick chunks?
    //      For example: game_chunk (Player, PhaseManager, ProjectilePool ...) + level1_chunk (enemies)
    //      Can have a property in the 
    // 5. Change to Play mode
    // ...
    // run update() for scripts

    // (Play -> Editor)
    // 1. unload_all_chunks()
    // 2. load_chunk() for all files stored since switching to Play
    // 3. Change to Editor mode
    // ...
    // run editor_update() for scripts


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
}

void Scene::OnCreateEntityEvent(const CreateEntityEvent& event)
{
    eeng::Log("CreateEntityEvent");

    using namespace Editor;
    auto command = CreateEntityCommand{ event.parent_entity, create_context() };
    cmd_queue->add(CommandFactory::Create<CreateEntityCommand>(command));
}

namespace
{
    bool is_child_of(SceneGraph& sg, const Entity& entity_child, const Entity& entity_parent)
    {
        return sg.tree.is_descendant_of(entity_child, entity_parent);

        // bool is_child = false;
        // sg.tree.ascend(entity_child, [&](auto& entity, size_t index) {
        //     if (entity == entity_child) return;
        //     if (entity == entity_parent) is_child = true;
        //     });
        // return is_child;
    }

    template<class Iterable>
    auto filter_out_descendants(SceneGraph& scenegraph, const Iterable& entities)
    {
        std::vector<Entity> filtered_entities;
        for (auto& entity : entities)
        {
            bool is_child = false;
            for (auto& entity_other : entities)
            {
                if (entity == entity_other) continue;
                is_child |= is_child_of(scenegraph, entity, entity_other);

                // Debug print child-parent check
                // std::cout << entt::to_integral(entity) << " is child to " << entt::to_integral(entity_other) << ": " << entity_is_child_of(scenegraph, entity, entity_other) << std::endl;
            }
            if (!is_child) filtered_entities.push_back(entity);
        }
        return filtered_entities;
    }
}

void Scene::OnDestroyEntitySelectionEvent(const DestroyEntitySelectionEvent& event)
{
    eeng::Log("DestroyEntityEvent: count = %i", event.entity_selection.size());

    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    auto context = create_context();
    auto filtered_entities = filter_out_descendants(*scenegraph, event.entity_selection.get_all());
    for (auto& root_entity : filtered_entities)
    {
        // Traverse scene graph branch and add destroy commands bottom-up
        auto branch = scenegraph->get_branch_bottomup(root_entity);
        using namespace Editor;
        for (auto& entity : branch)
        {
            auto command = DestroyEntityCommand{ entity, context };
            cmd_queue->add(CommandFactory::Create<DestroyEntityCommand>(command));
        }
    }
}

void Scene::OnCopyEntitySelectionEvent(const CopyEntitySelectionEvent& event)
{
    eeng::Log("CopyEntitySelectionEvent: count = %i", event.entity_selection.size());

    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    auto context = create_context();
    auto filtered_entities = filter_out_descendants(*scenegraph, event.entity_selection.get_all());
    for (auto& entity : filtered_entities)
    {

        using namespace Editor;
        auto command = CopyEntityBranchCommand{ entity, context };
        cmd_queue->add(CommandFactory::Create<CopyEntityBranchCommand>(command));
    }
}

void Scene::OnSetParentEntitySelectionEvent(const SetParentEntitySelectionEvent& event)
{
    eeng::Log("SetParentEntityEvent: count = %i", event.entity_selection.size());

    if (event.entity_selection.size() < 2) return;
    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    // Parent entity is the last selected
    auto parent_entity = event.entity_selection.last();

    // Extract all but last and filter out descendants
    auto entities = event.entity_selection.all_except_last();
    auto filtered_entities = filter_out_descendants(*scenegraph, entities);

    // Create branch commands per entity
    for (auto& entity : filtered_entities)
    {
        if (scenegraph->is_descendant_of(parent_entity, entity))
        {
            // Illegal to parent an entity to one of its descendants (unparent descendant first)
            eeng::LogError("Cannot parent entity %u to its descendant %u",
                entity.to_integral(),
                parent_entity.to_integral());

            continue;
        }

        using namespace Editor;
        auto command = ReparentEntityBranchCommand{ entity, parent_entity, create_context() };
        cmd_queue->add(CommandFactory::Create<ReparentEntityBranchCommand>(command));
    }
}

void Scene::OnUnparentEntitySelectionEvent(const UnparentEntitySelectionEvent& event)
{
    eeng::Log("UnparentEntityEvent: count = %i", event.entity_selection.size());

    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    // Filter out descendants
    auto filtered_entities = filter_out_descendants(*scenegraph, event.entity_selection.get_all());

    // Create branch commands per entity
    for (auto& entity : filtered_entities)
    {
        if (scenegraph->is_root(entity))
        {
            eeng::LogError("Entity %u already unparented", entity.to_integral());
            continue;
        }

        using namespace Editor;
        auto command = ReparentEntityBranchCommand{ entity, Entity{}, create_context() };
        cmd_queue->add(CommandFactory::Create<ReparentEntityBranchCommand>(command));
    }
}

void Scene::OnAddComponentToEntitySelectionEvent(const AddComponentToEntitySelectionEvent& event)
{
    eeng::Log("AddComponentToEntityEvent: comp_id %u, count = %i", event.component_id, event.entity_selection.size());

    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    Meta::ensure_storage(*registry, event.component_id);
    auto storage = registry->storage(event.component_id);
    auto context = create_context();
    using namespace Editor;

    for (auto& entity : event.entity_selection.get_all())
    {
        if (storage->contains(entity))
        {
            eeng::LogError("Entity %u already contains Component %u",
                entity.to_integral(),
                event.component_id);
            continue;
        }

        auto command = AddComponentToEntityCommand{ entity, event.component_id, context };
        cmd_queue->add(CommandFactory::Create<AddComponentToEntityCommand>(command));
    }
}

void Scene::OnRemoveComponentFromEntitySelectionEvent(const RemoveComponentFromEntitySelectionEvent& event)
{
    eeng::Log("RemoveComponentFromEntityEvent: comp_id %u, count = %i", event.component_id, event.entity_selection.size());

    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    auto storage = registry->storage(event.component_id);
    auto context = create_context();
    using namespace Editor;

    for (auto& entity : event.entity_selection.get_all())
    {
        if (!storage->contains(entity))
        {
            eeng::LogError("Entity %u does not contain Component %u",
                entity.to_integral(),
                event.component_id);
            continue;
        }

        auto command = RemoveComponentFromEntityCommand{ entity, event.component_id, context };
        cmd_queue->add(CommandFactory::Create<RemoveComponentFromEntityCommand>(command));
    }
}

// TODO: Remove + Command
void Scene::OnAddScriptToEntitySelectionEvent(const AddScriptToEntitySelectionEvent& event)
{
    eeng::Log("AddScriptToEntitySelectionEvent: path %s, count = %i",
        event.script_path.c_str(),
        event.entity_selection.size());

    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    //...


    //     sol::table add_script_from_file(
    //     auto& registry,
    //     entt::entity entity,
    //     auto& lua,
    //     const std::string& script_dir,
    //     const std::string& script_name
    // )
    auto split = FileManager::SplitPath(event.script_path);
    auto path = split.directory;
    auto just_filename = split.name_without_extension;
    add_script_from_file(*registry, event.entity_selection.first(), lua, path, just_filename);
}

// TODO: Remove + Command
void Scene::OnRemoveScriptFromEntitySelectionEvent(const RemoveScriptFromEntitySelectionEvent& event)
{
    eeng::Log("RemoveScriptFromEntitySelectionEvent: path %s, count = %i",
        event.script_path.c_str(),
        event.entity_selection.size());

    event.entity_selection.assert_valid([&](entt::entity entity) {
        return entity != entt::null && registry->valid(entity);
        });

    //...
}

/// Dispatched without queueing
void Scene::OnChunkModifiedEvent(const ChunkModifiedEvent& event)
{
    eeng::Log("ChunkModifiedEvent: entity %u, chunk = %s",
        event.entity.to_integral(),
        event.chunk_tag.c_str());

    chunk_registry.reassign_entity(event.chunk_tag, event.entity);
}

void Scene::OnRunScriptEvent(const RunScriptEvent& event)
{
    auto script = FileManager::SplitPath(event.script_path).filename;
    eeng::Log("OnRunScriptEvent: %s", script.c_str());

    // Run the Lua file
    try
    {
        lua->safe_script_file(event.script_path);
    }
    catch (const sol::error& e)
    {
        eeng::LogError("Could not execute %s", e.what());
    }

    // Quick fix to keep command state persistent
    // cmd_queue->remove_pending();
    cmd_queue->clear();
}