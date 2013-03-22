#ifndef _TILESET_H_
#define _TILESET_H_

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
#include "TileLayer.h"
#include "TileMap.h"

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
#endif //_TILESET_H_
