
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
    // inline void my_panic(sol::optional<std::string> maybe_msg) {
    //     std::cerr << "Lua is in a panic state and will now abort() the application" << std::endl;
    //     if (maybe_msg) {
    //         const std::string& msg = maybe_msg.value();
    //         std::cerr << "\terror message: " << msg << std::endl;
    //     }
    //     // When this function exits, Lua will exhibit default behavior and abort()
    // }

    // void inspect_script(const ScriptedBehaviorComponent& script)
    // {
    //     script.self.for_each([](const sol::object& key, const sol::object& value)
    //         { std::cout << key.as<std::string>() << ": "
    //         << sol::type_name(value.lua_state(), value.get_type())
    //         << std::endl; });
    // }

    void registerQuadComponent(sol::state& lua)
    {
        lua.new_usertype<QuadComponent>("QuadComponent",
            "type_id",
            &entt::type_hash<QuadComponent>::value,
            sol::call_constructor,
            sol::factories([](float r) {
                return QuadComponent{ r };
                }),
            "r",
            &QuadComponent::r,
            sol::meta_function::to_string,
            &QuadComponent::to_string
        );
    }

    void init_script(entt::registry& registry, entt::entity entity)
    {
        auto& script_comp = registry.get<ScriptedBehaviorComponent>(entity);
        for (auto& script : script_comp.scripts)
        {
            assert(script.self.valid());
            script.update = script.self["update"];
            assert(script.update.valid());

            // -> entityID?
            script.self["id"] = sol::readonly_property([entity]
                { return entity; });
            // -> registry?
            script.self["owner"] = std::ref(registry);

            if (auto&& f = script.self["init"]; f.valid())
                f(script.self);
            // inspect_script(script);
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

    // void register_input_script(sol::state& lua)
    // {
    //     // lua.script_file("lua/input.lua");
    //     lua.safe_script_file("lua/input.lua");
    // }

    // Register the input module with Lua
    void register_input_script(sol::state& lua) {
        sol::load_result result = lua.load_file("lua/input.lua");
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

    void update_input_script(sol::state& lua, float x, float y, bool button_pressed) {
        lua["update_input"](x, y, button_pressed);
    }

    // Called from Lua
    // Adds a behavior script to the ScriptedBehaviorComponent of an entity
    // Adds entity & registry to the script ('id', 'owner')
    // 
    void add_script(
        entt::registry& registry,
        entt::entity entity,
        const sol::table& script_table)
    {
        std::cout << "add_script " << (uint32_t)entity << std::endl;
        //return;
        assert(script_table.valid());

        ScriptedBehaviorComponent::BehaviorScript script{ script_table };
        script.update = script.self["update"];
        assert(script.update.valid());

        // -> entityID?
        script.self["id"] = sol::readonly_property([entity] { return entity; });
        // -> registry?
        script.self["owner"] = std::ref(registry);

        if (auto&& f = script.self["init"]; f.valid())
            f(script.self);
        // inspect_script(script);

        auto& script_comp = registry.get_or_emplace<ScriptedBehaviorComponent>(entity);
        script_comp.scripts.push_back({ script /*, script["update"]*/ });

        // Example: Print the table's contents
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
    }

    void add_script_from_file(
        entt::registry& registry,
        entt::entity entity,
        sol::state& lua,
        const std::string& script_file)
    {
        sol::load_result behavior_script = lua.load_file(script_file);
        sol::protected_function script_function = behavior_script;
        assert(behavior_script.valid());
        add_script(registry, entity, script_function());
    }
}

void Scene::reload_scripts()
{
    // Clear entt registry
    // We must do this before destroying the current Lua state,
    // since the Lua state is accessed when instances of ScriptedBehaviorComponent are destroyed.
    registry.clear();

    // Create Lua state
    lua = sol::state{ (sol::c_call<decltype(&my_panic), &my_panic>) };
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math);

    lua["add_script"] = &add_script;

    // Register input module
    register_input_script(lua);
    if (!lua["input"].valid()) {
        std::cerr << "Error: 'input' table not loaded properly" << std::endl;
        // return -1;
        assert(0);
    }
    update_input_script(lua, 0.0f, 0.0f, false);

    // Attach registry to Lua state
    lua.require("registry", sol::c_call<AUTO_ARG(&open_registry)>, false);

    lua["registry"] = std::ref(registry);

    // Expose components as user types to Lua 
    register_transform(lua);
    registerQuadComponent(lua);

    //
    lua.safe_script_file("lua/init.lua");

#if 1
    // Add 5x of a test behavior script
    // Requires the 'input' module to be registered
    // sol::load_result behavior_script = lua.load_file("lua/behavior.lua");
    // sol::protected_function script_function = behavior_script;
    // assert(behavior_script.valid());
    //
    // Create entities with behavior scripts
    // TODO: Have an init script that creates entities
    for (int i = 0; i < 5; ++i)
    {
        auto e = registry.create();
        registry.emplace<Transform>(e, Transform{ (float)-i, (float)-i });
        registry.emplace<QuadComponent>(e, QuadComponent{ 1.0f });

        add_script_from_file(registry, e, lua, "lua/behavior.lua");

        // sol::table script_table = script_function();
        // ScriptedBehaviorComponent script_comp;
        // ScriptedBehaviorComponent::BehaviorScript script{ script_table };
        // script_comp.scripts.push_back(script);
        // registry.emplace<ScriptedBehaviorComponent>(e, script_comp);

        // Done in behavior.init()
                    // QuadComponent quad_comp {1.0f};
                    // registry.emplace<QuadComponent>(e, quad_comp);
    }
    #endif
}

bool Scene::init()
{
    std::cout << "Scene::init()" << std::endl;
#if 1

    try
    {
        // Register registry meta functions to components
        register_meta_component<Transform>();
        register_meta_component<QuadComponent>();

        // ScriptedBehaviorComponent creation & destruction callbacks
        //registry.on_construct<ScriptedBehaviorComponent>().connect<&init_script>();
        registry.on_destroy<ScriptedBehaviorComponent>().connect<&release_script>();

        reload_scripts();
        return true;

        // lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math);

        //
        // NOT MADE IN RELOAD
        lua["add_script"] = &add_script;

        // Register input module
        register_input_script(lua);
        if (!lua["input"].valid()) {
            std::cerr << "Error: 'input' table not loaded properly" << std::endl;
            return -1;
        }
        update_input_script(lua, 100.0f, 200.0f, true);

        lua.require("registry", sol::c_call<AUTO_ARG(&open_registry)>, false);

        // Expose components to Lua
        register_transform(lua);
        registerQuadComponent(lua);

        // Add 5x of a test behavior script
        // Requires the 'input' module to be registered
        sol::load_result behavior_script = lua.load_file("lua/behavior.lua");
        sol::protected_function script_function = behavior_script;
        assert(behavior_script.valid());
        //
        // Create entities with behavior scripts
        // TODO: Have an init script that creates entities
        for (int i = 0; i < 5; ++i)
        {
            auto e = registry.create();
            registry.emplace<Transform>(e, Transform{ (float)i, (float)i });
            // Done by script registry.emplace<QuadComponent>(e, QuadComponent{ 1.0f });

            // add_script_from_file(registry, e, lua, "lua/behavior.lua");

            sol::table script_table = script_function();

            ScriptedBehaviorComponent script_comp;
            ScriptedBehaviorComponent::BehaviorScript script{ script_table };
            script_comp.scripts.push_back(script);
            registry.emplace<ScriptedBehaviorComponent>(e, script_comp);

            // Sone in behavior.init()
                        // QuadComponent quad_comp {1.0f};
                        // registry.emplace<QuadComponent>(e, quad_comp);
        }

        using namespace std::chrono_literals;

#if 0
        // constexpr auto target_frame_time = 500ms;
        int delta_time_ms{ 1000 };
        while (true)
        {
            using clock = std::chrono::high_resolution_clock;
            const auto begin_ticks = clock::now();

            // Script behavior update
            script_system_update(registry, delta_time_ms * 0.001f);

            // std::this_thread::sleep_for(target_frame_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_time_ms));

            {
                std::cout << "All QuadComponent" << std::endl;
                auto view = registry.view<QuadComponent>();
                for (auto&& entity : view)
                {
                    const auto& cc = view.get<QuadComponent>(entity);
                    std::cout << cc.to_string() << std::endl;
                }
            }

            //delta_time = std::chrono::duration_cast<fsec>(clock::now() - begin_ticks);
            //if (delta_time > 1s)
            //    delta_time = target_frame_time;

            if (_kbhit())
                break;
        }
        registry.clear();
#endif
    }
    // catch (const std::exception& e)
    catch (const sol::error& e)
    {
        std::cout << "exception: " << e.what();
        return -1;
    }
#else
    // Registry test
    // lua/registry_simple.lua, lua/iterate_entities.lua
    // Linking an entt::registry to Lua, creating entities, 
    // adding & modifying components, iterating & removing components
    try {
        register_meta_component<Transform>();

        // sol::state lua{};
        sol::state lua{}; //(sol::c_call<decltype(&my_panic), &my_panic>);
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);

        lua.require("registry", sol::c_call<AUTO_ARG(&open_registry)>, false);
        register_transform(lua); // Make Transform struct available to Lua

        lua["registry"] = std::ref(registry); // Make the registry available to Lua

        // Creates an entity and adds a Transform to it
        std::cout << "Adding an entity..." << std::endl;
        // lua.do_file("lua/registry_simple.lua");
        lua.safe_script_file("lua/registry_simple.lua");
        std::cout << "Adding an entity..." << std::endl;
        // lua.do_file("lua/registry_simple.lua");
        lua.safe_script_file("lua/registry_simple.lua");

        // lua.safe_script_file()
        // std::cout << "Running LUA script with assert(0)" << std::endl;
        // lua.script(R"(
        //     assert(false)
        // )");

        {
            std::cout << "All Transforms:" << std::endl;
            auto view = registry.view<Transform>();
            for (auto& ent : view)
            {
                auto& t = view.get<Transform>(ent);
                std::cout << t.to_string() << std::endl;
            }
        }

        const auto bowser = lua["bowser"].get<entt::entity>();
        const auto* xf = registry.try_get<Transform>(bowser);
        assert(xf != nullptr);
        const Transform& transform = lua["transform"];
        assert(xf->x == transform.x && xf->y == transform.y);

#if 0
        {
            // print 'size_hint'
            entt::runtime_view view{};
            view.iterate(registry.storage<Transform>());
            std::cout << "Initial size hint: " << view.size_hint() << std::endl;
        }
        {
            // Remove Component for all entities
            entt::runtime_view view3{};
            view3.iterate(registry.storage<Transform>());
            for (auto ent : view3)
                registry.remove<Transform>(ent);
            std::cout << "Removed Transform from all entities" << std::endl;
        }
        {
            // print 'size_hint' again
            entt::runtime_view view4{};
            view4.iterate(registry.storage<Transform>());
            // auto view2 = registry.runtime_view<Transform>();
            std::cout << "Size hint after removal: " << view4.size_hint() << std::endl;
        }
#endif

        //
        // lua.do_file("lua/iterate_entities.lua");
        lua.safe_script_file("lua/iterate_entities.lua");
        assert(registry.orphan(bowser) && "The only component (Transform) should  "
            "be removed by the script");

        {
            std::cout << "All Transforms:" << std::endl;
            auto view = registry.view<Transform>();
            for (auto& ent : view)
            {
                auto& t = view.get<Transform>(ent);
                std::cout << t.to_string() << std::endl;
            }
        }
    }
    catch (const sol::error& e)
    {
        std::cerr << "Lua script execution failed: " << e.what() << std::endl;
        return false;
    }
#endif

    // Do some entt stuff
    // entt::registry registry;
    // auto ent1 = registry.create();
    // struct Tfm
    // {
    //     float x, y, z;
    // };
    // registry.emplace<Tfm>(ent1, Tfm{});

    return true;
}

void Scene::update(float time_s, float deltaTime_s)
{
    lightPos = xyz(m4f::TRS(
        { 1000.0f, 1000.0f, 1000.0f },
        time_s * 0.0f,
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }) * linalg::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    eyePos = xyz(m4f::TRS(
        { 0.0f, 0.0f, 5.0f },
        0.0f,
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }) * vec4 {
        0.0f, 0.0f, 0.0f, 1.0f
    });

    update_input_script(lua, SceneBase::axis_x, SceneBase::axis_y, SceneBase::button_pressed);

    script_system_update(registry, deltaTime_s);
}

void Scene::renderUI()
{
    ImGui::Text("Drawcall count %i", drawcallCount);

    float available_width = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("Reload scripts", ImVec2(available_width, 0.0f)))
    {
        reload_scripts();
    }

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

void Scene::render(
    float time_s,
    int screenWidth,
    int screenHeight,
    ShapeRendererPtr renderer)
{
    // Perspective projection matrix
    const float aspectRatio = float(screenWidth) / screenHeight;
    // const float nearPlane = 1.0f, farPlane = 500.0f;
    // m4f P = m4f::GL_PerspectiveProjectionRHS(60.0f * fTO_RAD, aspectRatio, nearPlane, farPlane);
    // Orthographic projection matrix
    m4f P = m4f::GL_OrthoProjectionRHS(7.5f * aspectRatio, 7.5f, 1.0f, 10.0f);

    // View matrix
    m4f V = m4f::TRS(
        eyePos,
        0.0f,
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }).inverse();

    // Push some test shapes
    // renderer->push_states(Renderer::Color4u::Red);
    // renderer->push_quad(v3f{ 0.0f, 0.0f, 0.0f }, 5.0f);
    // renderer->pop_states<Renderer::Color4u>();

    // Background quad
    renderer->push_states(Renderer::Color4u{ 0x40ffffff });
    renderer->push_quad(v3f{ 0.0f, 0.0f, 0.0f }, 11.0f);
    renderer->pop_states<Renderer::Color4u>();

    // Render QuadComponents
    auto view = registry.view<Transform, QuadComponent>();
    float z = 0.0f;
    for (auto entity : view)
    {
        auto& transform_comp = registry.get<Transform>(entity);

        auto& quad_comp = registry.get<QuadComponent>(entity);
        const auto pos = v3f{ transform_comp.x, transform_comp.y, z += 0.01f };
        const auto size = quad_comp.r;

        // renderer->push_states(Renderer::Color4u::Blue);
        renderer->push_states(Renderer::Color4u{ 0x80ff0000 });
        renderer->push_quad(pos, size);
        renderer->pop_states<Renderer::Color4u>();
    }

    // Render shapes
    renderer->render(P * V);
    renderer->post_render();
}

void Scene::destroy()
{
    registry.clear();
}