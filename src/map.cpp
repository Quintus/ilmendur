#include "map.hpp"
#include "os.hpp"
#include "event.hpp"
#include "ilmendur.hpp"
#include "texture_pool.hpp"
#include "tmx.hpp"
#include "actors/actor.hpp"
#include "actors/startpos.hpp"
#include "actors/player.hpp"
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <pugixml.hpp>

#define TILEWIDTH 32

using namespace std;
namespace fs = std::filesystem;

MapLayer::MapLayer(Map& map, std::string name, Properties props)
    : mr_map(map),
      m_name(name),
      m_props(props)
{
}

MapLayer::~MapLayer()
{
}

ObjectLayer::ObjectLayer(Map& map, std::string name, Properties props)
    : MapLayer(map, name, props)
{
}

ObjectLayer::~ObjectLayer()
{
    for(Actor* p_actor: m_actors) {
        delete p_actor;
    }

    m_actors.clear();
}

void ObjectLayer::update()
{
    for(Actor* p_actor: m_actors) {
        p_actor->update();
    }

    // After everyone has moved, check collisions and reset positions
    // appropriately.
    checkCollisions();
}

void ObjectLayer::draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview)
{
    for(Actor* p_actor: m_actors) {
        p_actor->draw(p_stage, p_camview);
    }
}

TileLayer::TileLayer(Map& map, std::string name, Properties props, int width, int height, vector<int> gids)
    : MapLayer(map, name, props),
      m_width(width),
      m_height(height),
      m_gids(gids)
{
    string facedir = m_props.get("facedir");
    if (facedir == string("down")) {
        m_dir = TileLayer::layer_direction::down;
    } else if (facedir == string("both")) {
        m_dir = TileLayer::layer_direction::both;
    } else {
        m_dir = TileLayer::layer_direction::up;
    }
}

void TileLayer::update()
{
    // Nothing
}

void TileLayer::draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview)
{
    SDL_Rect srcrect;
    SDL_Rect destrect;
    SDL_Texture* p_tilesettexture = nullptr;

    // TODO: Honor p_camview width and height for scaling purposes.
    destrect.w = TILEWIDTH;
    destrect.h = TILEWIDTH;

    if (m_dir == TileLayer::layer_direction::up) {
        for(size_t i=0; i < m_gids.size(); i++) {
            int gid = m_gids[i];
            if (readTile(p_tilesettexture, srcrect, gid)) {
                destrect.x = (i % m_width * TILEWIDTH) - p_camview->x;
                destrect.y = (i / m_width * TILEWIDTH) - p_camview->y;
                SDL_RenderCopy(p_stage, p_tilesettexture, &srcrect, &destrect);
            }
        }
    }
    // TODO: Handle "down" and "both" direction depending on the player view direction.

}

static MapLayer* readLayer(const pugi::xml_node& node, Map& map)
{
    if (node.name() == string("layer")) {
        TileLayer* p_layer = new TileLayer(map,
                                           node.attribute("name").value(),
                                           TMX::readProperties(node),
                                           node.attribute("width").as_int(),
                                           node.attribute("height").as_int(),
                                           TMX::parseGidCsv(node.child("data").text().get()));
        return p_layer;
    } else if (node.name() == string("objectgroup")) {
        ObjectLayer* p_layer = new ObjectLayer(map,
                                               node.attribute("name").value(),
                                               TMX::readProperties(node));
        TMX::readTmxObjects(node, p_layer); // Modifies `p_layer`!
        return p_layer;
    // TODO: Remaining TMX layer types
    } else {
        throw(runtime_error(string("Unsupported <map> child type `") + node.name() + "' in map `" + map.name() + "'!"));
    }
}

Map::Map(const std::string& name)
    : m_name(name),
      m_width(0),
      m_height(0),
      mp_freya(nullptr),
      mp_benjamin(nullptr)
{
    // DEBUG: Try user-provided map of the name first, and only if it
    // does not exist try shipped map. This is only for debugging!
    fs::path abs_path(OS::userDataDir() / fs::u8path("maps") / fs::u8path(m_name + ".tmx"));
    if (!fs::exists(abs_path)) {
        abs_path = OS::gameDataDir() / fs::u8path("maps") / fs::u8path(m_name + ".tmx");
    }
    ifstream file(abs_path);
    assert(fs::exists(abs_path));

    pugi::xml_document doc;
    if (!doc.load(file)) {
        throw(std::runtime_error(string("Failed to load map '") + m_name + "'"));
    }

    if (doc.child("map").attribute("version").value() != string("1.5")) {
        throw(std::runtime_error(string("Expected TMX map format version 1.5, got '") + doc.child("map").attribute("version").value() + "'."));
    }
    if (doc.child("map").attribute("tilewidth").as_int() != TILEWIDTH) {
        throw(std::runtime_error(string("Map '" + m_name + "' does not have " + to_string(TILEWIDTH) + "px tile width")));
    }
    if (doc.child("map").attribute("tileheight").as_int() != TILEWIDTH) {
        throw(std::runtime_error(string("Map '" + m_name + "' does not have " + to_string(TILEWIDTH) + "px tile height")));
    }

    if (doc.child("map").attribute("version").value() != string("1.5")) {
        throw(std::runtime_error(string("Expected TMX map format version 1.5, got '") + doc.child("map").attribute("version").value() + "'."));
    }

    // TODO: More assertions on <MAP> attributes

    m_width  = doc.child("map").attribute("width").as_int();
    m_height = doc.child("map").attribute("height").as_int();
    assert(m_width > 0 && m_height > 0);

    for (const pugi::xml_node& node: doc.child("map").children()) {
        if (node.name() == string("properties")) {
            Properties props = TMX::readProperties(doc.child("map"));
            m_bg_music = props.get("background_music");
        } else if (node.name() == string("tileset")) {
            int firstgid    = node.attribute("firstgid").as_int();
            fs::path source = fs::u8path(node.attribute("source").value()).filename(); // Discard directory information as it's irrelevant
            assert(firstgid > 0);

            m_tilesets[firstgid] = new Tileset(source);
        } else {
            m_layers.push_back(readLayer(node, *this));
        }
    }
}

Map::~Map()
{
    for (MapLayer* p_layer: m_layers) {
        delete p_layer;
    }
    m_layers.clear();

    for(auto iter = m_tilesets.begin(); iter != m_tilesets.end(); iter++) {
        delete iter->second;
    }

    m_tilesets.clear();
}

void Map::draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview)
{
    for (MapLayer* p_layer: m_layers) {
        p_layer->draw(p_stage, p_camview);
    }
}

/**
 * Returns the drawing rectangle for the entire map in world coordinates.
 * The X/Y position for a map is always zero.
 */
SDL_Rect Map::drawRect() const
{
    return SDL_Rect{0, 0, m_width * TILEWIDTH, m_height * TILEWIDTH};
}

bool TileLayer::readTile(SDL_Texture*& p_texid, SDL_Rect& rect, int gid)
{
    static std::map<int,Tileset*>::iterator iter;
    if (gid == 0) {
        return false;
    }

    for(iter=mr_map.tilesets().begin(); iter != mr_map.tilesets().end(); iter++) {
        if (gid >= iter->first) {
            ++iter;
            if (iter == mr_map.tilesets().end() || gid < iter->first) {
                --iter;
                p_texid = iter->second->sdlTexture();
                iter->second->readTile(rect, gid - iter->first);
                return true;
            } else {
                --iter;
            }
        }
    }

    assert(false);
    return false;
}

void Map::update()
{
    /* Note: This is not the place to optimise by trying to only update
     * actors within the camera range. That would cause rather unnatural
     * behaviour. Always update all actors on the stage, and optimise by
     * not drawing them all in draw(). */
    for(MapLayer* p_layer: m_layers) {
        p_layer->update();
    }
}

/**
 * Checks for collisions, rectifying impossible result positions.
 * Note that apart from collision with the map boundary, collisions
 * can only occur between actors on the same layer.
 */
void ObjectLayer::checkCollisions()
{
    for(Actor* p_actor: m_actors) {
        checkCollideMapBoundary(p_actor);
        checkCollideActors(p_actor);
    }
}

void ObjectLayer::checkCollideMapBoundary(Actor* p_actor)
{
    /* Do not walk off the map. It is allowed to have the drawing rectangle
     * hang into the void, but not the collision box, i.e., the map's edge
     * counts as a wall. */
    SDL_Rect maprect = mr_map.drawRect();
    if (p_actor->isInvisible()) {
        if (p_actor->m_pos.x < 0.0f) {
            p_actor->m_pos.x = 0.0f;
        }
        if (p_actor->m_pos.x >= maprect.w) {
            p_actor->m_pos.x = maprect.w - 1.0f;
        }
        if (p_actor->m_pos.y < 0.0f) {
            p_actor->m_pos.y = 0.0f;
        }
        if (p_actor->m_pos.y >= maprect.h) {
            p_actor->m_pos.y = maprect.h - 1.0f;
        }
    } else {
        SDL_Rect collrect = p_actor->collisionBox();
        if (collrect.x < maprect.x) {
            p_actor->m_pos.x = maprect.x + p_actor->mp_texinfo->origx - p_actor->mp_texinfo->collx;
            p_actor->stopMoving();
        }
        if (collrect.x + collrect.w > maprect.x + maprect.w) {
            p_actor->m_pos.x = maprect.x + maprect.w - p_actor->mp_texinfo->origx - p_actor->mp_texinfo->collx;
            p_actor->stopMoving();
        }
        if (collrect.y < maprect.y) {
            p_actor->m_pos.y = maprect.y + p_actor->mp_texinfo->origy - p_actor->mp_texinfo->colly;
            p_actor->stopMoving();
        }
        if (collrect.y + collrect.h > maprect.y + maprect.h) {
            p_actor->m_pos.y = maprect.y + maprect.h - p_actor->mp_texinfo->origy - p_actor->mp_texinfo->colly;
            p_actor->stopMoving();
        }
    }

    // TODO: Fire event on p_actor
}

/**
 * Checks collisions with other actors. Only actors on the same
 * layer can collide, hence it is possible to have this as a
 * member function of ObjectLayer rather than Map.
 */
void ObjectLayer::checkCollideActors(Actor* p_actor)
{
    using Collision = pair<Actor*,Actor*>;

    // First, collect all intersecting actors.
    vector<Collision> collisions;
    SDL_Rect collrect = p_actor->collisionBox();
    SDL_Rect other_collbox;
    for(Actor* p_other: m_actors) {
        // Collision of an actor with itself is not possible.
        if (p_actor->m_id == p_other->m_id) {
            continue;
        }

        other_collbox = p_other->collisionBox();
        if (SDL_HasIntersection(&collrect, &other_collbox)) {
            // Ensure the actor with the smaller ID always comes first.
            if (p_actor->m_id < p_other->m_id) {
                collisions.push_back(make_pair(p_actor, p_other));
            } else {
                collisions.push_back(make_pair(p_other, p_actor));
            }
        }
    }

    /* The above code will yield duplicates: each actor colliding with
     * another actor will be included twice. The above code ensures
     * that the pairs are equal, as the actor with the lower ID always
     * comes first. */
    unique(collisions.begin(),
           collisions.end(),
           [](Collision& coll1, Collision& coll2) { return coll1.first->m_id == coll2.first->m_id && coll1.second->m_id == coll2.second->m_id; });

    // Now execute all the collisions.
    SDL_Rect collrect1;
    SDL_Rect collrect2;
    SDL_Rect intersect;
    for(Collision& coll: collisions) {
        collrect1 = coll.first->collisionBox();
        collrect2 = coll.second->collisionBox();

        /* If this condition evaluates to true, then the actors have already
         * been de-collided by a prior collision execution in this frame,
         * i.e. an actor collided with two or more actors in a single frame.
         * In that case, skip the collision. */
        if (SDL_IntersectRect(&collrect1, &collrect2, &intersect) != SDL_TRUE) {
            continue;
        }

        Event collev;
        collev.type = Event::Type::collision;
        collev.data.coll.intersect = intersect;
        collev.data.coll.p_other = coll.second;
        coll.first->handleEvent(collev);

        /* It may happen that the collision handler of `coll.first'
         * displaces the actors so that no intersection remains. Skip
         * the second event in that case. */
        if (SDL_IntersectRect(&collrect1, &collrect2, &intersect) == SDL_TRUE) {
            collev.data.coll.intersect = intersect;
            collev.data.coll.p_other = coll.first;
            coll.second->handleEvent(collev);
        }
    }
}

/**
 * Release ownership of `p_actor` from this layer, leaving the Actor
 * instance dangling. This function is only intended to be used
 * in Map::changeActorLayer() and nowhere else! It is an implementation
 * detail! `p_actor` is in an inconsistent state after this method
 * returns!
 *
 * \internal
 */
void ObjectLayer::releaseActor(Actor* p_actor)
{
    for(auto iter=m_actors.begin(); iter != m_actors.end(); iter++) {
        if ((*iter)->m_id == p_actor->m_id) {
            m_actors.erase(iter);
            return;
        }
    }

    /* If this assert triggers, then the actor requested to be released
     * was not on the layer. This indicates that p_actor's mr_layer
     * got out of sync with it's layer's member information -- which
     * in turn is most likely the result of not using Map::changeActorLayer()
     * for changing an actor's layer. ONLY EVER USE changeActorLayer() to
     * change an actor's layer! */
    assert(false);
}

/**
 * Adds `p_actor` to the actors owned by this layer. This is
 * an internal API only meant to be used during actor construction.
 * It leaves `p_actor` itself untouched!
 *
 * \internal
 */
void ObjectLayer::addActor(Actor* p_actor)
{
    assert(p_actor->mapLayer() == this);

    // Insert the actor at the position corresponding to its ID.
    for(auto iter=m_actors.begin(); iter != m_actors.end(); iter++) {
        if ((*iter)->id() > p_actor->id()) {
            m_actors.insert(iter, p_actor);
            return;
        }
    }

    // If this is reached, the actor has a higher ID than all existing actors.
    m_actors.push_back(p_actor);
}

vector<Actor*> Map::findAdjascentActors(Actor* p_actor, direction dir)
{
    vector<Actor*> results;
    ObjectLayer* p_layer = p_actor->mapLayer(); // Only actors on the same layer are considered
    SDL_Rect collbox1 = p_actor->collisionBox();
    SDL_Rect collbox2;
    SDL_Rect intersect;
    for (Actor* p_other: p_layer->actors()) {
        // p_actor may not collide with itself
        if (p_other->id() == p_actor->id()) {
            continue;
        }

        collbox2 = p_other->collisionBox();
        if (SDL_IntersectRect(&collbox1, &collbox2, &intersect) == SDL_TRUE) {
            results.push_back(p_other);
        } else {
            switch (dir) {
            case direction::none: // Currently not supported; maybe treat as any direction?
                assert(false);
                break;
            case direction::up:
                if ((collbox1.y == collbox2.y + collbox2.h) &&
                    hasOverlap(collbox1.x, collbox1.x + collbox1.w, collbox2.x, collbox2.x + collbox2.w)) {
                    results.push_back(p_other);
                }
                break;
            case direction::right:
                if ((collbox1.x + collbox1.w == collbox2.x) &&
                    hasOverlap(collbox1.y, collbox1.y + collbox1.h, collbox2.y, collbox2.y + collbox2.h)) {
                    results.push_back(p_other);
                }
                break;
            case direction::down:
                if ((collbox1.y + collbox1.h == collbox2.y) &&
                    hasOverlap(collbox1.x, collbox1.x + collbox1.w, collbox2.x, collbox2.x + collbox2.w)) {
                    results.push_back(p_other);
                }
                break;
            case direction::left:
                if ((collbox1.x == collbox2.x + collbox2.w) &&
                    hasOverlap(collbox1.y, collbox1.y + collbox1.h, collbox2.y, collbox2.y + collbox2.h)) {
                    results.push_back(p_other);
                }
                break;
            } // No default so the compiler can warn about missing values
        }
    }
    return results;
}

/**
 * Retrieves the hero pointers, if there are any. Otherwise, this
 * method crashes.
 */
void Map::heroes(Player** p_freya, Player** p_benjamin)
{
    assert(mp_freya && mp_benjamin);
    *p_freya = mp_freya;
    *p_benjamin = mp_benjamin;
}

/**
 * Locates the actors of type `startpos` and creates player actors
 * centred on them. If there are no `startpos` type actors, places the
 * heroes at (100|100) and (200|200) at the bottom-most object layer.
 * If there are no object layers, this method crashes.
 */
void Map::makeHeroes()
{
    for(MapLayer* p_layer: m_layers) {
        ObjectLayer* p_obj_layer = dynamic_cast<ObjectLayer*>(p_layer);
        if (p_obj_layer) {
            for(Actor* p_actor: p_obj_layer->actors()) {
                StartPosition* p_startpos = dynamic_cast<StartPosition*>(p_actor);
                if (p_startpos) {
                    if (p_startpos->herono == 1) {
                        mp_freya = new Player(p_obj_layer, 1);
                        mp_freya->warp(p_startpos->startpos);
                        mp_freya->turn(direction::up);
                        p_obj_layer->addActor(mp_freya);
                    } else if (p_startpos->herono == 2) {
                        mp_benjamin = new Player(p_obj_layer, 2);
                        mp_benjamin->warp(p_startpos->startpos);
                        mp_benjamin->turn(direction::up);
                        p_obj_layer->addActor(mp_benjamin);
                    } else {
                        // This is a two-player game.
                        assert(false);
                    }
                }
            }
        }
    }

    // If at least one hero was placed, it is ok.
    if (mp_freya || mp_benjamin) {
        return;
    }

    // No start positions found, use defaults
    for(MapLayer* p_layer: m_layers) {
        ObjectLayer* p_obj_layer = dynamic_cast<ObjectLayer*>(p_layer);
        if (p_obj_layer) {
            mp_freya = new Player(p_obj_layer, 1);
            mp_freya->warp(Vector2f(100.0f, 100.0f));
            mp_freya->turn(direction::up);
            p_obj_layer->addActor(mp_freya);

            mp_benjamin = new Player(p_obj_layer, 2);
            mp_benjamin->warp(Vector2f(200.0f, 200.0f));
            mp_benjamin->turn(direction::up);
            p_obj_layer->addActor(mp_benjamin);
            return;
        }
    }

    // No object layers. Error.
    assert(false);
}

/**
 * Search through all object layers of this map for the actor
 * that has the given ID. This is an expensive operation;
 * use sparingly.
 *
 * Returns false if there is no actor with the requested ID, otherwise
 * returns true. In the letter case, `*pp_actor` is set to a pointer
 * to the Actor instance. In the former `*pp_actor` is left untouched.
 */
bool Map::findActor(int id, Actor** pp_actor)
{
    assert(id > 0);
    assert(pp_actor);

    for (MapLayer* p_layer: m_layers) {
        ObjectLayer* p_obj_layer = dynamic_cast<ObjectLayer*>(p_layer);
        if (p_obj_layer) {
            for (auto iter=p_obj_layer->actors().begin();
                 iter != p_obj_layer->actors().end();
                 iter++) {
                if (id == (*iter)->id()) {
                    *pp_actor = *iter;
                    return true;
                }
            }
        }
    }

    return false;
}

/**
 * Changes the layer `p_actor' is on to the layer with the given name.
 * The method will crash with an assertion failure if `p_actor' is not
 * on this map or the requested target layer does not exist.
 */
void Map::changeActorLayer(Actor* p_actor, const string& target_layer_name)
{
    assert(&p_actor->mapLayer()->map() == this);

    // Find the layer the actor is on, remove it from there,
    // and move it to the target layer.
    ObjectLayer* p_layer = p_actor->mapLayer();
    p_layer->releaseActor(p_actor); // p_actor's mp_layer is now out of sync!!

    for(auto iter=m_layers.begin(); iter != m_layers.end(); iter++) {
        ObjectLayer* p_obj_layer = dynamic_cast<ObjectLayer*>(*iter);
        if (p_obj_layer && p_obj_layer->name() == target_layer_name) {
            p_actor->resetLayer(p_obj_layer); // Now p_obj_layer's actor list is broken
            p_obj_layer->addActor(p_actor);   // Fix it
            return;
        }
    }

    assert(false); // Invalid target layer requested
}
