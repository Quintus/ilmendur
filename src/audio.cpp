#include "audio.hpp"
#include "os.hpp"
#include "util.hpp"
#include <fstream>
#include <cassert>
#include <SDL2/SDL_mixer.h>

using namespace std;
namespace fs = std::filesystem;

AudioSystem::AudioSystem()
{
    for (const fs::directory_entry& iter: fs::directory_iterator(OS::gameDataDir() / fs::u8path("audio") / fs::u8path("music"))) {
        if (iter.path().extension() == fs::u8path(".ogg")) {
            ifstream file(iter.path(), ifstream::in | ifstream::binary);
            string binary(READ_FILE(file));
            assert(binary.size() > 1);

            string name = fs::relative(iter.path(), OS::gameDataDir() / fs::u8path("audio") / fs::u8path("music")).u8string();
            m_music_table[name] = binary;
        }
    }
}

AudioSystem::~AudioSystem()
{
}

void AudioSystem::playBackgroundMusic(const std::string& name)
{
    if (m_current_bg_music == name) {
        return;
    }

    assert(!m_music_table[name].empty());
    SDL_RWops* p_rwops = SDL_RWFromMem(m_music_table[name].data(), m_music_table[name].size());
    Mix_Music* p_music = Mix_LoadMUS_RW(p_rwops, SDL_TRUE);
    assert(Mix_PlayMusic(p_music, -1) == 0);
}

void AudioSystem::stopBackgroundMusic()
{
    assert(Mix_HaltMusic() == 0);
}
