
#include "imgui.h"
#include "mat.h"
#include "Scene.hpp"

// entt-sol2
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "bond.hpp"
#include "transform.hpp"

#define AUTO_ARG(x) decltype(x), x
using namespace linalg;

inline void my_panic(sol::optional<std::string> maybe_msg) {
    std::cerr << "Lua is in a panic state and will now abort() the application" << std::endl;
    if (maybe_msg) {
        const std::string& msg = maybe_msg.value();
        std::cerr << "\terror message: " << msg << std::endl;
    }
    // When this function exits, Lua will exhibit default behavior and abort()
}

bool Scene::init()
{
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

        std::cout << "All Transforms:" << std::endl;
        auto view = registry.view<Transform>();
        for (auto& ent : view)
        {
            auto& t = view.get<Transform>(ent);
            std::cout << t.to_string() << std::endl;
        }

        const auto bowser = lua["bowser"].get<entt::entity>();
        const auto* xf = registry.try_get<Transform>(bowser);
        assert(xf != nullptr);
        const Transform& transform = lua["transform"];
        assert(xf->x == transform.x && xf->y == transform.y);


        entt::runtime_view view2{};
        view2.iterate(registry.storage<Transform>());
        std::cout << "Initial size hint: " << view2.size_hint() << std::endl;
        //
        entt::runtime_view view3{};
        view3.iterate(registry.storage<Transform>());
        for (auto ent : view3)
            registry.remove<Transform>(ent);
        //
        entt::runtime_view view4{};
        view4.iterate(registry.storage<Transform>());
        // auto view2 = registry.runtime_view<Transform>();
        std::cout << "Size hint after removal: " << view4.size_hint() << std::endl;


        /*
                //
                // lua.do_file("lua/iterate_entities.lua");
                lua.safe_script_file("lua/iterate_entities.lua");
                assert(registry.orphan(bowser) && "The only component (Transform) should  "
                    "be removed by the script");
        */
        std::cout << "All Transforms:" << std::endl;
        auto view2_ = registry.view<Transform>();
        for (auto& ent : view2_)
        {
            auto& t = view2_.get<Transform>(ent);
            std::cout << t.to_string() << std::endl;
        }
    }
    catch (const sol::error& e)
    {
        std::cerr << "Lua script execution failed: " << e.what() << std::endl;
        return false;
    }

    // Do some entt stuff
    entt::registry registry;
    auto ent1 = registry.create();
    struct Tfm
    {
        float x, y, z;
    };
    registry.emplace<Tfm>(ent1, Tfm{});

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