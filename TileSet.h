#ifndef _TILESET_H_
#define _TILESET_H_

#include "Definitions.h"

struct StringHolder;

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
    StringHolder trim_filename(const char * filename);
    void load_image(const char * filename);
    void print();
};
#endif //_TILESET_H_
