/******************************************************
 * This file includes code from BtOgre:
 * <https://github.com/OGRECave/btogre/tree/30131b4b24c57cd4ca47683deba6fe7f91c9ab32>
 * The code was modified by the Ilmendur team.
 *
 * BtOgre license information:
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 ******************************************************/

#ifndef ILMENDUR_DEBUG_DRAWER_HPP
#define ILMENDUR_DEBUG_DRAWER_HPP
#include <btBulletDynamicsCommon.h>
#include <Ogre.h>

namespace PhysicsSystem {

  /**
   * A class to plug into Bullet’s visual debugging facilities.
   */
  class DebugDrawer: public btIDebugDraw {
  public:
      DebugDrawer(Ogre::SceneNode* node, btDynamicsWorld* world);
      ~DebugDrawer();
      void update();
      void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
      void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
      void reportErrorWarning(const char* warningString);
      void draw3dText(const btVector3& location,const char* textString);
      void setDebugMode(int mode);
      void clear();
      inline int getDebugMode() const { return m_debug_mode; }
  private:
      Ogre::SceneNode* mp_node;
      btDynamicsWorld* mp_bullet_world;

      Ogre::ManualObject m_lines;
      int m_debug_mode;
  };

}


#endif /* ILMENDUR_DEBUG_DRAWER_HPP */
