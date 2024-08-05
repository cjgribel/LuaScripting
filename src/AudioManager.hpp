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
    void playEffect(const std::string& name, int loops = 0);

    // Play registered music
    void playMusic(const std::string& name, int loops = -1);

    // Pause the music
    void pauseMusic();

    // Resume the music
    void resumeMusic();

    // Fade in music
    void fadeInMusic(const std::string& name, int loops = -1, int ms = 1000);

    // Fade out music
    void fadeOutMusic(int ms = 1000);

    // Check if music is playing
    bool isMusicPlaying();

    // Set volume for individual sound effects
    void setEffectVolume(const std::string& name, int volume);

    // Set volume for individual music tracks
    void setMusicVolume(const std::string& name, int volume);

    // Set master volume
    void setMasterVolume(int volume);

    // Remove a sound effect
    void removeEffect(const std::string& name);

    // Remove music
    void removeMusic(const std::string& name);

    // Clear all registered sound effects and music
    void clear();

    // Destroy the AudioManager, freeing resources and closing the audio subsystem
    void destroy();

private:
    AudioManager();
    ~AudioManager();

    // Disallow copy and assignment
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    int masterVolume = MIX_MAX_VOLUME; // Master volume (default is max)
    std::unordered_map<std::string, std::pair<Mix_Chunk*, int>> effects;
    std::unordered_map<std::string, std::pair<Mix_Music*, int>> musics;

    // Helper function to get effective volume
    int getEffectiveVolume(int volume) const;
};

#endif // AUDIOMANAGER_HPP
