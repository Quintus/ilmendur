#include "signpost.hpp"
#include "../event.hpp"
#include "../ilmendur.hpp"
#include "../texture_pool.hpp"
#include "../gui.hpp"
#include "player.hpp"

#define TILEWIDTH 32

using namespace std;

Signpost::Signpost(int id, ObjectLayer* p_layer, vector<string> texts)
    : Actor(id, p_layer),
      m_texts(texts)
{
}

Signpost::~Signpost()
{
}

void Signpost::update()
{
    // The signpost does nothing on update.
}

/**
 * Draw the sign onto the stage. This does not use Actor's normal
 * draw() implementation, because it renders a part of the
 * “signposts” tileset rather than a separate character graphic.
 */
void Signpost::draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview)
{
    TextureInfo* p_tileset = Ilmendur::instance().texturePool()["tilesets/signposts.png"];
    static const SDL_Rect srcrect { 32, 0, 32, 32 };
    SDL_Rect destrect;
    destrect.x = m_pos.x - 0.5 * TILEWIDTH;
    destrect.y = m_pos.y - 0.5 * TILEWIDTH;
    destrect.w = 32;
    destrect.h = 32;

    if (!SDL_HasIntersection(&destrect, p_camview)) {
        return;
    }

    // TODO: Honor p_camview width and height for scaling purposes.
    destrect.x -= p_camview->x;
    destrect.y -= p_camview->y;

    SDL_RenderCopy(p_stage, p_tileset->p_texture, &srcrect, &destrect);
}

SDL_Rect Signpost::collisionBox() const
{
    SDL_Rect rect;
    rect.x = m_pos.x - 0.5 * TILEWIDTH;
    rect.y = m_pos.y - 0.5 * TILEWIDTH;
    rect.w = TILEWIDTH;
    rect.h = TILEWIDTH;
    return rect;
}

void Signpost::handleEvent(const Event& event)
{
    // Signposts may not be walked through.
    if (event.type == Event::Type::collision) {
        antiCollide(event.data.coll.p_other, this, event.data.coll.intersect);
    }
}

void Signpost::interact(Actor* p_other)
{
    Player* p_player = dynamic_cast<Player*>(p_other);
    if (!p_player) {
        return;
    }

    // TODO: Check which one of the heroes it is. For now assume player 1
    GUISystem::messageDialog(1, GUISystem::text_velocity::instant, "Signpost", m_texts);
}
