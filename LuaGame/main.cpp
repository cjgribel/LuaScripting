
#include "config.h"
#include "glcommon.h"

// OpenGL debug message callback requires 4.3
#ifdef EENG_GLVERSION_43
#include "GLDebugMessageCallback.h"
#endif

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h> // included by audiomanager?
#include "AudioManager.hpp"

#include <entt/entt.hpp> // -> Scene source

// -> UI source
#include <iostream>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h" // ImGui widgets for std::string

#include "vec.h"
#include "mat.h"
#include "Log.hpp"
#include "ShapeRenderer.hpp"
#include "Scene.hpp"

using namespace linalg;
//const int WINDOW_WIDTH = 1600;
//const int WINDOW_HEIGHT = 900;
v2i gWindowSize{ 1600, 900 };
unsigned int FRAMETIME_MIN_MS = (unsigned int)(1000.0f / 60);
bool WIREFRAME = false;
bool SOUND_PLAY = false;
int DRAWCALL_COUNT;

SDL_GameController* controller1;

namespace
{
    // Helpers

    SDL_GameController* findController()
    {
        for (int i = 0; i < SDL_NumJoysticks(); i++)
        {
            if (SDL_IsGameController(i))
            {
                return SDL_GameControllerOpen(i);
            }
        }

        return nullptr;
    }

    // Circular buffer class
    class CircularBuffer {
    public:
        CircularBuffer(size_t size) : buffer(size), head(0), full(false) {}

        void add(float value) {
            buffer[head] = value;
            head = (head + 1) % buffer.size();
            if (head == 0) {
                full = true;
            }
        }

        std::vector<float> getBuffer() const {
            std::vector<float> result;
            if (full) {
                result.insert(result.end(), buffer.begin() + head, buffer.end());
            }
            result.insert(result.end(), buffer.begin(), buffer.begin() + head);
            return result;
        }

    private:
        std::vector<float> buffer;
        size_t head;
        bool full;
    };

    class PeriodicEvent
    {
        float t, delay, next_event;
        std::function<void()> event;

    public:
        PeriodicEvent(float hertz, const std::function<void()>&& event)
            : t(0.0f), delay(1.0f / hertz), next_event(0.0f), event(event) {}

        void update(float dt)
        {
            t += dt;
            while (t > next_event)
            {
                next_event += delay;
                event();
            }
        }
    };
}

// TODO: To some global state?
// namespace {
//     // Load a sound effect
//     Mix_Chunk* loadSoundEffect(const char* path) {
//         Mix_Chunk* effect = Mix_LoadWAV(path);
//         if (!effect) {
//             std::cerr << "Failed to load sound effect! Mix_Error: " << Mix_GetError() << std::endl;
//         }
//         return effect;
//     }

//     // Load music
//     Mix_Music* loadMusic(const char* path) {
//         Mix_Music* music = Mix_LoadMUS(path);
//         if (!music) {
//             std::cerr << "Failed to load music! Mix_Error: " << Mix_GetError() << std::endl;
//         }
//         return music;
//     }
// }

int main(int argc, char* argv[])
{
    // Hello standard output
    std::cout << "Hello SDL + Dear ImGui + enTT + Lua + sol" << std::endl;

    // Initialize SDL
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    AudioManager::getInstance().init();
    // // Initialize SDL_mixer with a specific audio format
    // if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    //     std::cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << std::endl;
    //     return 1;
    // }

    // Controllers
    controller1 = findController();

    // OpenGL context attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, EENG_GLVERSION_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, EENG_GLVERSION_MINOR);
#ifdef EENG_MSAA
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, EENG_MSAA_SAMPLES);
#endif

    // Create a window
    SDL_Window* window = SDL_CreateWindow("Hello SDL2 + sol2 + enTT + Dear ImGui",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        gWindowSize.x,
        gWindowSize.y,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (!window)
    {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create an OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;

        Mix_CloseAudio();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Make the OpenGL context current
    if (SDL_GL_MakeCurrent(window, gl_context) != 0)
    {
        std::cerr << "Failed to make OpenGL context current: " << SDL_GetError() << std::endl;
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    // OpenGL debug output callback
#ifdef EENG_GLVERSION_43
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugMessageCallback, nullptr);
#endif

    // Check for OpenGL errors before initializing ImGui
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error before initializing ImGui: " << error << std::endl;
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // ImGui fonts
    io.Fonts->AddFontFromFileTTF("../../assets/fonts/ProggyClean.ttf", 13.0f); // Default
    io.Fonts->AddFontFromFileTTF("../../assets/fonts/ProggyClean.ttf", 26.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    const char* glsl_version = "#version 410 core";
    if (!ImGui_ImplOpenGL3_Init(glsl_version))
    {
        std::cerr << "Failed to initialize ImGui with OpenGL backend" << std::endl;
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Check for OpenGL errors after initializing ImGui
    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after initializing ImGui: " << error << std::endl;
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

#if 0
    // Load sound effects and music
    Mix_Chunk* effect1 = loadSoundEffect("../../assets/sounds/Misc Lasers/Fire 1.mp3");
    Mix_Chunk* effect2 = loadSoundEffect("../../assets/sounds/Misc Lasers/Fire 2.mp3");
    Mix_Music* music = loadMusic("../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Title Screen.wav");

    if (!effect1 || !effect2 || !music) {
        // close(music, effect1, effect2);
        Mix_FreeMusic(music);
        Mix_FreeChunk(effect1);
        Mix_FreeChunk(effect2);
        return 1;
    }
    else
        std::cout << "Sound and music loaded..." << std::endl;

#if 1
    // SDL2_mixer

    // Play music (loop indefinitely)
    if (Mix_PlayMusic(music, -1) == -1) {
        std::cerr << "Failed to play music! Mix_Error: " << Mix_GetError() << std::endl;
    }

    // Play sound effects (loop indefinitely)
    if (Mix_PlayChannel(-1, effect1, -1) == -1) {
        std::cerr << "Failed to play sound effect 1! Mix_Error: " << Mix_GetError() << std::endl;
    }

    if (Mix_PlayChannel(-1, effect2, -1) == -1) {
        std::cerr << "Failed to play sound effect 2! Mix_Error: " << Mix_GetError() << std::endl;
    }
#endif
#endif
#if 0
    // Load and play an audio clip
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8* wavBuffer;
    SDL_AudioDeviceID deviceId = 0; // Declare deviceId outside of the if block
    // Load sound
    std::cout << "Playing sound..." << std::endl;
    if (SDL_LoadWAV("assets/sound/Juhani Junkala [Retro Game Music Pack] Title Screen.wav", &wavSpec, &wavBuffer, &wavLength) == NULL)
    {
        std::cerr << "Failed to load audio: " << SDL_GetError() << std::endl;
    }
    else
    {
        deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
        if (deviceId == 0)
        {
            std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        }
        else
        {
            // Enqueue the same sound ten times
            for (int i = 0; i < 10; i++)
                SDL_QueueAudio(deviceId, wavBuffer, wavLength);
            // SDL_PauseAudioDevice(deviceId, 0);
        }
    }
#endif

    // Log some state
    LOG_DEFINES(eeng::Log);
    {
        int glMinor, glMajor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinor);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajor);
        eeng::Log("GL version %i.%i (requested), %i.%i (actual)", EENG_GLVERSION_MAJOR, EENG_GLVERSION_MINOR, glMajor, glMinor);
    }
#ifdef EENG_MSAA
    {
        int actualMSAA;
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &actualMSAA);
        eeng::Log("MSAA %i (requested), %i (actual)", EENG_MSAA_SAMPLES, actualMSAA);
    }
#endif
#ifdef EENG_ANISO
    {
        GLfloat maxAniso;
#if defined(EENG_GLVERSION_43)
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
#elif defined(EENG_GLVERSION_41)
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
#endif
        eeng::Log("Anisotropic samples %i (requested), %i (max))", EENG_ANISO_SAMPLES, (int)maxAniso);
    }
#endif

    auto renderer = std::make_shared<Renderer::ImPrimitiveRenderer>();
    renderer->init();

    auto scene = std::make_shared<Scene>();
    scene->init(gWindowSize);

    // Main loop

    float time_s = 0.0f, deltaTime_s = 0.016f;
    Uint32 time_ms = 0;
    Uint32 elapsed_ms = 0;  // Effective frame time
    Uint32 idle_ms = 0;     // Idle frame time

    bool quit = false;
    SDL_Event event;
    eeng::Log("Entering main loop...");

    // Sound test
    AudioManager::getInstance().registerEffect("testfire1", "../../assets/sounds/Misc Lasers/Fire 1.mp3");
    AudioManager::getInstance().registerEffect("testfire2", "../../assets/sounds/Misc Lasers/Fire 2.mp3");
    AudioManager::getInstance().registerMusic("testmusic1", "../../assets/sounds/music/Juhani Junkala [Retro Game Music Pack] Title Screen.wav");
    //

    while (!quit)
    {
        const Uint32 now_ms = SDL_GetTicks();
        const float now_s = now_ms * 0.001f;
        deltaTime_s = now_s - time_s;
        time_ms = now_ms;
        time_s = now_s;

        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event); // Send events to ImGui

            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_CONTROLLERDEVICEADDED:
                if (!controller1)
                {
                    controller1 = SDL_GameControllerOpen(event.cdevice.which);
                }
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                if (controller1 && event.cdevice.which == SDL_JoystickInstanceID(
                    SDL_GameControllerGetJoystick(controller1)))
                {
                    SDL_GameControllerClose(controller1);
                    controller1 = findController();
                }
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                break;
            }
        }

        if (SDL_GameControllerGetButton(controller1, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X))
        {
            // printf("X was pressed!\n");
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        // Render GUI here
        {
            ImGui::Begin("Info");

            if (ImGui::CollapsingHeader("Backend", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // Total frame time & fps
                {
                    static float stable_fps = 1.0f;
                    static PeriodicEvent pevent{
                        1.0f, [&]() { stable_fps = ImGui::GetIO().Framerate; }
                    };
                    pevent.update(deltaTime_s);
                    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / stable_fps, stable_fps);
                }

                // Frame time plot
                {
                    static CircularBuffer fpsBuffer(100);
                    static PeriodicEvent pevent{ 10.0f, [&]() {
                        fpsBuffer.add(elapsed_ms);
                    } };
                    pevent.update(deltaTime_s);
                    const auto& buffer = fpsBuffer.getBuffer();
                    // ImGui::Text("Frame time");
                    if (!buffer.empty())
                    {
                        float available_width = ImGui::GetContentRegionAvail().x;
                        ImGui::PlotLines("##FrameTimePlot", buffer.data(), static_cast<int>(buffer.size()), 0, "Frame time (max 30 ms)", 0.0f, 30.0f, ImVec2(available_width, 80));
                    }
                }

                // Combo (drop-down) for fps settings
                static const char* items[] = { "10", "30", "60", "120", "Uncapped" };
                static int currentItem = 2;
                if (ImGui::BeginCombo("Target framerate##targetfps", items[currentItem]))
                {
                    for (int i = 0; i < IM_ARRAYSIZE(items); i++)
                    {
                        const bool isSelected = (currentItem == i);
                        if (ImGui::Selectable(items[i], isSelected))
                            currentItem = i;

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                if (currentItem == 0)
                    FRAMETIME_MIN_MS = (unsigned int)(1000.0f / 10);
                else if (currentItem == 1)
                    FRAMETIME_MIN_MS = (unsigned int)(1000.0f / 30);
                else if (currentItem == 2)
                    FRAMETIME_MIN_MS = (unsigned int)(1000.0f / 60);
                else if (currentItem == 3)
                    FRAMETIME_MIN_MS = (unsigned int)(1000.0f / 120);
                else if (currentItem == 4)
                    FRAMETIME_MIN_MS = 0;

                ImGui::Checkbox("Wireframe rendering", &WIREFRAME);

#if 1
                // AudioManager::getInstance().playEffect("testfire1");
                // AudioManager::getInstance().playEffect("testfire2");
                // AudioManager::getInstance().playMusic("testmusic1");

                if (ImGui::Button("Play music"))
                    AudioManager::getInstance().playMusic("testmusic1");

                ImGui::SameLine();
                if (ImGui::Button("Pause music"))
                    AudioManager::getInstance().pauseMusic();

                ImGui::SameLine();
                if (ImGui::Button("Play effect 1"))
                    AudioManager::getInstance().playEffect("testfire1");

                ImGui::SameLine();
                if (ImGui::Button("Play effect 2"))
                    AudioManager::getInstance().playEffect("testfire2");
#endif
#if 0
                // SDL standard sounsd system
                if (SOUND_PLAY)
                {
                    if (ImGui::Button("Pause sound"))
                    {
                        SDL_PauseAudioDevice(deviceId, 1);
                        SOUND_PLAY = false;
                    }
                }
                else
                {
                    if (ImGui::Button("Play sound"))
                    {
                        SDL_PauseAudioDevice(deviceId, 0);
                        SOUND_PLAY = true;
                    }
                }
#endif

                ImGui::Text("Controller State");

                if (controller1 != nullptr)
                {
                    ImGui::BeginChild("Controller State Frame", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4), true);
                    ImGui::Text("Buttons: A:%d B:%d X:%d Y:%d",
                        SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_A),
                        SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_B),
                        SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_X),
                        SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_Y));

                    ImGui::Text("Left Stick: X:%.2f Y:%.2f",
                        SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f,
                        SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f);

                    ImGui::Text("Right Stick: X:%.2f Y:%.2f",
                        SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f,
                        SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f);
                    ImGui::EndChild();
                }
                else
                {
                    ImGui::SameLine();
                    ImGui::Text("(No controller connected)");
                }
            }

            if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen))
            {
                scene->renderUI();
                // Default dark theme blue values
                // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.40f));
                // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
                // ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.06f, 0.53f, 0.98f, 1.00f));
                // Green-ish version
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.98f, 0.59f, 0.40f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.98f, 0.59f, 1.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.06f, 0.78f, 0.35f, 1.00f));
                float available_width = ImGui::GetContentRegionAvail().x;
                if (ImGui::Button("Reload Scene", ImVec2(available_width, 0.0f)))
                {
                    std::cout << "Destroying Scene from main ..." << std::endl;
                    scene->destroy();

                    std::cout << "Creating new Scene from main ..." << std::endl;
                    scene = std::make_shared<Scene>();
                    scene->init(gWindowSize);
                }
                ImGui::PopStyleColor(3);
            }

            ImGui::End(); // end info window
        }
        eeng::LogDraw("Log");

        // Face culling - takes place before rasterization
        glEnable(GL_CULL_FACE); // Perform face culling
        glFrontFace(GL_CCW);    // Define winding for a front-facing face
        glCullFace(GL_BACK);    // Cull back-facing faces
        // Rasterization stuff
        glEnable(GL_DEPTH_TEST); // Perform depth test when rasterizing
        glDepthFunc(GL_LESS);    // Depth test pass if z < existing z (closer than existing z)
        glDepthMask(GL_FALSE);    // If depth test passes, write z to z-buffer
        glDepthRange(0, 1);      // Z-buffer range is [0,1], where 0 is at z-near and 1 is at z-far

        // Define viewport transform = Clip -> Screen space (applied before rasterization)
        glViewport(0, 0, gWindowSize.x, gWindowSize.y);

        // Bind the default framebuffer (only needed when using multiple render targets)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Clear depth and color attachments of frame buffer
        // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        // glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (WIREFRAME)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_CULL_FACE);
        }

        // Update input
        const v4f axes{
            SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f,
            SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f,
            SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f,
            SDL_GameControllerGetAxis(controller1, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f
        };
        const vec4<bool> buttons{
            SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_A),
            SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_B),
            SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_X),
            SDL_GameControllerGetButton(controller1, SDL_CONTROLLER_BUTTON_Y)
        };
        scene->update_input(axes, buttons);

        // Update scene
        scene->update(time_s, deltaTime_s);

        // Render scene
        scene->render(time_s, renderer);

        // Render GUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);

        // Add a delay if frame time was faster than the target frame time
        elapsed_ms = SDL_GetTicks() - time_ms;
        if (elapsed_ms < FRAMETIME_MIN_MS)
        {
            idle_ms = FRAMETIME_MIN_MS - elapsed_ms;
            SDL_Delay(idle_ms);
        }
        else idle_ms = 0;

        // Example: Play the sound again after 5 seconds
        //        SDL_Delay(5000);
        //        if (!quit) {
        //            SDL_QueueAudio(deviceId, wavBuffer, wavLength);
        //            SDL_PauseAudioDevice(deviceId, 0);
        //        }
    }

    eeng::Log("Exiting...");

    scene->destroy();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    AudioManager::getInstance().destroy();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
