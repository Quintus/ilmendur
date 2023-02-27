#ifndef ILMENDUR_TELEPORT_HPP
#define ILMENDUR_TELEPORT_HPP
#include "actor.hpp"

class Entry: public Actor
{
public:
    Entry(int id, ObjectLayer* p_layer, direction enter_dir);
    virtual ~Entry();

    direction enterDirection() const;

private:
    direction m_enter_dir;
};

class Teleport: public Actor
{
public:
    Teleport(int id, ObjectLayer* p_layer, SDL_Rect box, int target_entry_id, std::string target_map_name);
    virtual ~Teleport();

    virtual void update();
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    virtual void handleEvent(const Event& event);

    virtual SDL_Rect collisionBox() const;
private:
    SDL_Rect m_collbox;
    int m_target_entry_id;
    std::string m_target_map_name;
};

#endif /* ILMENDUR_TELEPORT_HPP */
