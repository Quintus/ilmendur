#ifndef ILMENDUR_PHYSICS_CONVERTER_HPP
#define ILMENDUR_PHYSICS_CONVERTER_HPP
#include <OGRE/Ogre.h>

class btCollisionShape;

namespace PhysicsSystem {

    enum class ColliderType;

    btCollisionShape* calculateCollisionShape(Ogre::Entity* p_entity, ColliderType ctype);
}

#endif /* ILMENDUR_PHYSICS_CONVERTER_HPP */
