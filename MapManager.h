#ifndef _MAPMANAGER_H_
#define _MAPMANAGER_H_

#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include "allegro5/allegro.h"

#include "TileSet.h"
#include "TileLayer.h"
#include "TileMap.h"
#include "Entity.h"
#include "Structures.h"
#include "Common.h"

class MapManager
{
private:
    Camera* camera;
    TileMap * maps[64]; //Static array. REALLOC IF NEEDED
    int total_maps;
    TileSet * tilesets[256]; //Static array. REALLOC IF NEEDED
    int total_tilesets;
    TileMap * active_map;
    Entity * player;

    void log(const char* instring, ...);

public:
    MapManager(Camera * cam, Entity * plyer);
    ~MapManager();

    void bound_camera();

    void set_camera(Camera* cam);
    void reset_camera(int x=0, int y=0);
    void add_map(const char * filename);
    void reload_map(const char * filename);
    TileMap * get_active_map();
    TileMap * get_map(const char * mapname);
    void set_active_map(const char * mapname);

    void drawbg();
    void drawmg();
    void drawfg();

    void drawlayer(int layerindex);

    void draw();
    void update();
};
#endif // _MAPMANAGER_H_
