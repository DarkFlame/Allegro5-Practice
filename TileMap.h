#ifndef _TILEMAP_H_
#define _TILEMAP_H_

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_image.h>
#include "tinyxml.h"
#include "tinystr.h"
#include <stdlib.h>
#include <stdarg.h>
#include <zlib.h>

#include "Common.h"
#include "TileSet.h"
#include "TileLayer.h"

class TileMap
{
public:
    const char * filename;
    int width;      //tiles
    int height;     //tiles
    int tilewidth;  //pixels
    int tileheight; //pixels
    TileLayer ** tilelayers;
    int numlayers;
    TileSet ** tilesets;
    int numtilesets;
    ALLEGRO_COLOR backgroundcolor; //not implemented.

    TiXmlDocument *doc;

    void log(const char* instring, ...);

    // Reads the file in question into TinyXML DOM format
    TileMap(const char * fname);

    ~TileMap();

    void load_tilesets(TiXmlElement *root);

    void load_tilelayers(TiXmlElement *root);

    TileLayer * get_layer_for_name(char * name);

    TileSet * get_tileset_for_id(int id);

    void print();
};
#endif //_TILEMAP_H_
