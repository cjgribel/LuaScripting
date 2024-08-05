#include "AudioManager.hpp"
#include <SDL2/SDL.h>
#include <iostream>

// Get the singleton instance
AudioManager& AudioManager::getInstance() {
    static AudioManager instance;
    return instance;
}

// Constructor
AudioManager::AudioManager() = default;

// Destructor
AudioManager::~AudioManager() {
    destroy();
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
    // Free existing effect if it exists
    if (effects.count(name)) {
        Mix_FreeChunk(effects[name].first);
    }

    Mix_Chunk* effect = Mix_LoadWAV(path.c_str());
    if (!effect) {
        std::cerr << "Failed to load sound effect! Mix_Error: " << Mix_GetError() << std::endl;
        return false;
    }
    effects[name] = {effect, MIX_MAX_VOLUME}; // Set default volume
    return true;
}

// Load and register music
bool AudioManager::registerMusic(const std::string& name, const std::string& path) {
    // Free existing music if it exists
    if (musics.count(name)) {
        Mix_FreeMusic(musics[name].first);
    }

    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music) {
        std::cerr << "Failed to load music! Mix_Error: " << Mix_GetError() << std::endl;
        return false;
    }
    musics[name] = {music, MIX_MAX_VOLUME}; // Set default volume
    return true;
}

// Play a registered sound effect
void AudioManager::playEffect(const std::string& name, int loops) {
    if (effects.count(name)) {
        Mix_VolumeChunk(effects[name].first, getEffectiveVolume(effects[name].second));
        Mix_PlayChannel(-1, effects[name].first, loops);
    } else {
        std::cerr << "Sound effect not found: " << name << std::endl;
    }
}

// Play registered music
void AudioManager::playMusic(const std::string& name, int loops) {
    if (musics.count(name)) {
        Mix_VolumeMusic(getEffectiveVolume(musics[name].second));
        Mix_PlayMusic(musics[name].first, loops);
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

// Fade in music
void AudioManager::fadeInMusic(const std::string& name, int loops, int ms) {
    if (musics.count(name)) {
        Mix_VolumeMusic(getEffectiveVolume(musics[name].second));
        Mix_FadeInMusic(musics[name].first, loops, ms);
    } else {
        std::cerr << "Music not found: " << name << std::endl;
    }
}

// Fade out music
void AudioManager::fadeOutMusic(int ms) {
    Mix_FadeOutMusic(ms);
}

// Check if music is playing
bool AudioManager::isMusicPlaying() {
    return Mix_PlayingMusic();
}

// Set volume for individual sound effects
void AudioManager::setEffectVolume(const std::string& name, int volume) {
    if (effects.count(name)) {
        effects[name].second = volume;
    } else {
        std::cerr << "Sound effect not found: " << name << std::endl;
    }
}

// Set volume for individual music tracks
void AudioManager::setMusicVolume(const std::string& name, int volume) {
    if (musics.count(name)) {
        musics[name].second = volume;
    } else {
        std::cerr << "Music not found: " << name << std::endl;
    }
}

// Set master volume
void AudioManager::setMasterVolume(int volume) {
    masterVolume = volume;
}

// Get effective volume based on master volume
int AudioManager::getEffectiveVolume(int volume) const {
    return (volume * masterVolume) / MIX_MAX_VOLUME;
}

// Remove a sound effect
void AudioManager::removeEffect(const std::string& name) {
    if (effects.count(name)) {
        Mix_FreeChunk(effects[name].first);
        effects.erase(name);
    } else {
        std::cerr << "Sound effect not found: " << name << std::endl;
    }
}

// Remove music
void AudioManager::removeMusic(const std::string& name) {
    if (musics.count(name)) {
        Mix_FreeMusic(musics[name].first);
        musics.erase(name);
    } else {
        std::cerr << "Music not found: " << name << std::endl;
    }
}

// Clear all registered sound effects and music
void AudioManager::clear() {
    for (auto& pair : effects) {
        Mix_FreeChunk(pair.second.first);
    }
    effects.clear();

    for (auto& pair : musics) {
        Mix_FreeMusic(pair.second.first);
    }
    musics.clear();
}

// Destroy the AudioManager, freeing resources and closing the audio subsystem
void AudioManager::destroy() {
    clear();
    Mix_CloseAudio();
}
