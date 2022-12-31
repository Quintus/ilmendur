#ifndef ILMENDUR_PASSAGE_HPP
#define ILMENDUR_PASSAGE_HPP
#include "actor.hpp"

class Passage: public Actor
{
public:
    typedef unsigned short pass_direction;
    static const pass_direction up    = 1 << 0;
    static const pass_direction right = 1 << 1;
    static const pass_direction down  = 1 << 2;
    static const pass_direction left  = 1 << 3;

    Passage(int id, ObjectLayer* p_layer, SDL_Rect area, pass_direction dir, const std::string& targetlayer);
    virtual ~Passage();

    virtual void handleEvent(const Event& event);

    virtual SDL_Rect collisionBox() const;
private:
    Vector2f m_size;
    pass_direction m_passdir;
    std::string m_targetlayer;

    friend class Map;
};

#endif /* ILMENDUR_PASSAGE_HPP */
