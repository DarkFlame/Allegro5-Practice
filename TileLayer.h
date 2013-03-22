#ifndef _TILELAYER_H_
#define _TILELAYER_H_

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
#include "TileMap.h"

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
#endif //_TILELAYER_H_
