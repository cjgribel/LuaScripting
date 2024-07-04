
#include <thread>
#include <chrono>

#include "imgui.h"
#include "mat.h"
#include "Scene.hpp"

#include "bond.hpp"
#include "transform.hpp"
#include "kbhit.hpp"

#define AUTO_ARG(x) decltype(x), x
using namespace linalg;

namespace {

    // void inspect_script(const ScriptedBehaviorComponent& script)
    // {
    //     script.self.for_each([](const sol::object& key, const sol::object& value)
    //         { std::cout << key.as<std::string>() << ": "
    //         << sol::type_name(value.lua_state(), value.get_type())
    //         << std::endl; });
    // }

    void registerCircleColliderSetComponent(sol::state& lua)
    {
        lua.new_usertype<CircleColliderSetComponent>("CircleColliderSetComponent",
            "type_id",
            &entt::type_hash<CircleColliderSetComponent>::value,

            sol::call_constructor,
            sol::factories([](
                int width,
                int height,
                bool is_active,
                unsigned char layer_bit,
                unsigned char layer_mask)
                {
                    assert(width * height <= EntitySetSize);
                    return CircleColliderSetComponent{
                        .count = width * height,
                        .width = width,
                        .is_active = is_active,
                        .layer_bit = layer_bit,
                        .layer_mask = layer_mask };
                }),

            // "add_circle",
            // [](CircleColliderSetComponent& c, float x, float y, float radius, bool is_active)
            // {
            //     if (c.count >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.pos[c.count].x = x;
            //     c.pos[c.count].y = y;
            //     c.radii[c.count] = radius;
            //     c.is_active_flags[c.count] = is_active;
            //     c.count++;
            // },
            "set_circle",
            [](CircleColliderSetComponent& c,
                int i,
                int j,
                float x,
                float y,
                float radius,
                bool is_active)
            {
                int index = j * c.width + i;
                assert(index < c.count);
                c.pos[index].x = x;
                c.pos[index].y = y;
                c.radii[index] = radius;
                c.is_active_flags[index] = is_active;
            },
            "activate_all", [](CircleColliderSetComponent& c, bool is_active) {
                for (int i = 0; i < c.count; i++)
                    c.is_active_flags[i] = is_active;
                c.is_active = is_active;
            },
            // "get_radius", [](CircleColliderSetComponent& ccsc, int index) -> float {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     return ccsc.radii[index];
            // },
            // "set_radius", [](CircleColliderSetComponent& ccsc, int index, float value) {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     ccsc.radii[index] = value;
            // },
            // "get_is_active_flag", [](CircleColliderSetComponent& ccsc, int index) -> float {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     return ccsc.is_active_flags[index];
            // },
            "set_active_flag", [](CircleColliderSetComponent& ccsc, int index, bool is_active) {
                if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                //std::cout << index << std::endl;
                ccsc.is_active_flags[index] = is_active;
            },
            // TODO
            "is_any_active", [](CircleColliderSetComponent& c) -> bool {
                for (int i = 0; i < c.count; i++)
                {
                    if (c.is_active_flags[i]) return true;
                }
                return false;
            },
            "count",
            &CircleColliderSetComponent::count,
            sol::meta_function::to_string,
            &CircleColliderSetComponent::to_string
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

    void registerQuadSetComponent(sol::state& lua)
    {
        lua.new_usertype<QuadSetComponent>("QuadSetComponent",
            "type_id",
            &entt::type_hash<QuadSetComponent>::value,
            sol::call_constructor,
            sol::factories([](
                int width,
                int height,
                bool is_active)
                {
                    assert(width * height <= EntitySetSize);
                    return QuadSetComponent{
                        .count = width * height,
                        .width = width,
                        .is_active = is_active
                    };
                }),
            // "add_quad", [](QuadSetComponent& c, float x, float y, float size, uint32_t color, bool is_active) {
            //     if (c.count >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.pos[c.count].x = x;
            //     c.pos[c.count].y = y;
            //     c.sizes[c.count] = size;
            //     c.colors[c.count] = color;
            //     c.is_active_flags[c.count] = is_active;
            //     c.count++;
            // },
            "set_quad",
            [](QuadSetComponent& c,
                int i,
                int j,
                float x,
                float y,
                float size,
                uint32_t color,
                bool is_active)
            {
                int index = j * c.width + i;
                assert(index < c.count);
                c.pos[index].x = x;
                c.pos[index].y = y;
                c.sizes[index] = size;
                c.colors[index] = color;
                c.is_active_flags[index] = is_active;
            },
            "activate_all", [](QuadSetComponent& c, bool is_active) {
                for (int i = 0; i < c.count; i++)
                    c.is_active_flags[i] = is_active;
                c.is_active = is_active;
            },
            "get_pos", [](QuadSetComponent& c, int index) {
                if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                return std::make_tuple(c.pos[index].x, c.pos[index].y);
            },
            // "set_pos", [](QuadSetComponent& c, int index, float x, float y) {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.pos[index].x = x;
            //     c.pos[index].y = y;
            // },
            "get_size", [](QuadSetComponent& c, int index) -> float {
                if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                return c.sizes[index];
            },
            // "set_size", [](QuadSetComponent& c, int index, float value) {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     c.sizes[index] = value;
            // },
            "get_color", [](QuadSetComponent& c, int index) -> uint32_t {
                if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                return c.colors[index];
            },
            "set_color", [](QuadSetComponent& c, int index, uint32_t color) {
                if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                c.colors[index] = color;
            },
            "set_color_all", [](QuadSetComponent& c, uint32_t color) {
                for (int i = 0; i < c.count; i++)
                    c.colors[i] = color;
            },
            // "get_is_active_flag", [](QuadSetComponent& c, int index) -> float {
            //     if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
            //     return c.is_active_flags[index];
            // },
            "set_active_flag", [](QuadSetComponent& c, int index, bool is_active) {
                if (index < 0 || index >= EntitySetSize) throw std::out_of_range("Index out of range");
                c.is_active_flags[index] = is_active;
            },
            "count",
            &QuadSetComponent::count,
            sol::meta_function::to_string,
            &QuadSetComponent::to_string
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

    void script_system_update(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<ScriptedBehaviorComponent>();
        for (auto entity : view)
        {
            auto& script_comp = registry.get<ScriptedBehaviorComponent>(entity);
            for (auto& script : script_comp.scripts)
            {
                // auto& script = view.get<ScriptedBehaviorComponent>(entity);
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
        const CircleColliderSetComponent& colliderset_comp)
    {
        const int core_x = grid_comp.core_x;
        const int core_y = grid_comp.core_y;
        const int w = colliderset_comp.width;
        const int h = colliderset_comp.count / w;
        auto& q = grid_comp.visit_queue;
        auto& v = grid_comp.visited;
        auto& islands = grid_comp.islands;
        const auto& is_active = colliderset_comp.is_active_flags;

        v.assign(w * h, false);
        islands.clear();

        // Core is inactive => mark aall nodes as islands
        if (!colliderset_comp.is_active_flags[core_y * w + core_x])
        {
            for (int i = 0; i < w * h; i++)
            {
                if (is_active[i])
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
                    is_active[ny * w + nx])
                {
                    v[ny * w + nx] = true;
                    q.push({ nx, ny });
                }
            }
        }

        // Mark all unvisited active nodes as islands
        for (int i = 0; i < w * h; i++)
        {
            if (is_active[i] && !v[i])
                islands.push_back(i);
        }

#if 0
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                if (x == core_x && (h - y - 1) == core_y)
                    std::cout << "C ";
                else
                {
                    if (is_active[(h - y - 1) * w + x])
                        std::cout << "1 ";
                    else
                        std::cout << "0 ";
                }
            }
            std::cout << '\n';
        }
#endif
#if 0
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                if (x == core_x && (h - y - 1) == core_y)
                    std::cout << "C ";
                else
                {
                    if (v[(h - y - 1) * w + x])
                        std::cout << "1 ";
                    else
                        std::cout << "0 ";
                }
            }
            std::cout << '\n';
        }
#endif
        // for (auto& island_index : islands)
        //     std::cout << island_index << std::endl;
    }

    void IslandFinderSystem(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<IslandFinderComponent>();
        for (auto entity : view)
        {
            auto& grid_comp = view.get<IslandFinderComponent>(entity);
            auto& colliderset_comp = registry.get<CircleColliderSetComponent>(entity);
            update_IslandFinderComponent(grid_comp, colliderset_comp);
        }
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

bool Scene::init(const v2i& windowSize)
{
    assert(!is_initialized);

    std::cout << "Scene::init()" << std::endl;

    SceneBase::windowSize = windowSize;

    // Register registry meta functions to components
    register_meta_component<Transform>();
    register_meta_component<QuadComponent>();
    register_meta_component<CircleColliderComponent>();
    register_meta_component<ScriptedBehaviorComponent>();
    //
    register_meta_component<CircleColliderSetComponent>();
    register_meta_component<QuadSetComponent>();
    register_meta_component<IslandFinderComponent>();

    try
    {
        // Create enTT registry
        registry = entt::registry{};
        registry.on_destroy<ScriptedBehaviorComponent>().connect<&release_script>();

        // Create Lua state
        lua = sol::state{ (sol::c_call<decltype(&my_panic), &my_panic>) };
        lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string,
            sol::lib::math,
            sol::lib::os,
            sol::lib::table);

        // Register to Lua: helper functions for adding & obtaining scripts from entities
        lua["add_script"] = &add_script;
        lua["get_script"] = &get_script;

        // Placeholder particle emitter functions
        const auto emit_particle = [&](
            float x,
            float y,
            float vx,
            float vy,
            uint32_t color)
            {
                particleBuffer.push_point(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, color);
            };
        const auto emit_explosion = [&](
            float x,
            float y,
            float vx,
            float vy,
            int nbr_particles,
            uint32_t color)
            {
                particleBuffer.push_explosion(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, nbr_particles, color);
            };
        const auto emit_trail = [&](
            float x,
            float y,
            float vx,
            float vy,
            int nbr_particles,
            uint32_t color)
            {
                particleBuffer.push_trail(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, nbr_particles, color);
            };
        lua["emit_particle"] = emit_particle;
        lua["emit_explosion"] = emit_explosion;
        lua["emit_trail"] = emit_trail;

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

        // Register to Lua: component types
        register_transform(lua);
        registerQuadComponent(lua);
        registerCircleColliderComponent(lua);
        registerScriptedBehaviorComponent(lua);
        //
        registerCircleColliderSetComponent(lua);
        registerQuadSetComponent(lua);
        registerIslandFinderComponent(lua);

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

        // Run init script
        // lua.safe_script_file("lua/init.lua");
        lua.safe_script_file("../../LuaGame/lua/init.lua"); // TODO: working directory

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

        auto view = registry.view<CircleColliderSetComponent>();
        for (auto it1 = view.begin(); it1 != view.end(); ++it1)
        {
            auto entity1 = *it1;
            const auto& transform1 = registry.get<Transform>(entity1);
            const auto& collider1 = view.get<CircleColliderSetComponent>(entity1);
            if (!collider1.is_active) continue;

            for (auto it2 = it1; ++it2 != view.end(); )
            {
                auto entity2 = *it2;
                const auto& transform2 = registry.get<Transform>(entity2);
                const auto& collider2 = view.get<CircleColliderSetComponent>(entity2);

                if (!collider2.is_active) continue;
                // LAYER CHECK
                if (!(collider1.layer_bit & collider2.layer_mask)) continue;

                for (auto i = 0; i < collider1.count; i++)
                {
                    if (!collider1.is_active_flags[i]) continue;

                    for (auto j = 0; j < collider2.count; j++)
                    {
                        if (!collider2.is_active_flags[j]) continue;

                        float x1 = transform1.x + collider1.pos[i].x;
                        float y1 = transform1.y + collider1.pos[i].y;
                        float x2 = transform2.x + collider2.pos[j].x;
                        float y2 = transform2.y + collider2.pos[j].y;
                        const float r1 = collider1.radii[i];
                        const float r2 = collider2.radii[j];

                        // Calculate the distance between the two entities
                        float dx = x1 - x2; // transform1.x - transform2.x;
                        float dy = y1 - y2; // transform1.y - transform2.y;
                        float distanceSquared = dx * dx + dy * dy;
                        float radiusSum = r1 + r2;

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
                            float px = x1 - r1 * nx + r2 * nx;
                            float py = y1 - r1 * ny + r2 * ny;

                            // std::cout << "Collision detected between entity " << (uint32_t)entity1
                            //     << " and entity " << (uint32_t)entity2 << std::endl;
                            // std::cout << "Contact Point: (" << px << ", " << py << ")\n";
                            // std::cout << "Contact Normal: (" << nx << ", " << ny << ")\n";
                            // std::cout << "Penetration Depth: " << penetrationDepth << "\n";

                            // (nx, ny) points 2 -> 1
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

    ImGui::Text("Drawcall count %i", drawcallCount);

    ImGui::Text("Particles %i/%i", particleBuffer.size(), particleBuffer.capacity());

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

    // Background quad
    renderer->push_states(Renderer::Color4u{ 0x40ffffff });
    renderer->push_quad(v3f{ 0.0f, 0.0f, 0.0f }, 10.0f);
    renderer->pop_states<Renderer::Color4u>();

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
            renderer->push_quad(pos, size);
            renderer->pop_states<Renderer::Color4u>();
        }
    }

    // Render all QuadSetComponent
    {
        auto view = registry.view<QuadSetComponent>();
        // float z = 0.0f;
        for (auto entity : view)
        {
            auto& transform_comp = registry.get<Transform>(entity);
            auto& qc = view.get<QuadSetComponent>(entity);
            auto& cc = registry.get<CircleColliderSetComponent>(entity); // DEBUG

            for (int i = 0; i < qc.count; i++)
            {
                if (!qc.is_active_flags[i]) continue;
                assert(cc.is_active_flags[i] == qc.is_active_flags[i]);  // DEBUG

                const auto pos = xy0(qc.pos[i]) + v3f{ transform_comp.x, transform_comp.y, 0.0f };
                const auto& size = qc.sizes[i];
                const auto& color = qc.colors[i];

                renderer->push_states(Renderer::Color4u{ color });
                renderer->push_quad(pos, size);
                renderer->pop_states<Renderer::Color4u>();
            }
        }
    }

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
    renderer->render(P * V);
    renderer->post_render();
}

void Scene::destroy()
{
    // Explicitly destroy all ScriptedBehaviorComponent in order to
    // invoke on_destroy. Must be done before the Lua state is detroyed.
    registry.clear<ScriptedBehaviorComponent>();
    registry.clear();

    is_initialized = false;
}