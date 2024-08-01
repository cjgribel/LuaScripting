#ifndef AUDIOMANAGER_HPP
#define AUDIOMANAGER_HPP

#include <SDL_mixer.h>
#include <unordered_map>
#include <string>

class AudioManager {
public:
    // Get the singleton instance
    static AudioManager& getInstance();

    // Initialize SDL_mixer
    bool init();

    // Load and register a sound effect
    bool registerEffect(const std::string& name, const std::string& path);

    // Load and register music
    bool registerMusic(const std::string& name, const std::string& path);

    // Play a registered sound effect
    void playEffect(const std::string& name);

    // Play registered music
    void playMusic(const std::string& name);

    // Pause the music
    void pauseMusic();

    // Resume the music
    void resumeMusic();

    // Cleanup resources
    void cleanup();

private:
    AudioManager();
    ~AudioManager();

    // Disallow copy and assignment
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    std::unordered_map<std::string, Mix_Chunk*> effects;
    std::unordered_map<std::string, Mix_Music*> musics;
};

#endif // AUDIOMANAGER_HPP
