#include "AudioManager.hpp"
#include <SDL.h>
#include <iostream>
#include "Log.hpp"

// Get the singleton instance
AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

// Constructor
AudioManager::AudioManager() = default;

// Destructor
AudioManager::~AudioManager() {
    cleanup();
}

// Initialize SDL_mixer
bool AudioManager::init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << std::endl;
        return false;
    }
    return true;
}

// Load and register a sound effect
bool AudioManager::registerEffect(const std::string& name, const std::string& path) {
    Mix_Chunk* effect = Mix_LoadWAV(path.c_str());
    if (!effect) {
        std::cerr << "Failed to load sound effect! Mix_Error: " << Mix_GetError() << std::endl;
        return false;
    }
    effects[name] = effect;

    eeng::Log::log((std::string("Registered effect ") + name).c_str());
    return true;
}

// Load and register music
bool AudioManager::registerMusic(const std::string& name, const std::string& path) {
    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music) {
        std::cerr << "Failed to load music! Mix_Error: " << Mix_GetError() << std::endl;
        return false;
    }
    musics[name] = music;
    
    eeng::Log::log((std::string("Registered music ") + name).c_str());
    return true;
}

// Play a registered sound effect
void AudioManager::playEffect(const std::string& name) {
    if (effects.count(name)) {
        Mix_PlayChannel(-1, effects[name], 0);
    } else {
        std::cerr << "Sound effect not found: " << name << std::endl;
    }
}

// Play registered music
void AudioManager::playMusic(const std::string& name) {
    if (musics.count(name)) {
        Mix_PlayMusic(musics[name], -1);
    } else {
        std::cerr << "Music not found: " << name << std::endl;
    }
}

// Pause the music
void AudioManager::pauseMusic() {
    Mix_PauseMusic();
}

// Resume the music
void AudioManager::resumeMusic() {
    Mix_ResumeMusic();
}

// Cleanup resources
void AudioManager::cleanup() {
    for (auto& pair : effects) {
        Mix_FreeChunk(pair.second);
    }
    effects.clear();

    for (auto& pair : musics) {
        Mix_FreeMusic(pair.second);
    }
    musics.clear();

    Mix_CloseAudio();
}
