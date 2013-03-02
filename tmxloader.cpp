#include <stdio.h>
#include <cmath>
#include <allegro5/allegro.h>
#include "tinyxml.h"
#include "tinystr.h"
#include <list>
#include <string>
#include <map>
#include <zlib.h>

//--Map file basic structure
//  Map             (version, orientation, width, height, tilewidth, tileheight)
//      Tileset     (firstgid, name, tilewidth, tileheight)
//          Image   (source, width, height)
//      Layer       (name, width, height)
//          Data    (encoding, compression)

class TileType;
class TileSet;
class TileLayer;
class TileMap;

class TileType{
public:
    std::map<std::string, std::string> properties;
    int id;
};

class TileSet{
public:
    std::string image_source; //Image filename to pull the tileset from.
    ALLEGRO_BITMAP *image;
    int imgwidth;  //pixels
    int imgheight; //pixels
    ALLEGRO_COLOR trans;

    int spacing; //pixels. Applies to the image
    int margin;  //pixels. Applies to the image

    int tileoffsetx; //pixels
    int tileoffsety; //pixels

    int tilewidth; //pixels
    int tileheight;//pixels
    int firstgid;  //the first tile id of this tileset. all following tiles will be ++
    std::string name;

    void load_from_file(std::string filename);
};

class TileLayer{
public:
    std::string name;
    int width; //tiles
    int height;//tiles
    std::string compression; //either gzip or zlib.  If not set, no compression is used.
    std::string encoding;    //either base64 or csv. If not set, no encoding is used, plain XML tags.

    std::list<int> tiles;
};

class TileMap{
public:
    int width; //tiles
    int height;//tiles
    int tilewidth; //pixels
    int tileheight;//pixels
    std::list<TileLayer> layers; //doubly-linked list
    ALLEGRO_COLOR backgroundcolor; //not implemented.

    TiXmlDocument doc;

    // Reads the file in question into TinyXML DOM format
    void load_from_file(const char filename){
        TiXmlDocument doc(&filename);
        doc.LoadFile();
        doc.Print();
    }
};
