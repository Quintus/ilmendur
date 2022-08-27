#include "map.hpp"
#include "os.hpp"
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <pugixml.hpp>

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

static vector<TmxObject> readTmxObjects(const pugi::xml_node& node)
{
    vector<TmxObject> result;

    for(const pugi::xml_node& obj_node: node.children("object")) {
        TmxObject obj;
        obj.id    = obj_node.attribute("id").as_int();
        obj.x     = obj_node.attribute("x").as_float();
        obj.y     = obj_node.attribute("y").as_float();
        obj.props = readProperties(obj_node);

        // TMX file is invalid TMX if one of these asserts triggers.
        assert(obj.id > 0);
        assert(obj.x >= 0.0f);
        assert(obj.y >= 0.0f);

        string type = obj.props.get("type");
        if (type == string("static")) {
            obj.type = TmxObject::Type::static_object;
        } else if (type == string("npc")) {
            obj.type = TmxObject::Type::npc;
        } else if (type == string("collbox")) {
            obj.type = TmxObject::Type::collision_box;
        } else {
            // Valid TMX, but an error by the map editor: unknown object type requested.
            throw(runtime_error(string("Unknown object type `") + type + "' found in TMX file!"));
        }

    }

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
        layer.data.p_obj_layer          = new TmxObjLayer();
        layer.data.p_obj_layer->props   = readProperties(node);
        layer.data.p_obj_layer->objects = readTmxObjects(node);
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
