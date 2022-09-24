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

private:
    std::map<std::string, std::string> m_music_table;
    std::string m_current_bg_music;
};

#endif /* ILMENDUR_AUDIO_HPP */
