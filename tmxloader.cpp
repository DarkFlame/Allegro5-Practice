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

class TileType
{
public:
    std::map<char *, char *> properties;
    int id;
};

class TileSet
{
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

    void parseXmlElement(TiXmlElement* element)
    {
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
    void print()
    {
        char * str = "------\ntileset '%s'\n\nfirst tile ID: %i\ntilesize: (%i,%i)\ntileoffset: (%i,%i)\nspacing:%i\nmargin:%i\n\nimage source: '%s'\nimage dimensions: (%i,%i)\n------\n";
        printf(str, name,firstgid,tilewidth,tileheight,tileoffsetx,tileoffsety,spacing,margin, image_source,imgwidth,imgheight);
    }
};

class TileLayer
{
public:
    const char * name;
    int width; //tiles
    int height;//tiles
    const char * compression; //either gzip or zlib.  If not set, no compression is used.
    const char * encoding;    //either base64 or csv. If not set, no encoding is used, plain XML tags.

    int ** tiles; // 2D Array of tile IDs
    //TileLayer(const char * csvstring)
    TileLayer(TiXmlElement *elem)
    {
        width = atoi(elem->Attribute("width"));
        height = atoi(elem->Attribute("height"));
        name = elem->Attribute("name");

        tiles = (int **)malloc(sizeof(int *) * width);
        for (int x=0;x<=width;x++)
        {
            tiles[x] = (int *)malloc(sizeof(int) * height);
        }

        TiXmlElement *data = elem->FirstChildElement("data");
        compression = data->Attribute("compression");
        encoding = data->Attribute("encoding");

        const char* csvstring;
        csvstring = data->GetText();
        printf("\nORIGINAL STRING\n'%s'\n\n",csvstring);
        printf("''%i''",csvstring[50]);
        char idbuffer[32];
        int bufslot = 0; // The index of idbuffer where the first null-termination lies
        int strindex = 0;
        for (int x=0;x<width;x++)
        {
            for (int y=0;y<height;y++)
            {
                printf("char: [%i]: %s", strindex,csvstring[strindex]);
                while (!(&csvstring[strindex]==",")) // NOT A COMMA, a digit in an ID.
                {
                    //TODO check for EOF
                    /*printf("\n\nchecking for EOF\n");
                    printf("%i\n",csvstring[strindex]);*/
                    if (csvstring[strindex] == 0)
                    {
                        //printf("\nEOF\n");
                        idbuffer[0] = 0;
                        break;
                    }
                    idbuffer[bufslot] = csvstring[strindex];
                    idbuffer[bufslot+1] = 0;
                    bufslot++;
                    strindex++;
                }
                printf("%i\n",idbuffer[0]);
                if (!(idbuffer[0] == 0))
                {
                    tiles[x][y] = atoi(idbuffer);
                }
                idbuffer[0] = 0;
                bufslot = 0;
                strindex++;
            }
        }
    }
    ~TileLayer()
    {
        for (int x=0;x<width;x++)
        {
            free(tiles[x]);
        }
        free(tiles);
    }

    void print()
    {
        const char * str = "\n------\nTile Layer '%s'\ndimensions:(%i,%i)\n%s %s\n";
        //printf(str, name,width,height,compression,encoding);
        int i=0;
        printf("\n\nLAYERS: %i\n\n",5);
        for (int y=0;y<height;y++)
        {
            for (int x=0;x<width;x++)
            {
                printf("%i ",tiles[x][y]);
            }
            printf("\n");
        }
    }
    /*
    to size:
    tiles = (int **)malloc(sizeof(int *) * width);
    for (int x=0;x<=width;x++){
        tiles[x] = (int *)malloc(sizeof(int) * height);
    }

    to access:
    tile = tiles[x/tilewidth][y/tileheight]
    CHECK TO MAKE SURE IN BOUNDS.

    to destroy:
    for (int x=0;x<width;x++){
        free(tiles[x]);
    }
    free(tiles);
    */
};

class TileMap
{
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

    ~TileMap()
    {
        for (int i=0;i<sizeof(layers)/sizeof(layers[0]);i++)
        {
            free(&layers[i]);
        }
        free(layers);
        for (int i=0;i<sizeof(tilesets)/sizeof(tilesets[0]);i++)
        {
            free(&tilesets[i]);
        }
        free(tilesets);
    }

    // Reads the file in question into TinyXML DOM format
    void load_from_file(const char * fname)
    {
        filename = fname;
        TiXmlDocument doc(fname);
        doc.LoadFile();

        //  Load map metadata
        TiXmlElement *root = doc.FirstChildElement();
        width = atoi(root->Attribute("width"));
        height = atoi(root->Attribute("height"));
        tilewidth = atoi(root->Attribute("tilewidth"));
        tileheight= atoi(root->Attribute("tileheight"));
        TiXmlElement *r = root->FirstChildElement("tileset");

        print();
        printf("\n");
        //  Load tileset metadata
        TileSet *tsbuf;

        int numtilesets;
        r = root->FirstChildElement("tileset");
        for (numtilesets=1;r!=NULL;numtilesets++)
        {
            r = r->NextSiblingElement("tileset");
        }

        TileSet * tilesets[numtilesets];
        r = root->FirstChildElement("tileset");
        while (r != NULL)
        {
            numtilesets--;
            tsbuf = new TileSet;
            tsbuf->parseXmlElement(r);
            tilesets[numtilesets] = tsbuf;
            //tsbuf.print();

            r = r->NextSiblingElement("tileset");
        }
        for (int i = 0;i<sizeof(tilesets)/sizeof(tilesets[0]);i++)
        {
            tilesets[i]->print();
        }

        //  Load Layer data
        TileLayer *tlbuf;

        int numlayers;
        r = root->FirstChildElement("layer");
        for (numlayers=1;r!=NULL;numtilesets++)
        {
            r = r->NextSiblingElement("layer");
        }

        TileLayer * tilelayers[numlayers];
        r = root->FirstChildElement("layer");
        while (r != NULL)
        {
            numlayers--;
            tlbuf = new TileLayer(r);
            tilelayers[numlayers] = tlbuf;

            r = r->NextSiblingElement("layer");
        }
        for (int i=0;i<sizeof(tilelayers)/sizeof(tilelayers[0]);i++)
        {
            tilelayers[i]->print();
        }
    }

    void print()
    {
        char * str = "------\nmap %s\n\ndimensions: (%i,%i) (tiles)\ntilesize: (%i,%i)\npixel dimensions: (%i,%i)\n%i layers\n%i tilesets\n------\n";
        //sizeof(array)/sizeof(array[0])
        printf(str, filename,width,height,tilewidth,tileheight,width*tilewidth,height*tileheight,sizeof(layers)/sizeof(layers[0]),sizeof(tilesets)/sizeof(tilesets[0]));
    }
};
