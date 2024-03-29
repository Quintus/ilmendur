#ifndef ILMENDUR_COLLBOX_HPP
#define ILMENDUR_COLLBOX_HPP
#include "actor.hpp"

class CollisionBox: public Actor
{
public:
    CollisionBox(int id, ObjectLayer* p_layer, SDL_Rect box);
    virtual ~CollisionBox();

    virtual void update();
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    virtual void handleEvent(const Event& event);

    virtual SDL_Rect collisionBox() const;
private:
    SDL_Rect m_collbox;
};

#endif /* ILMENDUR_COLLBOX_HPP */
