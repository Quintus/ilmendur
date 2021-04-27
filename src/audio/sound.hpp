#ifndef RPG_SOUND_HPP
#define RPG_SOUND_HPP
#include "../core/point.hpp"
#include <OGRE/OgreVector.h>
#include <string>

namespace AudioSystem {

    class Sound
    {
    public:
        Sound(const std::string& name);
        ~Sound();

        const std::string& getName();

        // Set/get pitch (perceived frequency of the music, “highness”).
        void setPitch(float pitch);
        float getPitch();

        // Set/get volume.
        void setVolume(float volume);
        float getVolume();

        // Sound spatialization: where in a 3D room is the sound emitted?
        void setPosition(Ogre::Vector3 pos);
        Ogre::Vector3 getPosition();

        // Play the sound.
        void play();

        // Sound spatialization: where in a 3D room is the listener?
        // Where does he look? The coordinates passed to setListenerPosition()
        // will be world coordinates.
        static void setListenerPosition(Ogre::Vector3 pos);
        static void setListenerDirection(Ogre::Vector3 direction);
        static Ogre::Vector3 getListenerPosition();
        static Ogre::Vector3 getListenerDirection();
    };
}

#endif /* RPG_SOUND_HPP */
