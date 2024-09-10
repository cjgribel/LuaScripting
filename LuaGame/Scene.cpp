
#include <thread>
#include <chrono>

#include <algorithm>
#include <execution>

#include "imgui.h"
#include "mat.h"

#include "MetaInspect.hpp"
#include "MetaClone.hpp"

#include "Scene.hpp"

#include "bond.hpp"
#include "transform.hpp"
//#include "kbhit.hpp"
#include "AudioManager.hpp"
#include "CoreComponents.hpp"
#include "DebugClass.h"

#include "InspectorState.hpp" // for MetaInspect

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
        entt::meta<T>()
            .type(entt::type_hash<T>::value()) //.prop(display_name_hs, "int")
            //            .template func < [](nlohmann::json& j, const void* ptr) { j = *static_cast<const T*>(ptr); }, entt::as_void_t > (to_json_hs)
            //            .template func < [](const nlohmann::json& j, void* ptr) { *static_cast<T*>(ptr) = j; }, entt::as_void_t > (from_json_hs)

            .template func < [](void* ptr, Editor::InspectorState& inspector) -> bool {
            return Editor::inspect_type(*static_cast<T*>(ptr), inspector);
            } > (inspect_hs)
                ;

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

    void bindAudioManager(sol::state& lua)
    {
        auto& audioManager = AudioManager::getInstance();

        lua.new_usertype<AudioManager>("AudioManager",
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

        lua["audio_manager"] = &audioManager;
    }

    void registerCircleColliderGridComponent(sol::state& lua)
    {
        lua.new_usertype<CircleColliderGridComponent>("CircleColliderGridComponent",
            "type_id",
            &entt::type_hash<CircleColliderGridComponent>::value,

            sol::call_constructor,
            sol::factories([](
                int width,
                int height,
                bool is_active,
                unsigned char layer_bit,
                unsigned char layer_mask)
                {
                    assert(width * height <= EntitySetSize);
                    auto c = CircleColliderGridComponent{
                        .element_count = width * height,
                        .width = width,
                        .is_active = is_active,
                        .layer_bit = layer_bit,
                        .layer_mask = layer_mask };
                    //for (int i = 0; i < width * height; i++)
                    //    c.active_indices.add(i);
                    return c;
                }),

            // "add_circle",
            // [](CircleColliderGridComponent& c, float x, float y, float radius, bool is_active)
            // {
            //     if (c.count >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.pos[c.count].x = x;
            //     c.pos[c.count].y = y;
            //     c.radii[c.count] = radius;
            //     c.is_active_flags[c.count] = is_active;
            //     c.count++;
            // },
            "set_circle_at",
            [](CircleColliderGridComponent& c,
                int index,
                float x,
                float y,
                float radius,
                bool is_active)
            {
                assert(index >= 0 && index < c.element_count);
                // c.pos[index].x = x;
                // c.pos[index].y = y;
                // c.radii[index] = radius;
                // c.is_active_flags[index] = is_active;

                auto& circle = c.circles[index];
                circle.pos = v2f{ x, y };
                circle.radius = radius;

                if (is_active) c.active_indices.add(index);
                else c.active_indices.remove(index);
            },
            "set_active_flag_all", [](CircleColliderGridComponent& c, bool is_active)
            {
                for (int i = 0; i < c.element_count; i++)
                {
                    // c.is_active_flags[i] = is_active;
                    if (is_active) c.active_indices.add(i);
                    else c.active_indices.remove(i);
                }
                c.is_active = is_active;
            },
            // "get_radius", [](CircleColliderGridComponent& ccsc, int index) -> float {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     return ccsc.radii[index];
            // },
            // "set_radius", [](CircleColliderGridComponent& ccsc, int index, float value) {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     ccsc.radii[index] = value;
            // },
            // "get_is_active_flag", [](CircleColliderGridComponent& ccsc, int index) -> float {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     return ccsc.is_active_flags[index];
            // },
            "set_active_flag_at", [](CircleColliderGridComponent& c, int index, bool is_active)
            {
                assert(index >= 0 && index < c.element_count);
                //if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                //std::cout << index << std::endl;
                // c.is_active_flags[index] = is_active;
                if (is_active) c.active_indices.add(index);
                else c.active_indices.remove(index);
            },
            // TODO
            "is_any_active", [](CircleColliderGridComponent& c) -> bool
            {
                return c.active_indices.get_dense_count() > 0;
                // for (int i = 0; i < c.count; i++)
                // {
                //     if (c.is_active_flags[i]) return true;
                // }
                // return false;
            },
            "get_element_count", [](CircleColliderGridComponent& c)
            {
                return c.element_count;
            },
            "is_active",
            &CircleColliderGridComponent::is_active
            // sol::meta_function::to_string,
            // &CircleColliderGridComponent::to_string
        );
    }

    void registerIslandFinderComponent(sol::state& lua)
    {
        lua.new_usertype<IslandFinderComponent>("IslandFinderComponent",
            "type_id",
            &entt::type_hash<IslandFinderComponent>::value,
            sol::call_constructor,
            sol::factories([](int core_x, int core_y) {
                return IslandFinderComponent{
                    .core_x = core_x,
                    .core_y = core_y
                };
                }),
            "get_nbr_islands", [](IslandFinderComponent& c) {
                return c.islands.size();
            },
            "get_island_index_at", [](IslandFinderComponent& c, int index) {
                assert(index < c.islands.size());
                return c.islands[index];
            }
            // TODO
//            sol::meta_function::to_string,
//            &IslandFinderComponent::to_string
);
    }

    void registerQuadGridComponent(sol::state& lua)
    {
        lua.new_usertype<QuadGridComponent>("QuadGridComponent",
            "type_id",
            &entt::type_hash<QuadGridComponent>::value,
            sol::call_constructor,
            sol::factories([](
                int width,
                int height,
                bool is_active)
                {
                    assert(width * height <= EntitySetSize);
                    return QuadGridComponent{
                        .count = width * height,
                        .width = width,
                        .is_active = is_active
                    };
                }),
            // "add_quad", [](QuadGridComponent& c, float x, float y, float size, uint32_t color, bool is_active) {
            //     if (c.count >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.pos[c.count].x = x;
            //     c.pos[c.count].y = y;
            //     c.sizes[c.count] = size;
            //     c.colors[c.count] = color;
            //     c.is_active_flags[c.count] = is_active;
            //     c.count++;
            // },
            "set_quad_at",
            [](QuadGridComponent& c,
                int index,
                float x,
                float y,
                float size,
                uint32_t color,
                bool is_active)
            {
                assert(index >= 0 && index < c.count);
                c.pos[index].x = x;
                c.pos[index].y = y;
                c.sizes[index] = size;
                c.colors[index] = color;
                // if (is_active && !c.is_active_flags[index]) c.active_indices[c.nbr_active++] = index;
                c.is_active_flags[index] = is_active;
            },
            "set_active_flag_all", [](QuadGridComponent& c, bool is_active) {
                for (int i = 0; i < c.count; i++)
                    c.is_active_flags[i] = is_active;
                c.is_active = is_active;
            },
            "get_pos_at", [](QuadGridComponent& c, int index) {
                //if (index < 0 || index >= c.count) throw std::out_of_range("Index out of range");
                assert(index >= 0 && index < c.count);
                return std::make_tuple(c.pos[index].x, c.pos[index].y);
            },
            // "set_pos", [](QuadGridComponent& c, int index, float x, float y) {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.pos[index].x = x;
            //     c.pos[index].y = y;
            // },
            "get_size_at", [](QuadGridComponent& c, int index) -> float {
                //if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                assert(index >= 0 && index < c.count);
                return c.sizes[index];
            },
            // "set_size", [](QuadGridComponent& c, int index, float value) {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.sizes[index] = value;
            // },
            "get_color_at", [](QuadGridComponent& c, int index) -> uint32_t {
                // if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                assert(index >= 0 && index < c.count);
                return c.colors[index];
            },
            "set_color_at", [](QuadGridComponent& c, int index, uint32_t color) {
                // if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                assert(index >= 0 && index < c.count);
                c.colors[index] = color;
            },
            "set_color_all", [](QuadGridComponent& c, uint32_t color) {
                for (int i = 0; i < c.count; i++)
                    c.colors[i] = color;
            },
            // "get_is_active_flag", [](QuadGridComponent& c, int index) -> float {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     return c.is_active_flags[index];
            // },
            "set_active_flag_at", [](QuadGridComponent& c, int index, bool is_active) {
                // if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                assert(index >= 0 && index < c.count);
                c.is_active_flags[index] = is_active;
            },
            "get_element_count", [](QuadGridComponent& c) {
                return c.count;
            },
            //"count",
            //&QuadGridComponent::count,
            "is_active",
            &QuadGridComponent::is_active,
            sol::meta_function::to_string,
            &QuadGridComponent::to_string
        );
    }

    void registeDataGridComponent(sol::state& lua)
    {
        lua.new_usertype<DataGridComponent>("DataGridComponent",
            "type_id",
            &entt::type_hash<DataGridComponent>::value,

            sol::call_constructor,
            sol::factories([](
                int width,
                int height)
                {
                    assert(width * height <= EntitySetSize);
                    return DataGridComponent{
                        .count = width * height,
                        .width = width };
                }),
            "set_slot1_at",
            [](DataGridComponent& c,
                int index,
                float value)
            {
                assert(index >= 0 && index < c.count);
                c.slot1[index] = value;
            },
            "set_slot2_at",
            [](DataGridComponent& c,
                int index,
                float value)
            {
                assert(index >= 0 && index < c.count);
                c.slot2[index] = value;
            },
            "get_slot1_at",
            [](DataGridComponent& c, int index)
            {
                assert(index >= 0 && index < c.count);
                return c.slot1[index];
            },
            "get_slot2_at",
            [](DataGridComponent& c, int index)
            {
                assert(index >= 0 && index < c.count);
                return c.slot2[index];
            }
            //sol::meta_function::to_string,
            //&CircleColliderGridComponent::to_string
        );
    }

    void registerQuadComponent(sol::state& lua)
    {
        lua.new_usertype<QuadComponent>("QuadComponent",
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

    void registerCircleColliderComponent(sol::state& lua)
    {
        lua.new_usertype<CircleColliderComponent>("CircleColliderComponent",
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

    void registerScriptedBehaviorComponent(sol::state& lua)
    {
        lua.new_usertype<ScriptedBehaviorComponent>("ScriptedBehaviorComponent",
            "type_id",
            &entt::type_hash<ScriptedBehaviorComponent>::value,
            sol::call_constructor,
            sol::factories([]() {
                return ScriptedBehaviorComponent{ };
                }),
            // "scripts",
            // &ScriptedBehaviorComponent::scripts,
            // "get_script_by_id",
            // &ScriptedBehaviorComponent::get_script_by_id,
            sol::meta_function::to_string,
            &ScriptedBehaviorComponent::to_string
        );
    }

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
    void script_system_update(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<ScriptedBehaviorComponent>();
        for (auto entity : view)
        {
            assert(entity != entt::null);
            assert(registry.valid(entity));

            auto& script_comp = view.get<ScriptedBehaviorComponent>(entity);
            for (auto& script : script_comp.scripts)
            {
                assert(script.self.valid());
                script.update(script.self, delta_time);
            }
        }
    }

    // Register the input module with Lua
    void register_input_script(sol::state& lua)
    {
        // sol::load_result result = lua.load_file("lua/input.lua");
        sol::load_result result = lua.load_file("../../LuaGame/lua/input.lua"); // TODO: working directory
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Failed to load input.lua: " << err.what() << std::endl;
            return;
        }
        sol::protected_function_result script_result = result();
        if (!script_result.valid()) {
            sol::error err = script_result;
            std::cerr << "Failed to execute input.lua: " << err.what() << std::endl;
            return;
        }
    }

    void update_input_script(sol::state& lua, v4f axes, vec4<bool> buttons)
    {
        lua["update_input"](
            axes.x,
            axes.y,
            axes.z,
            axes.w,
            buttons.x,
            buttons.y,
            buttons.z,
            buttons.w
            );
    }

    // Called from Lua
    // Adds a behavior script to the ScriptedBehaviorComponent of an entity
    // Adds entity & registry to the script ('id', 'owner')
    // 
    sol::table add_script(
        entt::registry& registry,
        entt::entity entity,
        const sol::table& script_table,
        const std::string& identifier)
    {
        std::cout << "add_script " << (uint32_t)entity << std::endl;
        //return;
        assert(script_table.valid());

        ScriptedBehaviorComponent::BehaviorScript script{ script_table };

        script.update = script.self["update"];
        assert(script.update.valid());

        script.on_collision = script.self["on_collision"];
        assert(script.on_collision.valid());

        script.identifier = identifier;

        // -> entityID?
        script.self["id"] = sol::readonly_property([entity] { return entity; });
        // -> registry?
        script.self["owner"] = std::ref(registry);

        if (auto&& f = script.self["init"]; f.valid())
            f(script.self);
        // inspect_script(script);

        auto& script_comp = registry.get_or_emplace<ScriptedBehaviorComponent>(entity);
        script_comp.scripts.push_back({ script /*, script["update"]*/ });

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
        return script.self;
    }

    void add_script_from_file(
        entt::registry& registry,
        entt::entity entity,
        sol::state& lua,
        const std::string& script_file,
        const std::string& identifier)
    {
        sol::load_result behavior_script = lua.load_file(script_file);
        sol::protected_function script_function = behavior_script;
        assert(behavior_script.valid());
        add_script(registry, entity, script_function(), identifier);
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

    void IslandFinderSystem(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<IslandFinderComponent>();
#ifdef EENG_COMPILER_CLANG
        for (auto entity : view)
        {
            auto& grid_comp = view.get<IslandFinderComponent>(entity);
            auto& colliderset_comp = registry.get<CircleColliderGridComponent>(entity);
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
void bind_conditional_observer(sol::state& lua, ConditionalObserver& observer)
{
    lua.new_usertype<ConditionalObserver>("ConditionalObserver",
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

    lua["observer"] = &observer;
}

namespace Inspector
{
    void inspect_scene_graph_node(SceneGraph& scenegraph, entt::registry& registry, size_t index = 0)
    {
        assert(index >= 0 && index < scenegraph.tree.size());

        auto [entity, nbr_children, branch_stride, parent_ofs] = scenegraph.tree.get_node_info_at(index);

        std::string label = Editor::get_entity_name(registry, entity, entt::resolve<HeaderComponent>());
        // Add entity nbr to start for clarity
        label = "[entity#" + std::to_string(entt::to_integral(entity)) + "] " + label;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
        if (!nbr_children) flags |= ImGuiTreeNodeFlags_Leaf;

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNodeEx(label.c_str(), flags)) {

            // Recursively display each child node
            int child_index = index + 1;
            for (int i = 0; i < nbr_children; ++i)
            {
                inspect_scene_graph_node(scenegraph, registry, child_index);

                auto [entity, nbr_children, branch_stride, parent_ofs] = scenegraph.tree.get_node_info_at(child_index);
                child_index += branch_stride; //scenegraph.tree[current_index].;
            }

            ImGui::TreePop();
        }
    }

    void inspect_scenegraph(SceneGraph& scenegraph, entt::registry& registry)
    {
        static bool open = true;
        bool* p_open = &open;

        // ImGui window
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (!ImGui::Begin("Scene graph", p_open))
        {
            ImGui::End();
            return;
        }

        // Explicit traverse button
        ImGui::SameLine();
        if (ImGui::Button("Traverse"))
        {
            scenegraph.traverse(registry);
        }

        //
        if (scenegraph.size())
        {
            // For all roots ...
            size_t i = 0;
            while (i < scenegraph.tree.size())
            {
                inspect_scene_graph_node(scenegraph, registry, i);
                i += scenegraph.tree.nodes[i].m_branch_stride;
            }
        }

        ImGui::End(); // Window
    }

    void inspect_registry(entt::registry& registry)
    {
        static Editor::InspectorState inspector{};
        static bool open = true;
        bool* p_open = &open;

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (!ImGui::Begin("Inspector", p_open))
        {
            ImGui::End();
            return;
        }

        ImGui::SameLine();
        if (ImGui::Button("Clone entity"))
        {
            auto entity_clone = registry.create();
            auto entity_src = entt::entity{ 109 };
            Editor::clone_entity(registry, entity_src, entity_clone);

            // Deep-copy entire entity
            //Copier copier{ *scene };
            //active_entity = copier.CopyPrimaryEntity(active_entity, components);
            //scene->issue_reload_render_entities();
        }

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode("Entities"))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
            const ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
            //        const ImGuiTableFlags flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable;
            if (ImGui::BeginTable("InspectorTable", 2, flags))
            {

                // entt::id_type of HeaderComponent, to obtain names for entities with those
                auto header_meta = entt::resolve<HeaderComponent>();

                Editor::inspect_registry(registry, header_meta, inspector);
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
    }
}

inline void lua_panic_func(sol::optional<std::string> maybe_msg)
{
    std::cerr << "Lua is in a panic state and will now abort() the application" << std::endl;
    if (maybe_msg) {
        const std::string& msg = maybe_msg.value();
        std::cerr << "\terror message: " << msg << std::endl;
    }
    // When this function exits, Lua will exhibit default behavior and abort()
}

void Scene::destroy_pending_entities()
{
    int count = 0;
    while (entities_pending_destruction.size())
    {
        auto entity = entities_pending_destruction.back();
        entities_pending_destruction.pop_back();

        // Destroy entity. May lead to additional entities being added to the queue.
        registry.destroy(entity);

        // Remove from scene graph
        if (scenegraph.tree.contains(entity))
            scenegraph.erase_node(entity);

        count++;
    }

    if (count)
        eeng::Log::log("%i entities destroyed", count);
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

    try
    {
        // Create enTT registry
        registry = entt::registry{};

        // Or, rely on RAII to unload scripts?
        registry.on_destroy<ScriptedBehaviorComponent>().connect<&release_script>();

        // Create Lua state
        lua = sol::state{ (sol::c_call<decltype(&lua_panic_func), &lua_panic_func>) };
        lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string,
            sol::lib::math,
            sol::lib::os,
            sol::lib::table);

        // - Start of Lua binding -
        lua.create_named_table("engine");

        lua["engine"]["entity_null"] = entt::entity{ entt::null };

        // Create entity
        //
        lua["engine"]["create_entity"] = [&](entt::entity parent_entity) {
            auto entity = registry.create();
            if (parent_entity == entt::null)
            {
                scenegraph.create_node(entity);
            }
            else
            {
                assert(scenegraph.tree.contains(parent_entity));
                scenegraph.create_node(entity, parent_entity);
            }
            return entity;
            };

        // Destroy entity
        //
        lua["engine"]["destroy_entity"] = [&](entt::entity entity) {
            assert(registry.valid(entity));
            entities_pending_destruction.push_back(entity);
            };

        // Register to Lua: helper functions for adding & obtaining scripts from entities
        lua["add_script"] = &add_script;
        lua["get_script"] = &get_script;

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
        lua["log"] = [&](const std::string& text)
            {
                eeng::Log::log("[Lua] %s", text.c_str());
                //eeng::Log::log((std::string("[Lua] ") + text).c_str());
            };

        // Particle emitter functions
        lua["emit_particle"] = [&](
            float x,
            float y,
            float vx,
            float vy,
            uint32_t color)
            {
                particleBuffer.push_point(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, color);
            };
        lua["emit_explosion"] = [&](
            float x,
            float y,
            float vx,
            float vy,
            int nbr_particles,
            uint32_t color)
            {
                particleBuffer.push_explosion(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, nbr_particles, color);
            };
        lua["emit_trail"] = [&](
            float x,
            float y,
            float vx,
            float vy,
            int nbr_particles,
            uint32_t color)
            {
                particleBuffer.push_trail(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, nbr_particles, color);
            };

        // Register to Lua: input module
        register_input_script(lua);
        if (!lua["input"].valid()) {
            std::cerr << "Error: 'input' table not loaded properly" << std::endl;
            // return -1;
            assert(0);
        }
        //update_input_script(lua, 0.0f, 0.0f, false);

        // Attach registry to Lua state
        lua.require("registry", sol::c_call<AUTO_ARG(&open_registry)>, false);
        lua["registry"] = std::ref(registry);

        // Register AudioManager to Lua
        bindAudioManager(lua);
        // Register core components to Lua
        register_transform(lua);
        registerQuadComponent(lua); // remove
        registerCircleColliderComponent(lua); // remove
        registerScriptedBehaviorComponent(lua);
        //
        registerCircleColliderGridComponent(lua);
        registerQuadGridComponent(lua);
        registerIslandFinderComponent(lua);
        registeDataGridComponent(lua);
        //
        HeaderComponent_metaregister(lua); // entt::meta (registry stuff added separately) + sol meta

        // ImGui -> Lua
        lua.set_function("ImGui_Text", &ImGui_Text);
        lua.set_function("ImGui_Begin", &ImGui_Begin);
        lua.set_function("ImGui_End", &ImGui_End);
        lua.set_function("ImGui_SetNextWindowPos", &ImGui_SetNextWindowPos);
        lua.set_function("ImGui_SetNextWindowWorldPos", [&](float x, float y)
            {
                // Transform from world to screen space
                const v4f pos_ss = (VP * P * V) * v4f{ x, y, 0.0f, 1.0f };
                ImGui_SetNextWindowPos(pos_ss.x / pos_ss.w, SceneBase::windowSize.y - pos_ss.y / pos_ss.w);
            });

        // Bind observer
        bind_conditional_observer(lua, observer);

        // Load & execute init script
        lua.safe_script_file("../../LuaGame/lua/init.lua"); // TODO: working directory

        assert(lua["game"].valid());
        assert(lua["game"]["init"].valid());
        assert(lua["game"]["destroy"].valid());
        lua["game"]["init"](lua["game"]);
        // lua["game"]["destroy"]();

        // Lua binding done
        
        // Inspect the Lua state
        {
            // Lambda function to inspect Lua tables
            std::function<void(sol::state&, sol::table, const std::string)>
                inspect_lua = [&](sol::state& lua, sol::table tbl, const std::string indent) 
                {
                for (auto& kv : tbl) 
                {
                    sol::object key = kv.first;
                    sol::object value = kv.second;

                    // Assuming the key is a string for simplicity
                    std::string key_str = key.as<std::string>();

                    // Correct usage of lua_typename with the Lua state and the type code
                    std::string type_name = lua_typename(lua.lua_state(), static_cast<int>(value.get_type()));

                    std::cout << indent << key_str << " (" << type_name << ")\n";

                    if (value.get_type() == sol::type::table) {
                        inspect_lua(lua, value.as<sol::table>(), indent + "   ");
                    }
                    else if (value.get_type() == sol::type::function) {
                        std::cout << indent << "  [function]\n";
                    }
                    else {
                        std::cout << indent << "  [" << lua["tostring"](value).get<std::string>() << "]\n";
                    }
                }
                };

                std::cout << "Inspect Lua engine state:" << std::endl;
                inspect_lua(lua, lua["engine"], "   ");
                std::cout << "Inspect Lua game state:" << std::endl;
                inspect_lua(lua, lua["game"], "   ");
        }

        // To test inspection
        auto debug_entity = registry.create();
        registry.emplace<DebugClass>(debug_entity);

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

    update_input_script(lua, SceneBase::axes, SceneBase::buttons);

    particleBuffer.update(deltaTime_s);

    script_system_update(registry, deltaTime_s);

    // Debug print
    if (entities_pending_destruction.size())
    {
        std::cout << "Destroying " << (int)entities_pending_destruction.size() << " entities... ";
        for (auto entity : entities_pending_destruction) std::cout << entt::to_integral(entity) << " ";
        std::cout << std::endl;
    }

    // Entity destruction takes place outside the update loop
    destroy_pending_entities();
    // if (entities_pending_destruction.size())
    // {
    //     //
    //     std::cout << "Destroying " << (int)entities_pending_destruction.size() << " entities... ";
    //     for (auto entity : entities_pending_destruction) std::cout << entt::to_integral(entity) << " ";
    //     std::cout << std::endl;

    //     //eeng::Log::log("Destroying %i entities...", (int)entities_pending_destruction.size());
    //     for (auto entity : entities_pending_destruction)
    //         registry.destroy(entity);
    //     entities_pending_destruction.clear();
    // }

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
                if (!registry.all_of<ScriptedBehaviorComponent>(entity)) return;
                auto& script_comp = registry.get<ScriptedBehaviorComponent>(entity);
                for (auto& script : script_comp.scripts)
                {
                    assert(script.self.valid());
                    script.on_collision(script.self, x, y, nx, ny, collider_index, other_entity);
                }
            };

        auto view = registry.view<CircleColliderGridComponent>();
        for (auto it1 = view.begin(); it1 != view.end(); ++it1)
        {
            auto entity1 = *it1;
            const auto& transform1 = registry.get<Transform>(entity1);
            const auto& collider1 = view.get<CircleColliderGridComponent>(entity1);
            if (!collider1.is_active) continue;

            const auto R1 = m2f::rotation(transform1.rot);
            for (auto it2 = it1; ++it2 != view.end(); )
            {
                auto entity2 = *it2;
                const auto& transform2 = registry.get<Transform>(entity2);
                const auto& collider2 = view.get<CircleColliderGridComponent>(entity2);

                if (!collider2.is_active) continue;
                // LAYER CHECK
                if (!(collider1.layer_bit & collider2.layer_mask)) continue;

                const auto R2 = m2f::rotation(transform2.rot);
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

                        const float x1 = transform1.x + pos1.x;
                        const float y1 = transform1.y + pos1.y;
                        const float x2 = transform2.x + pos2.x;
                        const float y2 = transform2.y + pos2.y;

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

    }

void Scene::renderUI()
{
    assert(is_initialized);

    {
        size_t nbr_entities = 0;
        auto view = registry.view<entt::entity>();
        for (auto entity : view) nbr_entities++;
        ImGui::Text("Nbr of active entities %i", (int)nbr_entities);
    }

    ImGui::Text("Drawcall count %i", drawcallCount);

    ImGui::Checkbox("Debug render", &debug_render);

    ImGui::Text("Particles %i/%i", particleBuffer.size(), particleBuffer.capacity());

    Inspector::inspect_registry(registry);

    Inspector::inspect_scenegraph(scenegraph, registry);

    // float available_width = ImGui::GetContentRegionAvail().x;
    // if (ImGui::Button("Reload scripts", ImVec2(available_width, 0.0f)))
    // {
    //     reload_scripts();
    // }

    // if (ImGui::ColorEdit3("Light color",
    //     glm::value_ptr(lightColor),
    //     ImGuiColorEditFlags_NoInputs))
    // {
    // }

    // // Combo (drop-down) for animation clip
    // if (characterMesh)
    // {
    //     int curAnimIndex = characterAnimIndex;
    //     std::string label = (curAnimIndex == -1 ? "Bind pose" : characterMesh->getAnimationName(curAnimIndex));
    //     if (ImGui::BeginCombo("Character animation##animclip", label.c_str()))
    //     {
    //         // Bind pose item
    //         const bool isSelected = (curAnimIndex == -1);
    //         if (ImGui::Selectable("Bind pose", isSelected))
    //             curAnimIndex = -1;
    //         if (isSelected)
    //             ImGui::SetItemDefaultFocus();

    //         // Clip items
    //         for (int i = 0; i < characterMesh->getNbrAnimations(); i++)
    //         {
    //             const bool isSelected = (curAnimIndex == i);
    //             const auto label = characterMesh->getAnimationName(i) + "##" + std::to_string(i);
    //             if (ImGui::Selectable(label.c_str(), isSelected))
    //                 curAnimIndex = i;
    //             if (isSelected)
    //                 ImGui::SetItemDefaultFocus();
    //         }
    //         ImGui::EndCombo();
    //         characterAnimIndex = curAnimIndex;
    //     }
    // }
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
        auto view = registry.view<Transform, QuadComponent>();
        float z = 0.0f;
        for (auto entity : view)
        {
            auto& transform_comp = registry.get<Transform>(entity);

            auto& quad_comp = registry.get<QuadComponent>(entity);
            if (!quad_comp.is_visible) continue;

            const auto pos = v3f{ transform_comp.x, transform_comp.y, 0.0f };
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
        auto view = registry.view<QuadGridComponent>();
        for (auto entity : view)
        {
            auto& quadgrid = view.get<QuadGridComponent>(entity);
            if (!quadgrid.is_active) continue;

            auto& transform_comp = registry.get<Transform>(entity);
            const auto G = m4f::TRS(
                v3f{ transform_comp.x, transform_comp.y, 0.0f },
                transform_comp.rot, v3f_001,
                v3f_111
            );

            for (int i = 0; i < quadgrid.count; i++)
            {
                if (!quadgrid.is_active_flags[i]) continue;

                const auto& pos = xy0(quadgrid.pos[i]);
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

#if 0
    // Render all CircleColliderGridComponent
    if (debug_render)
    {
        auto view = registry.view<CircleColliderGridComponent>();
        for (auto entity : view)
        {
            auto& collidergrid = view.get<CircleColliderGridComponent>(entity);

            auto& transform_comp = registry.get<Transform>(entity);
            const auto G = m4f::TRS(
                v3f{ transform_comp.x, transform_comp.y, 0.0f },
                transform_comp.rot, v3f_001,
                v3f_111
            );

            for (int i = 0; i < collidergrid.count; i++)
            {
                const auto& pos = xy0(collidergrid.pos[i]);
                const auto& r = collidergrid.radii[i];
                const auto M = set_translation(m4f::scaling(r, r, 1.0f), pos);
                const bool visible = collidergrid.is_active_flags[i];
                const auto color = 0xffffffff * visible + 0xff808080 * (1 - visible);

                renderer->push_states(G * M, Renderer::Color4u{ color });
                renderer->push_circle_ring<8>();
                renderer->pop_states<m4f, Renderer::Color4u>();
            }
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
    std::cout << "entities_pending_destruction.size() " << entities_pending_destruction.size() << std::endl;

    // Call the 
    lua["game"]["destroy"](lua["game"]); // <- entities flagged for destruction ???

    std::cout << "entities_pending_destruction.size() " << entities_pending_destruction.size() << std::endl;

    destroy_pending_entities();
    // NOTE: registry should be empty here, so 

    // clear() seem to invoke registry.on_destroy<ScriptedBehaviorComponent>,
    // which will cause destroy() to be called for all behaviors
    // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system
    //
    // Explicitly destroy all ScriptedBehaviorComponent, in order to invoke 
    // registry.on_destroy<ScriptedBehaviorComponent>
    // registry.clear<ScriptedBehaviorComponent>();

    {
        std::cout << "Entities in registry: ";
        auto view = registry.view<entt::entity>();
        for (auto entity : view) std::cout << entt::to_integral(entity) << " ";
        std::cout << std::endl;
    }
    registry.clear();

    std::cout << "entities_pending_destruction.size() " << entities_pending_destruction.size() << std::endl;

    is_initialized = false;

    scenegraph.dump_to_cout(registry, entt::resolve<HeaderComponent>());

    std::cout << "Done: Scene::destroy()" << std::endl;
}