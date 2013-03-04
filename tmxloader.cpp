#include <stdio.h>
#include <cmath>
#include <allegro5/allegro.h>
#include "tinyxml.h"
#include "tinystr.h"
#include <list>
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
    std::map<char *, char *> properties;
    int id;
};

class TileSet{
public:
    char * image_source; //Image filename to pull the tileset from.
    ALLEGRO_BITMAP *image;
    int imgwidth;  //pixels
    int imgheight; //pixels
    ALLEGRO_COLOR trans;

    int spacing; //pixels. Applies to the image
    int margin;  //pixels. Applies to the image

    int tileoffsetx; //pixels
    int tileoffsety; //pixels

    const int tilewidth; //pixels
    const int tileheight;//pixels
    const int firstgid;  //the first tile id of this tileset. all following tiles will be ++
    const char * name;
};

class TileLayer{
public:
    char * name;
    int width; //tiles
    int height;//tiles
    char * compression; //either gzip or zlib.  If not set, no compression is used.
    char * encoding;    //either base64 or csv. If not set, no encoding is used, plain XML tags.

    int * tiles;
};

class TileMap{
public:
    int width; //tiles
    int height;//tiles
    int tilewidth; //pixels
    int tileheight;//pixels
    std::list<TileLayer> layers; //doubly-linked list
    std::list<TileSet> tilesets; //doubly-linked list
    ALLEGRO_COLOR backgroundcolor; //not implemented.

    TiXmlDocument doc;

    // Reads the file in question into TinyXML DOM format
    void load_from_file(const char * filename){
        TiXmlDocument doc(filename);
        doc.LoadFile();

        TiXmlElement *root = doc.FirstChildElement();
        TiXmlElement *r = root->FirstChildElement("tileset");
        TileSet tempts = TileSet();
        while (r != NULL){
            // use strtol
            tempts.firstgid = r->Attribute("firstgid")-'0';
            tempts.name = r->Attribute("name");
            tempts.tilewidth = r->Attribute("tilewidth")-'0';
            tempts.tileheight = r->Attribute("tileheight")-'0';
            printf("--Tileset--\n%s\n",r->Attribute("name"));

            r = r->NextSiblingElement("tileset");
        }
    }
};
