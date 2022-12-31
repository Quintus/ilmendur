#ifndef ILMENDUR_GLOBALS_HPP
#define ILMENDUR_GLOBALS_HPP
#include <string>
#include <map>
/* This file is for widely used global declarations. Keep it small and
 * lightweight; it is meant to be included in several other headers. */

enum class direction { none, up, right, down, left };

// TODO: Replace this with a map+tuple construction
struct Properties {
    std::map<std::string,std::string> string_props;
    std::map<std::string,int> int_props;
    std::map<std::string,float> float_props;
    std::map<std::string,bool> bool_props;

    std::string get(const std::string& name) { return string_props[name]; };
    int getInt(const std::string& name) { return int_props[name]; };
    float getFloat(const std::string& name) { return float_props[name]; };
    bool getBool(const std::string& name) { return bool_props[name]; };
};

#endif /* ILMENDUR_GLOBALS_HPP */
