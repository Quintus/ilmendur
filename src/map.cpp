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

    for (const pugi::xml_node& prop_node: node.children("properties")) {
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

    return props;
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
    } else if (node.name() == string("objectgroup")) {
        layer.type = Map::LayerType::Object;
        layer.data.p_obj_layer = new TmxObjLayer();
        // TODO
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

            m_tilesets.emplace(make_pair(firstgid, Tileset(source)));
        } else {
            m_layers.push_back(readLayer(node, m_name));
        }
    }
}

void Map::draw()
{
    // TODO
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
}
