#ifndef ILMENDUR_TMX_HPP
#define ILMENDUR_TMX_HPP
#include "util.hpp"
#include "globals.hpp"
#include <string>
#include <vector>
#include <pugixml.hpp>
// Various utilities related to specifically dealing with Tiled's TMX map format

class Actor;
class ObjectLayer;

namespace TMX {

    std::vector<int> parseGidCsv(const std::string& csv);
    Properties readProperties(const pugi::xml_node& node);
    void readTmxObjects(const pugi::xml_node& node, ObjectLayer* p_target_layer);
}

#endif /* ILMENDUR_TMX_HPP */
