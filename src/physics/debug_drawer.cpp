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

#include "debug_drawer.hpp"
#include "physics.hpp"

using namespace PhysicsSystem;

DebugDrawer::DebugDrawer(Ogre::SceneNode* node, btDynamicsWorld* world)
    : mp_node(node), mp_bullet_world(world), m_lines(""), m_debug_mode(DBG_DrawWireframe)
{
    m_lines.setCastShadows(false);
    mp_node->attachObject(&m_lines);
    mp_bullet_world->setDebugDrawer(this);
}

DebugDrawer::~DebugDrawer()
{
    clear();
    mp_node->detachObject(&m_lines);
    mp_bullet_world->setDebugDrawer(nullptr);
}

void DebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
    if (m_lines.getSections().empty())
    {
        const char* matName = "Ogre/Debug/LinesMat";
        auto mat = Ogre::MaterialManager::getSingleton().getByName(matName, Ogre::RGN_INTERNAL);
        if (!mat)
        {
            mat = Ogre::MaterialManager::getSingleton().create(matName, Ogre::RGN_INTERNAL);
            auto p = mat->getTechnique(0)->getPass(0);
            p->setLightingEnabled(false);
            p->setVertexColourTracking(Ogre::TVC_AMBIENT);
        }
        m_lines.setBufferUsage(Ogre::HBU_CPU_TO_GPU);
        m_lines.begin(mat, Ogre::RenderOperation::OT_LINE_LIST);
    }
    else if (m_lines.getCurrentVertexCount() == 0)
        m_lines.beginUpdate(0);

    Ogre::ColourValue col(color.x(), color.x(), color.z());
    m_lines.position(bulletVec2Ogre(from));
    m_lines.colour(col);
    m_lines.position(bulletVec2Ogre(to));
    m_lines.colour(col);
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
    drawLine(PointOnB, PointOnB + normalOnB * distance * 20, color);
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
    Ogre::LogManager::getSingleton().logWarning(warningString);
}

void DebugDrawer::draw3dText(const btVector3& location,const char* textString)
{
}

void DebugDrawer::setDebugMode(int mode)
{
    m_debug_mode = mode;

    if (m_debug_mode == DBG_NoDebug)
        clear();
}

void DebugDrawer::clear() { m_lines.clear(); }

void DebugDrawer::update()
{
    mp_bullet_world->debugDrawWorld();
    m_lines.end();
}
