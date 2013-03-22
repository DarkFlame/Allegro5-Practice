#include "MapManager.h"

#include "TileSet.h"
#include "TileLayer.h"
#include "TileMap.h"
#include "Entity.h"
#include "Structures.h"
#include "Common.h"

void MapManager::log(const char* instring, ...)
{
    //--Logs instring to stderr with some default formatting
    char buffer[256];

    fprintf(stderr,"[MapManager] ");

    va_list args;
    va_start (args, instring);
    vsprintf(buffer, instring, args);
    fprintf(stderr,buffer);
    va_end (args);
    fprintf(stderr,"\n");
}

void MapManager::load_map(TileMap* map_to_load)
{
    //--Loads the map's tilesets (If not already)
    log("Loading map '%s'",map_to_load->filename);
}

MapManager::MapManager(Camera * cam, Entity * plyer)
{
    player = plyer;
    camera = cam;

    //--Constructor
    for (int i=0;i<63;i++)
    {
        maps[i] = NULL;
    }
    for (int i=0;i<255;i++)
    {
        tilesets[i] = NULL;
    }
    total_maps = 0;
    total_tilesets = 0;
}
MapManager::~MapManager()
{
    //--Destroys all maps at the end of the program
    log("Destructor beginning");
    for (int i=0;i<63;i++)
    {
        if (maps[i] != NULL)
        {
            log("Deleting map '%s'",maps[i]->filename);
            delete maps[i];
        }
    }
    /*log("Deleting maps array");
    delete maps;*/
    log("Destructor completed");
}

void MapManager::bound_camera()
{
    camera->stopped = false;
    //--Checks the camera's offset and bounds it within the level if it's outside of it.
    if (camera->x<0)
    {
        camera->x = 0;
        camera->stopped = true;
    }
    if (camera->y<0)
    {
        camera->y = 0;
        camera->stopped = true;
    }
    if (camera->x > active_map->width*active_map->tilewidth-SCREEN_W)
    {
        camera->x = active_map->width*active_map->tilewidth-SCREEN_W;
        camera->stopped = true;
    }
    if (camera->y > active_map->height*active_map->tileheight-SCREEN_H)
    {
        camera->y = active_map->height*active_map->tileheight-SCREEN_H;
        camera->stopped = true;
    }
}

void MapManager::set_camera(Camera* cam)
{
    camera = cam;
}
void MapManager::reset_camera(int x=0, int y=0)
{
    //--Resets the camera to position x,y
    log("Reset the camera to (%i,%i)",x,y);
    camera->x=x;
    camera->y=y;
        fprintf(stderr, "FOOBAR\n");
}
void MapManager::add_map(const char * filename)
{
    //--Adds map_to_add to the maps to manage
    maps[total_maps] = new TileMap(filename);
    log("Added new map to Map Manager '%s'",maps[total_maps]->filename);
    load_map(maps[total_maps]);
    total_maps++;
}
void MapManager::reload_map(const char * filename)
{
    //--Reloads all data related to the map at filename
    log("Reloading map '%s'",filename);
    load_map(get_map(filename));
}
TileMap * MapManager::get_active_map()
{
    return active_map;
}
TileMap * MapManager::get_map(const char * mapname)
{
    //--Iterates over maps and returns the first TileMap with the name mapname
    int i = 0;
    int result = strncmp(mapname,maps[i]->filename, 256);
    if (result==0)
    {
        return maps[i];
    }
    while (result != 0)
    {
        i++;
        if (i>=total_maps)
        {
            return NULL;
        }
        result = strncmp(mapname,maps[i]->filename, 256);
    }
    return maps[i-1];
}
void MapManager::set_active_map(const char * mapname)
{
    //--Switches maps to the map of name mapname
    //active_map = mapname;
    TileMap* mapbuf = get_map(mapname);
    if (!mapbuf)
    {
        log("Map '%s' does not exist. Could not set active.",mapname);
        return;
    }
    active_map = mapbuf;
    log("Set active map to '%s'",mapname);
    reset_camera();
}

void MapManager::drawbg()
{
    //-- Draws the background only
    for (int layerindex=0;layerindex<active_map->numlayers;layerindex++)
    {
        //--Check if it's the midground. If it is, then stop here
        if (strncmp(active_map->tilelayers[layerindex]->name, "midground", 256) == 0)
        {
            return;
        }
        drawlayer(layerindex);
    }
}
void MapManager::drawmg()
{
    //--Draws the midground only
    for (int layerindex=0;layerindex<active_map->numlayers;layerindex++)
    {
        if (strncmp(active_map->tilelayers[layerindex]->name, "midground", 256) == 0)
        {
            drawlayer(layerindex);
        }
    }
}
void MapManager::drawfg()
{
    //--Draws the foreground only
    bool hitmid = false;
    for (int layerindex=0;layerindex<active_map->numlayers;layerindex++)
    {
        if (hitmid)
        {
            drawlayer(layerindex);
        }
        if (strncmp(active_map->tilelayers[layerindex]->name, "midground", 256) == 0)
        {
            hitmid = true;
        }
    }
}

void MapManager::drawlayer(int layerindex)
{
    for (int y=0;y<active_map->tilelayers[layerindex]->height;y++)
        {
            //log("Starting y iteration");
            for (int x=0;x<active_map->tilelayers[layerindex]->width;x++)
            {
                //log("Starting x iteration");
                //--ID is active_map->tilelayers[layerindex]->tiles[x][y]
                TileLayer * tlbuf = active_map->tilelayers[layerindex];
                //log("Got tilelayer %s",tlbuf->name);
                //log("Getting tile at (%i,%i)",x,y);
                int id = tlbuf->tiles[x][y];
                //log("Getting tileset for id %i",id);
                TileSet * tsbuf = active_map->get_tileset_for_id(id);
                /*if (tsbuf == NULL)
                {
                    break;
                }*/
                //log("Got tileset %s",tsbuf->name);
                //--Get the location of the tile on the tileset image
                if (tsbuf != NULL)
                {
                    int toffset = active_map->tilelayers[layerindex]->tiles[x][y] - tsbuf->firstgid;
                    //log("toffset is %i",toffset);
                    int sx,sy;
                    //--Wrap toffset on the x
                    if (toffset < tsbuf->imgwidth/tsbuf->tilewidth)
                    {
                        //--It's in the first row, this is easy.
                        sx = toffset*tsbuf->tilewidth;
                        sy = 0;
                    }
                    else
                    {
                        int times = 0;
                        int toffsetcopy = toffset;
                        while (toffset >= tsbuf->imgwidth/tsbuf->tilewidth)
                        {
                            toffset -= tsbuf->imgwidth/tsbuf->tilewidth;
                            times++;
                        }

                        //log("toffset remainder %i",toffset);
                        sx = tsbuf->tilewidth*(toffsetcopy-(times*tsbuf->imgwidth/tsbuf->tilewidth));
                        sy = tsbuf->tileheight*times;
                        //log("s (%i,%i)",sx,sy);
                    }
                    int dx = x*active_map->tilewidth+-camera->x;
                    int dy = y*active_map->tileheight+-camera->y;
                    al_draw_bitmap_region(tsbuf->image, sx,sy, tsbuf->tilewidth,tsbuf->tileheight, dx,dy, 0);
                }

            }
        }
}

void MapManager::draw()
{
    //--Draws the active map to target
    //--DRAWING THE TILESET FOR PLACEHOLDER
    //--For each layer in the current map
    for (int layerindex=0;layerindex<active_map->numlayers;layerindex++)
    {
        /*if (strncmp(active_map->tilelayers[layerindex]->name, "midground", 256) != 0)
        {
            break;
        }*/
        //log("Iterating over a layer");
        //--For each row in the current layer on the current map
        drawlayer(layerindex);
    }
}
void MapManager::update()
{
    //--Possibly unneeded. Calls game logic for each map
    //  might have another function for the manager itself such
    //  as camera movement.
    //camera->x = camera->x+player->curSpeed.x;
    //camera->y = camera->y+player->curSpeed.y;
    bound_camera();
}
