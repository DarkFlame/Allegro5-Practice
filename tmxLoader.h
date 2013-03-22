#ifndef _TMXLOADER_H_
#define _TMXLOADER_H_

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

class TileSet
{
private:
    void log(const char* instring, ...);

public:
    char image_source[256]; //Image filename to pull the tileset from.
    ALLEGRO_BITMAP *image;
    int imgwidth;  //pixels
    int imgheight; //pixels
    ALLEGRO_COLOR colorkey;

    int spacing; //pixels. Applies to the image
    int margin;  //pixels. Applies to the image

    int tileoffsetx; //pixels
    int tileoffsety; //pixels

    int tilewidth; //pixels
    int tileheight;//pixels
    int firstgid;  //the first tile id of this tileset. all following tiles will be incremented by 1
    const char * name;

    TileSet(TiXmlElement* element);
    ~TileSet();
    char* trim_filename(const char * filename);
    void load_image(const char * filename);
    void print();
};

class TileLayer
{
private:
    void log(const char* instring, ...);

public:
    char * name;
    int width; //tiles
    int height;//tiles
    const char * compression; //either gzip or zlib.  If not set, no compression is used.
    const char * encoding;    //either base64 or csv. If not set, no encoding is used, plain XML tags.

    int ** tiles; // 2D Array of tile IDs

    //TileLayer(const char * csvstring)
    TileLayer(TiXmlElement *elem);
    ~TileLayer();

    void parse_data(TiXmlElement *elem);

    void print();
};

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

    TileSet * get_tileset_for_id(int id);

    void print();
};
#endif // _TMXLOADER_H_
