
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

    // void inspect_script(const ScriptComponent& script)
    // {
    //     script.self.for_each([](const sol::object& key, const sol::object& value)
    //         { std::cout << key.as<std::string>() << ": "
    //         << sol::type_name(value.lua_state(), value.get_type())
    //         << std::endl; });
    // }

    void init_script(entt::registry& registry, entt::entity entity)
    {
        auto& script_comp = registry.get<ScriptComponent>(entity);
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
        auto& script_comp = registry.get<ScriptComponent>(entity);
        for (auto& script : script_comp.scripts)
        {
            // auto& script = registry.get<ScriptComponent>(entity);
            if (auto&& f = script.self["destroy"]; f.valid())
                f(script.self);
            script.self.abandon();
        }
    }

    void script_system_update(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<ScriptComponent>();
        for (auto entity : view)
        {
            auto& script_comp = registry.get<ScriptComponent>(entity);
            for (auto& script : script_comp.scripts)
            {
                // auto& script = view.get<ScriptComponent>(entity);
                assert(script.self.valid());
                script.update(script.self, delta_time);
            }
        }
    }

    // void register_input_module(sol::state& lua)
    // {
    //     // lua.script_file("lua/input.lua");
    //     lua.safe_script_file("lua/input.lua");
    // }

    // Register the input module with Lua
    void register_input_module(sol::state& lua) {
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

    void update_input(sol::state& lua, float x, float y, bool button_pressed) {
        lua["update_input"](x, y, button_pressed);
    }

}

bool Scene::init()
{
#if 1

    try
    {
        // 
        register_meta_component<Transform>();

        // ScriptComponent creation & destruction callbacks
        registry.on_construct<ScriptComponent>().connect<&init_script>();
        registry.on_destroy<ScriptComponent>().connect<&release_script>();

        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);

        // Register input module
        register_input_module(lua);
        if (!lua["input"].valid()) {
            std::cerr << "Error: 'input' table not loaded properly" << std::endl;
            return -1;
        }
        update_input(lua, 100.0f, 200.0f, true);

        lua.require("registry", sol::c_call<AUTO_ARG(&open_registry)>, false);

        register_transform(lua); // Make Transform struct available to Lua

        // Add 5x of a test behavior script
        // Requires the 'input' module to be registered
        sol::load_result behavior_script = lua.load_file("lua/behavior.lua");
        sol::protected_function script_function = behavior_script;
        assert(behavior_script.valid());

        for (int i = 0; i < 5; ++i)
        {
            auto e = registry.create();
            registry.emplace<Transform>(e, Transform{ (float)i, (float)i });

            sol::table script_table = script_function();

            ScriptComponent script_comp;
            ScriptComponent::Script script{ script_table };
            script_comp.scripts.push_back(script);
            registry.emplace<ScriptComponent>(e, script_comp);
        }

        using namespace std::chrono_literals;

        // constexpr auto target_frame_time = 500ms;
        int delta_time_ms{ 1000 };

        while (true)
        {
            using clock = std::chrono::high_resolution_clock;
            const auto begin_ticks = clock::now();

            script_system_update(registry, delta_time_ms * 0.001f);
            // std::this_thread::sleep_for(target_frame_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_time_ms));

            //delta_time = std::chrono::duration_cast<fsec>(clock::now() - begin_ticks);
            //if (delta_time > 1s)
            //    delta_time = target_frame_time;

            if (_kbhit())
                break;
        }
        registry.clear();
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
        { 0.0f, 0.0f, 10.0f },
        0.0f,
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }) * vec4 {
        0.0f, 0.0f, 0.0f, 1.0f
    });


}

void Scene::renderUI()
{
    ImGui::Text("Drawcall count %i", drawcallCount);

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
    // Projection matrix
    const float aspectRatio = float(screenWidth) / screenHeight;
    const float nearPlane = 1.0f, farPlane = 500.0f;
    m4f P = m4f::GL_PerspectiveProjectionRHS(60.0f * fTO_RAD, aspectRatio, nearPlane, farPlane);

    // View matrix
    m4f V = m4f::TRS(
        eyePos,
        0.0f,
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }).inverse();

    // Push shapes
    renderer->push_states(Renderer::Color4u::Red);
    renderer->push_quad(v3f{ 0.0f, 0.0f, 0.0f }, 5.0f);
    renderer->pop_states<Renderer::Color4u>();

    // Render shapes
    renderer->render(P * V);
    renderer->post_render();
}

void Scene::destroy()
{

}