#ifndef ILMENDUR_MAP_HPP
#define ILMENDUR_MAP_HPP
#include "tileset.hpp"
#include "globals.hpp"
#include <vector>
#include <map>

// TODO: Move these somewhere else
struct TmxProperties {
    std::map<std::string,std::string> string_props;
    std::map<std::string,int> int_props;
    std::map<std::string,float> float_props;
    std::map<std::string,bool> bool_props;

    std::string get(const std::string& name) { return string_props[name]; };
    int getInt(const std::string& name) { return int_props[name]; };
    float getFloat(const std::string& name) { return float_props[name]; };
    bool getBool(const std::string& name) { return bool_props[name]; };
};

class Actor;
class Player;
struct TmxObjLayer
{
    std::string name;
    TmxProperties props;
    std::vector<Actor*> actors;
};

struct TmxTileLayer {
    std::string name;
    int width;
    int height;
    TmxProperties props;
    std::vector<int> gids;

    enum class layer_direction { up, down, both };
    layer_direction dir;
};

class Map
{
public:
    Map(const std::string& name);
    ~Map();

    void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    void update();
    SDL_Rect drawRect() const;

    void addHero(Player* p_player, int herono);
    void addActor(Actor* p_actor, const std::string& layername);
    bool findActor(int id, Actor** pp_actor, TmxObjLayer** pp_layer);
    void changeActorLayer(Actor* p_actor, const std::string& target_layer_name);
    std::vector<Actor*> findAdjascentActors(Actor* p_actor, direction dir);

    inline const std::string& backgroundMusic() const { return m_bg_music; }

    // Helper types for dealing with Tiled layers. Actually, only
    // Tile and Object are supported by the Layer struct.
    enum class LayerType { Tile, Object, Image, Group };
    struct Layer {
        LayerType type;
        union {
            TmxTileLayer* p_tile_layer;
            TmxObjLayer* p_obj_layer;
        } data;
    };

private:
    bool readTile(SDL_Texture*& p_texid, SDL_Rect& rect, int gid);
    void checkCollisions();
    void checkCollideMapBoundary(Actor* p_actor);
    void checkCollideActors(Actor* p_actor, TmxObjLayer& layer);

    std::string m_name;
    std::map<int,Tileset*> m_tilesets;
    std::vector<Layer> m_layers;
    int m_width;
    int m_height;
    std::string m_bg_music;
};

#endif /* ILMENDUR_MAP_HPP */
