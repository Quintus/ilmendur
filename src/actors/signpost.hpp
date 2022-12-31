#ifndef ILMENDUR_SIGNPOST_HPP
#define ILMENDUR_SIGNPOST_HPP
#include "actor.hpp"
#include <vector>
#include <string>

class Signpost: public Actor
{
public:
    Signpost(int id, ObjectLayer* p_layer, SDL_Rect collbox, std::vector<std::string> texts);
    virtual ~Signpost();

    virtual void update();
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    virtual void handleEvent(const Event& event);
    virtual void interact(Actor* p_other);

    virtual SDL_Rect collisionBox() const;
private:
    std::vector<std::string> m_texts;
    SDL_Rect m_collbox;
};

#endif /* ILMENDUR_SIGNPOST_HPP */
