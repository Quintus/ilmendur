#ifndef ILMENDUR_MAP_HPP
#define ILMENDUR_MAP_HPP
#include "tileset.hpp"
#include "globals.hpp"
#include <vector>
#include <map>

class Actor;
class Player;
class Map;

class MapLayer
{
public:
    MapLayer(Map& map, std::string name, Properties props);
    virtual ~MapLayer();

    virtual void update() = 0;
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview) = 0;

    inline const std::string& name() { return m_name; }
    inline const Properties& props() { return m_props; }
    inline Map& map() { return mr_map; }
protected:
    Map& mr_map;
    std::string m_name;
    Properties m_props;
};

class TileLayer: public MapLayer
{
public:
    enum class layer_direction { up, down, both };

    TileLayer(Map& map, std::string name, Properties props, int width, int height, std::vector<int> gids);
    virtual void update();
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
private:
    bool readTile(SDL_Texture*& p_texid, SDL_Rect& rect, int gid);

    int m_width;
    int m_height;
    std::vector<int> m_gids;
    layer_direction m_dir;
};

class ObjectLayer: public MapLayer
{
public:
    ObjectLayer(Map& map, std::string name, Properties props);
    virtual ~ObjectLayer();
    virtual void update();
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    inline const std::vector<Actor*>& actors() { return m_actors; }

    void addActor(Actor* p_actor);
    void releaseActor(Actor* p_actor);
private:
    void checkCollisions();
    void checkCollideMapBoundary(Actor* p_actor);
    void checkCollideActors(Actor* p_actor);

    std::vector<Actor*> m_actors;
};

class Map
{
public:
    Map(const std::string& name);
    ~Map();

    void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    void update();
    SDL_Rect drawRect() const;

    void makeHeroes();
    void heroes(Player** p_freya, Player** p_benjamin);

    bool findActor(int id, Actor** pp_actor);
    void changeActorLayer(Actor* p_actor, const std::string& target_layer_name);
    std::vector<Actor*> findAdjascentActors(Actor* p_actor, direction dir);

    inline const std::string& backgroundMusic() const { return m_bg_music; }

    const std::string& name() { return m_name; }
    std::map<int, Tileset*>& tilesets() { return m_tilesets; }

    // Helper types for dealing with Tiled layers. Actually, only
    // Tile and Object are supported by the Layer struct.
    // enum class LayerType { Tile, Object, Image, Group };
    // struct Layer {
    //     LayerType type;
    //     union {
    //         TmxTileLayer* p_tile_layer;
    //         TmxObjLayer* p_obj_layer;
    //     } data;
    // };

private:
    std::string m_name;
    std::map<int,Tileset*> m_tilesets;
    std::vector<MapLayer*> m_layers;
    int m_width;
    int m_height;
    std::string m_bg_music;
    Player* mp_freya;
    Player* mp_benjamin;
};

#endif /* ILMENDUR_MAP_HPP */
