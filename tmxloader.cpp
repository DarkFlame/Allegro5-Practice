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

        printf("\nBeginning parse\n");
        const char* csvstring = data->GetText();
        char idbuffer[32];
        int bufslot = 0; // The index of idbuffer where the first null-termination lies
        int strindex = 0;
        for (int y=0;y<height;y++)
        {
            for (int x=0;x<width;x++)
            {
                printf("\nNext tile: ");
                while (csvstring[strindex]!=',')
                {
                    if (csvstring[strindex] >= '0' && csvstring[strindex] <= '9')
                    {
                        idbuffer[bufslot] = csvstring[strindex];
                        idbuffer[bufslot+1] = 0;
                        bufslot++;
                    }
                    else if (strindex >= strlen(csvstring))
                        break;
                    strindex++;
                }
                printf("ID %s",idbuffer);
                tiles[x][y] = atoi(idbuffer);
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
        const char * str = "\n------\nTile Layer '%s'\ndimensions:(%i,%i)\n%s encoding, %s compression\n";
        printf(str, name,width,height,encoding,compression);

        printf("  ");
        for (int x=0;x<width;x++)
            printf("-");
        printf("\n\n");

        int i=0;
        for (int y=0;y<height;y++)
        {
            printf("| ",y+1);
            for (int x=0;x<width;x++)
            {
                if (tiles[x][y] != 0)
                    printf("%i",tiles[x][y]);
                else
                    printf(" ");
            }
            printf(" |\n");
        }
        printf("\n  ");
        for (int x=0;x<width;x++)
            printf("-");
        printf("\n");
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

        int numtilesets=0;
        r = root->FirstChildElement("tileset");
        while (r!=NULL)
        {
            r = r->NextSiblingElement("tileset");
            numtilesets++;
        }

        TileSet ** tilesets = (TileSet **)malloc(sizeof(TileSet *)*numtilesets);
        r = root->FirstChildElement("tileset");
        for (int i=0;r!=NULL&&i<=numtilesets;i++)
        {
            tsbuf = new TileSet;
            tsbuf->parseXmlElement(r);
            tilesets[i] = tsbuf;
            //tsbuf.print();

            r = r->NextSiblingElement("tileset");
            printf("\nTilesets2");
        }
        for (int i=0;i<numtilesets;i++)
        {
            tilesets[i]->print();
            printf("\nPrinting tilesets");
        }

        //  Load Layer data
        TileLayer *tlbuf;

        int numlayers=0;
        r = root->FirstChildElement("layer");
        while (r!=NULL)
        {
            r = r->NextSiblingElement("layer");
            numlayers++;
        }

        TileLayer ** tilelayers = (TileLayer **)malloc(sizeof(TileLayer *)*numlayers);
        r = root->FirstChildElement("layer");
        for (int i=0;r!=NULL&&i<=numlayers;i++)
        {
            tlbuf = new TileLayer(r);
            tilelayers[i] = tlbuf;

            r = r->NextSiblingElement("layer");
        }
        for (int i=0;i<numlayers;i++)
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
