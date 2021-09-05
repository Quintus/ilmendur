#ifndef ILMENDUR_MUSIC_HPP
#define ILMENDUR_MUSIC_HPP
#include <string>

namespace AudioSystem {

    class Music
    {
    public:
        Music(const std::string& name);
        ~Music();

        const std::string& getName();

        // Set/get whether the music is intended to loop. If looping
        // is enabled, play() will honour the loop time points, allowing
        // a music to loop only over a part (used to specifically play
        // only once the beginning of a music).
        void setLoop(bool loop);
        bool doesLoop();

        // Set/get pitch (perceived frequency of the music, “highness”).
        void setPitch(float pitch);
        float getPitch();

        // Set/get volume.
        void setVolume(float volume);
        float getVolume();

        void play(); // Play it from the beginning or pause point, loop if requested.
        void pause(); // Pause it; play() will resume from here.
        void stop(); // Stop the music and reset it to the beginning (even before possible loop points).
        void reset(); // Shortcut for calling stop() followed by play().
    };
};

#endif /* ILMENDUR_MUSIC_HPP */
