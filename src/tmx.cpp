#include "tmx.hpp"
#include "actors/actor.hpp"
#include "actors/collbox.hpp"
#include "actors/passage.hpp"
#include "actors/startpos.hpp"
#include "actors/player.hpp"
#include "actors/signpost.hpp"
#include "map.hpp"
#include "i18n.hpp"
#include <cassert>
#include <algorithm>
#include <pugixml.hpp>

#include <iostream>

using namespace std;
using namespace TMX;

Properties TMX::readProperties(const pugi::xml_node& node)
{
    Properties props;
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

vector<int> TMX::parseGidCsv(const std::string& csv)
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

/**
 * This is the main function responsible for deserialising actors from
 * the map TMX file. It constructs all actors and adds them into
 * `target_layer`.
 */
void TMX::readTmxObjects(const pugi::xml_node& node, ObjectLayer* p_target_layer)
{
    vector<Actor*> result;

    for(const pugi::xml_node& obj_node: node.children("object")) {
        int id              = obj_node.attribute("id").as_int();
        float x             = obj_node.attribute("x").as_float();
        float y             = obj_node.attribute("y").as_float();
        float w             = obj_node.attribute("width").as_float(); // zero if unset
        float h             = obj_node.attribute("height").as_float(); // zero if unset
        Properties props    = readProperties(obj_node);

        assert(id > 0);

        string type = props.get("type");
        if (type == string("static")) {
            const string& graphic = props.get("graphic");
            const string& ani     = props.get("animation_mode");
            assert(!graphic.empty());

            // Note: Actors should always be placed with Point objects in
            // Tiled, which do not have width/height values in the TMX file.
            Actor* p_actor = new Actor(id, p_target_layer, graphic);
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
        } else if (type == "startpos") {
            result.push_back(new StartPosition(id, p_target_layer, Vector2f(x, y), atoi(props.get("startpos").c_str())));
        } else if (type == string("npc")) {
            cout << "DEBUG WARNING: Ignoring npc actor for now" << endl;
        } else if (type == "signpost") {
            SDL_Rect rect;
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;

            string translated_text = gettext(props.get("text").c_str());
            vector<string> texts = splitString(translated_text, "<NM>");
            result.push_back(new Signpost(id, p_target_layer, rect, texts));
        } else if (type == string("collbox")) {
            SDL_Rect rect;
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;
            result.push_back(new CollisionBox(id, p_target_layer, rect));
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
            result.push_back(new Passage(id, p_target_layer, rect, dir, target));
        } else {
            // Valid TMX, but an error by the map editor: unknown object type requested.
            throw(runtime_error(string("Unknown object type `") + type + "' found in TMX file!"));
        }

    }

    // Ensure all actors are sorted by ID
    sort(result.begin(),
         result.end(),
         [](Actor*& a, Actor*& b) { return a->id() < b->id(); });

    // Important final step: Make the counter-link from the layer to
    // to the actors so that the layer owns the actors.
    for(Actor* p_actor: result) {
        tmxAddActor(p_target_layer, p_actor);
    }
}

/**
 * Internal function used by TMX::readTmxObjects() that is `friend`
 * with ObjectLayer.
 *
 * \internal
 */
void TMX::tmxAddActor(ObjectLayer* p_target_layer, Actor* p_actor)
{
    p_target_layer->addActor(p_actor);
}
