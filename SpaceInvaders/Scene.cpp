
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

    void update_input_script(sol::state& lua, float x, float y, bool button_pressed)
    {
        lua["update_input"](x, y, button_pressed);
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

    sol::table get_script(
        entt::registry& registry,
        entt::entity entity,
        const std::string& identifier)
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
}

bool Scene::init()
{
    assert(!is_initialized);
    std::cout << "Scene::init()" << std::endl;

    // Register registry meta functions to components
    register_meta_component<Transform>();
    register_meta_component<QuadComponent>();
    register_meta_component<CircleColliderComponent>();
    register_meta_component<ScriptedBehaviorComponent>();

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

        // Placeholder particle emitter function
        const auto emit_particle = [&](
            float x,
            float y,
            float vx,
            float vy,
            uint32_t color)
            {
                particleBuffer.push_point(v3f{ x, y, 0.01f }, v3f{ vx, vy, 0.0f }, color);
            };
        lua["emit_particle"] = emit_particle;

        // Register to Lua: input module
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

        // Register to Lua: component types
        register_transform(lua);
        registerQuadComponent(lua);
        registerCircleColliderComponent(lua);
        registerScriptedBehaviorComponent(lua);

        // Run init script
        lua.safe_script_file("lua/init.lua");

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
        // return false;
    }

    is_initialized = true;
    return true;
}

void Scene::update(float time_s, float deltaTime_s)
{
    assert(is_initialized);

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

    particleBuffer.update(deltaTime_s);

    script_system_update(registry, deltaTime_s);

    // Placeholder collision system
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

void Scene::render(
    float time_s,
    int screenWidth,
    int screenHeight,
    ShapeRendererPtr renderer)
{
    assert(is_initialized);

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
    renderer->push_quad(v3f{ 0.0f, 0.0f, 0.0f }, 10.0f);
    renderer->pop_states<Renderer::Color4u>();

    // Render QuadComponents
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

    // Add some test particles
    const int N = 1;
    for (int i = 0; i < N; i++)
    {
        const float angle = fPI / N * i;
        const float x = std::cos(angle);
        const float y = std::sin(angle);
        particleBuffer.push_point(v3f{ 0.0f, 0.0f, 0.0f }, v3f{ x, y, 0.0f } *4, 0xff0000ff);
    }

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