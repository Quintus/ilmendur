#include "map.hpp"
#include "os.hpp"
#include "actor.hpp"
#include "collbox.hpp"
#include "passage.hpp"
#include "ilmendur.hpp"
#include "texture_pool.hpp"
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <pugixml.hpp>

#include <iostream>

#define TILEWIDTH 32

using namespace std;
namespace fs = std::filesystem;

static TmxProperties readProperties(const pugi::xml_node& node)
{
    TmxProperties props;
    const pugi::xml_node& properties_node = node.child("properties");

    if (properties_node) {
        for (const pugi::xml_node& prop_node: properties_node.children("property")) {
            string prop_name = prop_node.attribute("name").value();
            string prop_val  = prop_node.attribute("value").value();
            string prop_type = prop_node.attribute("type").value();
            if (prop_type == "int") {
                props.int_props[prop_name] = atoi(prop_val.c_str());
            } else if (prop_type == "float") {
                props.float_props[prop_name] = atof(prop_val.c_str());
            } else if (prop_type == "bool") {
                props.int_props[prop_name] = prop_val == "true";
            } else { // Treat all the rest as strings, these types are not used
                props.string_props[prop_name] = prop_val;
            }
        }
    }

    return props;
}

static vector<int> parseGidCsv(const std::string& csv)
{
    vector<int> result;
    size_t pos = 0;
    size_t ppos = 0;
    while ((pos = csv.find(",", pos)) != string::npos) {
        int gid = atoi(csv.substr(ppos+1, pos-ppos).c_str());
        result.push_back(gid);
        // TODO: Detect rotations
        ppos = pos;
        pos++;
    }

    int gid = atoi(csv.substr(ppos+1, pos-ppos).c_str());
    result.push_back(gid);

    return result;
}

vector<Actor*> readTmxObjects(const pugi::xml_node& node)
{
    vector<Actor*> result;

    for(const pugi::xml_node& obj_node: node.children("object")) {
        int id              = obj_node.attribute("id").as_int();
        float x             = obj_node.attribute("x").as_float();
        float y             = obj_node.attribute("y").as_float();
        float w             = obj_node.attribute("width").as_float(); // zero if unset
        float h             = obj_node.attribute("height").as_float(); // zero if unset
        TmxProperties props = readProperties(obj_node);

        assert(id > 0);

        string type = props.get("type");
        if (type == string("static")) {
            const string& graphic = props.get("graphic");
            const string& ani     = props.get("animation_mode");
            assert(!graphic.empty());

            // Note: Actors should always be placed with Point objects in
            // Tiled, which do not have width/height values in the TMX file.
            Actor* p_actor = new Actor(id, graphic);
            p_actor->warp(Vector2f(x, y));

            if (!ani.empty()) {
                if (ani == string("never")) {
                    p_actor->setAnimationMode(Actor::animation_mode::never);
                } else if (ani == string("on_move")) {
                    p_actor->setAnimationMode(Actor::animation_mode::on_move);
                } else if (ani == string("always")) {
                    p_actor->setAnimationMode(Actor::animation_mode::always);
                } else {
                    throw(runtime_error("Invalid animation mode `" + ani + "' for object with ID " + to_string(id) + "!"));
                }
            }

            result.push_back(p_actor);
        } else if (type == string("npc")) {
            cout << "DEBUG WARNING: Ignoring npc actor for now" << endl;
        } else if (type == string("collbox")) {
            SDL_Rect rect;
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;
            result.push_back(new CollisionBox(id, rect));
        } else if (type == string("passage")) {
            SDL_Rect rect;
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;

            const string& direction = props.get("direction");
            Passage::pass_direction dir = 0;
            if (direction.find("all") != string::npos) {
                dir = Passage::up | Passage::right | Passage::down | Passage::left;
            } else {
                if (direction.find("up") != string::npos) {
                    dir |= Passage::up;
                }
                if (direction.find("right") != string::npos) {
                    dir |= Passage::right;
                }
                if (direction.find("down") != string::npos) {
                    dir |= Passage::down;
                }
                if (direction.find("left") != string::npos) {
                    dir |= Passage::left;
                }
            }

            const string& target = props.get("target");
            assert(!target.empty());
            result.push_back(new Passage(id, rect, dir, target));
        } else {
            // Valid TMX, but an error by the map editor: unknown object type requested.
            throw(runtime_error(string("Unknown object type `") + type + "' found in TMX file!"));
        }

    }

    sort(result.begin(),
         result.end(),
         [](Actor*& a, Actor*& b) { return a->id() < b->id(); });

    return result;
}

static Map::Layer readLayer(const pugi::xml_node& node, const std::string& mapname)
{
    Map::Layer layer;
    if (node.name() == string("layer")) {
        layer.type = Map::LayerType::Tile;
        layer.data.p_tile_layer = new TmxTileLayer();
        layer.data.p_tile_layer->name   = node.attribute("name").value();
        layer.data.p_tile_layer->width  = node.attribute("width").as_int();
        layer.data.p_tile_layer->height = node.attribute("height").as_int();
        layer.data.p_tile_layer->props  = readProperties(node);

        string facedir = layer.data.p_tile_layer->props.get("facedir");
        if (facedir == string("down")) {
            layer.data.p_tile_layer->dir = TmxTileLayer::direction::down;
        } else if (facedir == string("both")) {
            layer.data.p_tile_layer->dir = TmxTileLayer::direction::both;
        } else {
            layer.data.p_tile_layer->dir = TmxTileLayer::direction::up;
        }

        layer.data.p_tile_layer->gids = parseGidCsv(node.child("data").text().get());
    } else if (node.name() == string("objectgroup")) {
        layer.type = Map::LayerType::Object;
        layer.data.p_obj_layer         = new TmxObjLayer();
        layer.data.p_obj_layer->name   = node.attribute("name").value();
        layer.data.p_obj_layer->props  = readProperties(node);
        layer.data.p_obj_layer->actors = readTmxObjects(node);
    // TODO: Remaining TMX layer types
    } else {
        throw(runtime_error(string("Unsupported <map> child type `") + node.name() + "' in map `" + mapname + "'!"));
    }

    return layer;
}

Map::Map(const std::string& name)
    : m_name(name),
      m_width(0),
      m_height(0)
{
    fs::path abs_path(OS::gameDataDir() / fs::u8path("maps") / fs::u8path(m_name + ".tmx"));
    fstream file(abs_path);
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
        if (node.name() == string("tileset")) {
            int firstgid    = node.attribute("firstgid").as_int();
            fs::path source = fs::u8path(node.attribute("source").value()).filename(); // Discard directory information as it's irrelevant
            assert(firstgid > 0);

            m_tilesets[firstgid] = new Tileset(source);
        } else {
            m_layers.push_back(readLayer(node, m_name));
        }
    }
}

Map::~Map()
{
    for(Layer& layer: m_layers) {
        switch (layer.type) {
        case LayerType::Tile:
            delete layer.data.p_tile_layer;
            break;
        case LayerType::Object:
            for(Actor* p_actor: layer.data.p_obj_layer->actors) {
                delete p_actor;
            }
            delete layer.data.p_obj_layer;
            break;
        case LayerType::Image: // Ignore
        case LayerType::Group: // Ignore
        default: // Ignore
            break;
        }
    }

    for(auto iter = m_tilesets.begin(); iter != m_tilesets.end(); iter++) {
        delete iter->second;
    }
}

void Map::draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview)
{
    SDL_Rect srcrect;
    SDL_Rect destrect;
    SDL_Texture* p_tilesettexture = nullptr;

    // TODO: Honor p_camview width and height for scaling purposes.
    destrect.w = TILEWIDTH;
    destrect.h = TILEWIDTH;

    for(size_t li=0; li < m_layers.size(); li++) {
        const Layer& layer = m_layers[li];
        switch (layer.type) {
        case LayerType::Tile:
            if (layer.data.p_tile_layer->dir == TmxTileLayer::direction::up) {
                for(size_t i=0; i < layer.data.p_tile_layer->gids.size(); i++) {
                    int gid = layer.data.p_tile_layer->gids[i];
                    if (readTile(p_tilesettexture, srcrect, gid)) {
                        destrect.x = (i % m_width * TILEWIDTH) - p_camview->x;
                        destrect.y = (i / m_width * TILEWIDTH) - p_camview->y;
                        SDL_RenderCopy(p_stage, p_tilesettexture, &srcrect, &destrect);
                    }
                }
            }
            // TODO: Handle "down" and "both" direction depending on the player view direction.
            break;
        case LayerType::Object:
            for(Actor* p_actor: layer.data.p_obj_layer->actors) {
                p_actor->draw(p_stage, p_camview);
            }
            break;
        default:
            // Ignore
            break;
        }
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

bool Map::readTile(SDL_Texture*& p_texid, SDL_Rect& rect, int gid)
{
    static map<int,Tileset*>::iterator iter;
    if (gid == 0) {
        return false;
    }

    for(iter=m_tilesets.begin(); iter != m_tilesets.end(); iter++) {
        if (gid >= iter->first) {
            ++iter;
            if (iter == m_tilesets.end() || gid < iter->first) {
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
    for(Layer& layer: m_layers) {
        if (layer.type == LayerType::Object) {
            for(Actor* p_actor: layer.data.p_obj_layer->actors) {
                p_actor->update();
            }
        }
    }

    // After everyone has moved, check collisions and reset positions
    // appropriately.
    checkCollisions();
}

/**
 * Checks for collisions, rectifying impossible result positions.
 * Note that apart from collision with the map boundary, collisions
 * can only occur between actors on the same layer.
 */
void Map::checkCollisions()
{
    for(Layer& layer: m_layers) {
        if (layer.type == LayerType::Object) {
            for(Actor* p_actor: layer.data.p_obj_layer->actors) {
                checkCollideMapBoundary(p_actor);
                checkCollideActors(p_actor, *layer.data.p_obj_layer);
            }
        }
    }
}

void Map::checkCollideMapBoundary(Actor* p_actor)
{
    /* Do not walk off the map. It is allowed to have the drawing rectangle
     * hang into the void, but not the collision box, i.e., the map's edge
     * counts as a wall. */
    SDL_Rect maprect = drawRect();
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
 * layer can collide.
 */
void Map::checkCollideActors(Actor* p_actor, TmxObjLayer& layer)
{
    using Collision = pair<Actor*,Actor*>;

    // First, collect all intersecting actors.
    vector<Collision> collisions;
    SDL_Rect collrect = p_actor->collisionBox();
    SDL_Rect other_collbox;
    for(Actor* p_other: layer.actors) {
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

        if (coll.first->isMoving() && !coll.second->isMoving()) {
            actorAntiCollide(*coll.first, collrect1, collrect2, intersect);
        } else if (!coll.first->isMoving() && coll.second->isMoving()) {
            actorAntiCollide(*coll.second, collrect2, collrect1, intersect);
        } else if (coll.first->isMoving() && coll.second->isMoving()) {
            // If both are moving, move them both apart by half.
            intersect.w /= 2.0f;
            intersect.h /= 2.0f;
            actorAntiCollide(*coll.first, collrect1, collrect2, intersect);
            actorAntiCollide(*coll.second, collrect2, collrect1, intersect);
        } else {
            // If none is moving, move the smaller one out of the larger one.
            if (collrect1.w * collrect1.h < collrect2.w * collrect2.h) {
                actorAntiCollide(*coll.first, collrect1, collrect2, intersect);
            } else {
                actorAntiCollide(*coll.second, collrect2, collrect1, intersect);
            }
        }

        // TODO: Move this into event handler code
        Passage* p_passage = nullptr;
        Actor* p_other = nullptr;
        if ((p_passage = dynamic_cast<Passage*>(coll.first))) {
            p_other = coll.second;
        } else {
            p_passage = dynamic_cast<Passage*>(coll.second);
            p_other = coll.first;
        }
        if (p_passage) {
            // Find the layer the moving object is on, remove it from there,
            // and move it to the target layer.
            TmxObjLayer* p_layer = nullptr;
            assert(findActor(p_other->m_id, nullptr, &p_layer));
            assert(p_layer);
            for(auto iter=p_layer->actors.begin(); iter != p_layer->actors.end(); iter++) {
                if ((*iter)->m_id == p_other->m_id) {
                    p_layer->actors.erase(iter);
                    break;
                }
            }

            for(auto iter=m_layers.begin(); iter != m_layers.end(); iter++) {
                if (iter->type == LayerType::Object && iter->data.p_obj_layer->name == p_passage->m_targetlayer) {
                    iter->data.p_obj_layer->actors.push_back(p_other);

                    sort(iter->data.p_obj_layer->actors.begin(),
                         iter->data.p_obj_layer->actors.end(),
                         [](Actor* p_a, Actor* p_b){return p_a->m_id < p_b->m_id;});
                    break;
                }
            }
        }

        // TODO: Fire some event. TODO2: On which of the two?
    }
}

void Map::actorAntiCollide(Actor& actor, SDL_Rect& collrect1, SDL_Rect& collrect2, SDL_Rect& intersect)
{
    // First, the easy cases: four cardinal directions.
    if (actor.m_movedir.x == 0.0f && actor.m_movedir.y < 0.0f) { // North
        actor.stopMoving();
        actor.m_pos.y += intersect.h;
    } else if (actor.m_movedir.x > 0.0f && actor.m_movedir.y == 0.0f) { // East
        actor.stopMoving();
        actor.m_pos.x -= intersect.w;
    } else if (actor.m_movedir.x == 0.0f && actor.m_movedir.y > 0.0f) { // South
        actor.stopMoving();
        actor.m_pos.y -= intersect.h;
    } else if (actor.m_movedir.x < 0.0f && actor.m_movedir.y == 0.0f) { // West
        actor.stopMoving();
        actor.m_pos.x += intersect.w;
    } else { // Something in between. Complicated. (Note: both actors might not be moving at all!)
        /* The below seems to work fairly well. A clean solution would
         * probably generalise into a proper vector-movement based
         * approach. */
        if (intersect.h > intersect.w) {
            if (collrect1.x <= collrect2.x) {
                actor.m_pos.x -= intersect.w;
            } else {
                actor.m_pos.x += intersect.w;
            }
        } else {
            if (collrect1.y <= collrect2.y) {
                actor.m_pos.y -= intersect.h;
            } else {
                actor.m_pos.y += intersect.h;
            }
        }
        actor.stopMoving();

        // My prior attempt to do this properly follows:
        ///* Der kollidierende Aktor wird zunächst horizontal verschoben, bis
        // * er aus dem kollidierten Aktor entfernt wurde. Anschließend
        // * wird der Sinussatz angewandt, um die noch fehlende korrekte
        // * Y-Position des kollidierenden Aktors zu berechnen. Dabei darf
        // * die Einbeziehung des zweiten Winkels wegfallen, da er 90°
        // * beträgt und sin(90°)=1 gilt. Bewegt der Aktor sich auf der X-Achse,
        // * kann die Berechnung ganz entfallen. */
        //float angle = M_PI - actor.m_movedir.angleWith(Vector2f(0,1));
        //actor.stopMoving();
        //if (collrect1.x <= collrect2.x) { // Kollidierer kommt von links
        //    actor.m_pos.x -= intersect.w;
        //} else { // Kollidierer kommt von rechts
        //    actor.m_pos.x += intersect.w;
        //}
        //if (!float_equal(0.5*M_PI - angle, 0.0f)) { // 0.5π rad = 90°
        //    actor.m_pos.y += intersect.w / sinf(0.5*M_PI - angle);
        //}
        //
        //cout << "This gives an angle of " << (180.0f*angle)/M_PI << "° with the Y axis" << endl;
        //cout << "ID " << actor.m_id << " is now at (" << actor.m_pos.x << "|" << actor.m_pos.y << ")" << endl;        }
    }
}

/**
 * Add an actor to the map, to the layer named by `layername'. The map
 * takes ownership of `p_actor', do not delete it anymore.
 */
void Map::addActor(Actor* p_actor, const string& layername)
{
    for(Layer& layer: m_layers) {
        if (layer.type == LayerType::Object && layer.data.p_obj_layer->name == layername) {
            // Insert the actor at the position corresponding to its ID.
            for (auto iter = layer.data.p_obj_layer->actors.begin(); iter != layer.data.p_obj_layer->actors.end(); iter++) {
                if ((*iter)->id() > p_actor->id()) {
                    layer.data.p_obj_layer->actors.insert(iter, p_actor);
                    return;
                }
            }
            // If this is reached, the actor has a higher ID than all existing actors.
            layer.data.p_obj_layer->actors.push_back(p_actor);
            return;
        }
    }

    // If this triggers, a non-existant layer was requested. Note that
    // actors can only be added to object layers.
    assert(false);
}

bool Map::findActor(int id, Actor** pp_actor, TmxObjLayer** pp_layer)
{
    assert(id > 0);

    for (Layer& layer: m_layers) {
        if (layer.type == LayerType::Object) {
            for (auto iter=layer.data.p_obj_layer->actors.begin();
                 iter != layer.data.p_obj_layer->actors.end();
                 iter++) {
                if (id == (*iter)->m_id) {
                    if (pp_actor) {
                        *pp_actor = *iter;
                    }
                    if (pp_layer) {
                        *pp_layer = layer.data.p_obj_layer;
                    }
                    return true;
                }
            }
        }
    }

    return false;
}
