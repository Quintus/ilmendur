#include "audio.hpp"
#include "os.hpp"
#include "util.hpp"
#include <fstream>
#include <cassert>
#include <thread>
#include <chrono>
#include <SDL2/SDL_mixer.h>

using namespace std;
namespace fs = std::filesystem;

/**
 * Creates the audio system, preloading from disk what is necessary.
 */
AudioSystem::AudioSystem()
{
    // TODO: Maybe not a good idea to preload all music into memory; could grow large!
    // It is only done here because SDL's own file loading functions are unable
    // to deal with Unicode path names, so C++11's filesystem library can be used.
    for (const fs::directory_entry& iter: fs::directory_iterator(OS::gameDataDir() / fs::u8path("audio") / fs::u8path("music"))) {
        if (iter.path().extension() == fs::u8path(".ogg")) {
            ifstream file(iter.path(), ifstream::in | ifstream::binary);
            string binary(READ_FILE(file));
            assert(binary.size() > 1);

            string name = fs::relative(iter.path(), OS::gameDataDir() / fs::u8path("audio") / fs::u8path("music")).u8string();
            m_music_table[name] = binary;
        }
    }

    // Preload all sounds into memory (no disk access please for short sounds)
    // Note that the sounds are stored in decoded form in m_sound_table.
    for (const fs::directory_entry& iter: fs::recursive_directory_iterator(OS::gameDataDir() / fs::u8path("audio") / fs::u8path("sounds"))) {
        if (iter.path().extension() == fs::u8path(".ogg")) {
            ifstream file(iter.path(), ifstream::in | ifstream::binary);
            string binary(READ_FILE(file));
            assert(binary.size() > 1);

            string name = fs::relative(iter.path(), OS::gameDataDir() / fs::u8path("audio") / fs::u8path("sounds")).u8string();
            SDL_RWops* p_rwops = SDL_RWFromMem(binary.data(), binary.size());
            m_sound_table[name] = Mix_LoadWAV_RW(p_rwops, SDL_TRUE); // frees p_rwops, and returns a completely decoded version of `binary'.
            // Note that in contrast to music loading, it is not required
            // to keep `binary' around.
        }
    }
}

/**
 * Stops the background music and frees all audio memory.
 */
AudioSystem::~AudioSystem()
{
    stopBackgroundMusic();
    for(auto iter: m_sound_table) {
        Mix_FreeChunk(static_cast<Mix_Chunk*>(iter.second));
    }
    m_sound_table.clear();
}

/**
 * Switch to the requested background music, which is a path relative to `audio/music`.
 */
void AudioSystem::playBackgroundMusic(const std::string& name)
{
    if (m_current_bg_music.name == name) {
        return;
    }

    assert(m_music_table.count(name) != 0);
    SDL_RWops* p_rwops = SDL_RWFromMem(m_music_table[name].data(), m_music_table[name].size());
    Mix_Music* p_music = Mix_LoadMUS_RW(p_rwops, SDL_TRUE);
    assert(Mix_PlayMusic(p_music, -1) == 0);

    m_current_bg_music.name = name;
    m_current_bg_music.p_sdl_handle = p_music;
}

/**
 * Stops the current background music, if there is any. If not, this
 * method does nothing.
 */
void AudioSystem::stopBackgroundMusic()
{
    if (isPlayingBackgroundMusic()) {
        assert(Mix_HaltMusic() == 0);
        Mix_FreeMusic(static_cast<Mix_Music*>(m_current_bg_music.p_sdl_handle));
        m_current_bg_music.name.clear();
        m_current_bg_music.p_sdl_handle = nullptr;
    }
}

/**
 * Queries whether there is a background music currently playing.
 */
bool AudioSystem::isPlayingBackgroundMusic() const
{
    return !m_current_bg_music.name.empty();
}

/**
 * Plays a sound. `name` is the path relative to the audio/sounds directory.
 * Returns the channel used.
 *
 * There may only ever be one sound being played at once in any given
 * channel. If you request a sound to be played in a channel that is
 * already playing a sound, the sound will be replaced with the new
 * one. This is not normally the desired effect, so as an exception,
 * if channel::any is specified for `chan`, a random free channel will
 * be picked. This is the only case where the return value of this
 * method does not equal the value of `chan` passed; it will never
 * return channel::any.
 *
 * This method crashes with an assertion failure if the sound cannot
 * be played for whatever reason.
 */
AudioSystem::channel AudioSystem::playSound(const std::string& name, AudioSystem::channel chan)
{
    assert(m_sound_table.count(name) != 0);
    int result = Mix_PlayChannel(static_cast<int>(chan), static_cast<Mix_Chunk*>(m_sound_table[name]), 0);
    assert(result != -1);
    return static_cast<AudioSystem::channel>(result);
}

/**
 * Like `playSound()', but blocks until the sound has finished playing.
 * The blocking time is not exact, but resolves with a resolution of
 * about 25 milliseconds.
 */
AudioSystem::channel AudioSystem::playSoundBlocking(const std::string& name, AudioSystem::channel chan)
{
    chan = playSound(name, chan);
    while (Mix_Playing(static_cast<int>(chan))) {
        this_thread::sleep_for(chrono::milliseconds(25));
    }

    return chan;
}
