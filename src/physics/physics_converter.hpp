#ifndef ILMENDUR_PHYSICS_CONVERTER_HPP
#define ILMENDUR_PHYSICS_CONVERTER_HPP

class btCollisionShape;
namespace Ogre { class Entity; }

namespace PhysicsSystem {

    enum class ColliderType;

    btCollisionShape* calculateCollisionShape(Ogre::Entity* p_entity, ColliderType ctype);
}

#endif /* ILMENDUR_PHYSICS_CONVERTER_HPP */
