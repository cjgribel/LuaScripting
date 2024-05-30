
#include "imgui.h"
#include "mat.h"
#include "Scene.hpp"
using namespace linalg;
//using namespace Renderer;

bool Scene::init()
{
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