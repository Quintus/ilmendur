#ifndef ILMENDUR_AUDIO_HPP
#define ILMENDUR_AUDIO_HPP
#include <string>
#include <map>

class AudioSystem
{
public:
    AudioSystem();
    ~AudioSystem();

    void playBackgroundMusic(const std::string& name);
    void stopBackgroundMusic();
    bool isPlayingBackgroundMusic() const;

    enum class channel {
        any = -1,
        ui = 10 // UI channel should be a high one so it is normally free
    };
    channel playSound(const std::string& name, channel chan);
    channel playSoundBlocking(const std::string& name, channel chan);

private:
    std::map<std::string, std::string> m_music_table;
    std::map<std::string, void*> m_sound_table;

    struct {
        std::string name;
        void* p_sdl_handle;
    } m_current_bg_music;
};

#endif /* ILMENDUR_AUDIO_HPP */
