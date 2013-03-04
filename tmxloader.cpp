#include <stdio.h>
#include <cmath>
#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include "tinyxml.h"
#include "tinystr.h"
#include <stdlib.h>
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
    const char * image_source; //Image filename to pull the tileset from.
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
    const char * name;
    void parseXmlElement(TiXmlElement* element){
        firstgid = atoi(element->Attribute("firstgid"));
        name = element->Attribute("name");
        tilewidth = atoi(element->Attribute("tilewidth"));
        tileheight = atoi(element->Attribute("tileheight"));

        TiXmlElement * imageelem = element->FirstChildElement("image");
        image_source = imageelem->Attribute("source");
        imgwidth = atoi(imageelem->Attribute("width"));
        imgheight= atoi(imageelem->Attribute("height"));
        const char * transtmp = imageelem->Attribute("trans");
        if (transtmp)
            trans = al_color_html(transtmp);
    }
    void print(){
        char * str = "------\ntileset '%s'\n\nfirst tile ID: %i\ntilesize: (%i,%i)\ntileoffset: (%i,%i)\nspacing:%i\nmargin:%i\n\nimage source: '%s'\nimage dimensions: (%i,%i)\n------\n";
        printf(str, name,firstgid,tilewidth,tileheight,tileoffsetx,tileoffsety,spacing,margin, image_source,imgwidth,imgheight);
    }
};

class TileLayer{
public:
    char * name;
    int width; //tiles
    int height;//tiles
    char * compression; //either gzip or zlib.  If not set, no compression is used.
    char * encoding;    //either base64 or csv. If not set, no encoding is used, plain XML tags.

    int * tiles; // Array of tile IDs
};

class TileMap{
public:
    const char * filename;
    int width; //tiles
    int height;//tiles
    int tilewidth; //pixels
    int tileheight;//pixels
    //std::list<TileLayer> layers; //doubly-linked list
    TileLayer ** layers;
    //std::list<TileSet> tilesets; //doubly-linked list
    TileSet ** tilesets;
    ALLEGRO_COLOR backgroundcolor; //not implemented.

    TiXmlDocument doc;

    // Reads the file in question into TinyXML DOM format
    void load_from_file(const char * fname){
        filename = fname;
        TiXmlDocument doc(fname);
        doc.LoadFile();

        TiXmlElement *root = doc.FirstChildElement();
        width = atoi(root->Attribute("width"));
        height = atoi(root->Attribute("height"));
        tilewidth = atoi(root->Attribute("tilewidth"));
        tileheight= atoi(root->Attribute("tileheight"));
        TiXmlElement *r = root->FirstChildElement("tileset");

        print();
        printf("\n");
        TileSet *tempts;

        int numtilesets;
        for (numtilesets=0;r!=NULL;numtilesets++){
            r = r->NextSiblingElement("tileset");
        }

        TileSet * tilesets[numtilesets];
        r = root->FirstChildElement("tileset");
        while (r != NULL){
            numtilesets--;
            tempts = new TileSet;
            tempts->parseXmlElement(r);
            tilesets[numtilesets] = tempts;
            printf("TILESETLOADED %s",tempts->name);
            printf("\n\n%i\n\n",numtilesets);
            //tempts.print();

            r = r->NextSiblingElement("tileset");
        }
        for (int i = 0;i<sizeof(tilesets)/sizeof(tilesets[0]);i++){
            tilesets[i]->print();
        }
    }

    void print(){
        char * str = "------\nmap %s\n\ndimensions: (%i,%i) (tiles)\ntilesize: (%i,%i)\npixel dimensions: (%i,%i)\n%i layers\n%i tilesets\n------\n";
        //sizeof(array)/sizeof(array[0])
        printf(str, filename,width,height,tilewidth,tileheight,width*tilewidth,height*tileheight,sizeof(layers)/sizeof(layers[0]),sizeof(tilesets)/sizeof(tilesets[0]));
    }
};
