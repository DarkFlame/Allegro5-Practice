#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include "tinyxml.h"
#include "tinystr.h"
#include <stdlib.h>
#include <zlib.h>

#include "Common.h"

//--Map file basic structure
//  Map             (version, orientation, width, height, tilewidth, tileheight)
//      Tileset     (firstgid, name, tilewidth, tileheight)
//          Image   (source, width, height)
//      Layer       (name, width, height)
//          Data    (encoding, compression)

class TileSet;
class TileLayer;
class TileMap;

/*
To find which tileset to get a tile from:
int i=0;
while (id_to_find > tilemap.tilesets[i]->firstgid)
{

    i++;
}
i--;
if (i<0) i=0;
*/

class TileSet
{
public:
    const char * image_source; //Image filename to pull the tileset from.
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

    TileSet(TiXmlElement* element)
    {
        name = element->Attribute("name");
        element->Attribute("firstgid",      &firstgid);
        element->Attribute("tilewidth",     &tilewidth);
        element->Attribute("tileheight",    &tileheight);
        element->Attribute("spacing",       &spacing);
        element->Attribute("margin",        &margin);
        element->Attribute("tileoffsetx",   &tileoffsetx);
        element->Attribute("tileoffsety",   &tileoffsety);

        TiXmlElement * imageelem = element->FirstChildElement("image");
        image_source = imageelem->Attribute("source");
        imageelem->Attribute("width",&imgwidth);
        imageelem->Attribute("height", &imgheight);

        const char * trans = imageelem->Attribute("trans");
        if (trans)
            colorkey = al_color_html(trans);
    }
    void print()
    {
        const char * str = "------\ntileset '%s'\n\nfirst tile ID: %i\ntilesize: (%i,%i)\ntileoffset: (%i,%i)\nspacing:%i\nmargin:%i\n\nimage source: '%s'\nimage dimensions: (%i,%i)\n------\n";
        printf(str, name,firstgid,tilewidth,tileheight,tileoffsetx,tileoffsety,spacing,margin, image_source,imgwidth,imgheight);
    }
};

class TileLayer
{
public:
    char * name;
    int width; //tiles
    int height;//tiles
    const char * compression; //either gzip or zlib.  If not set, no compression is used.
    const char * encoding;    //either base64 or csv. If not set, no encoding is used, plain XML tags.

    int ** tiles; // 2D Array of tile IDs
    //TileLayer(const char * csvstring)
    TileLayer(TiXmlElement *elem)
    {
        fprintf(stderr, "Initializing layer object\n");
        fprintf(stderr, "Getting attributes\n");
        elem->Attribute("width",    &width);
        elem->Attribute("height",   &height);
        name = NULL;
        ResetString(name, elem->Attribute("name"));

        fprintf(stderr, "Allocating tiles array\n");
        tiles = (int **)malloc(sizeof(int *) * width);
        fprintf(stderr, "Populating tiles array\n");
        for (int x=0;x<width;x++)
        {
            tiles[x] = (int *)malloc(sizeof(int) * height);
        }

        fprintf(stderr, "Calling parse_data()\n");
        parse_data(elem);
        fprintf(stderr, "TileLayer constructor successfully completed\n");
        fprintf(stderr, "\n");
    }
    ~TileLayer()
    {
        fprintf(stderr, "       Deleting layer data\n");
        for (int x=0;x<width;x++)
        {
            free(tiles[x]);
        }
        fprintf(stderr, "       Deleting layer data array\n");
        free(tiles);
    }

    void parse_data(TiXmlElement *elem)
    {
        fprintf(stderr,"Creating data element object\n");
        TiXmlElement *data = elem->FirstChildElement("data");
        compression = data->Attribute("compression");
        encoding = data->Attribute("encoding");

        fprintf(stderr, "Getting text for data\n");
        const char* csvstring = data->GetText();
        char idbuffer[32];
        int bufslot = 0; // The index of idbuffer where the first null-termination lies
        unsigned int strindex = 0;
        fprintf(stderr, "Beginning to parse data\n");
        for (int y=0;y<height;y++)
        {
            for (int x=0;x<width;x++)
            {
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
                tiles[x][y] = atoi(idbuffer);
                idbuffer[0] = 0;
                bufslot = 0;
                strindex++;
            }
        }
        fprintf(stderr, "Data successfully parsed\n");
    }

    void print()
    {
        const char * str = "\n------\nTile Layer '%s'\ndimensions:(%i,%i)\n%s encoding, %s compression\n";
        printf(str, name,width,height,encoding,compression);

        printf("  ");
        for (int x=0;x<width;x++)
            printf("-");
        printf("\n\n");

        for (int y=0;y<height;y++)
        {
            printf("| ");
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

    // Reads the file in question into TinyXML DOM format
    TileMap(const char * fname)
    {
        fprintf(stderr, "Loading XML document '%s'\n", fname);
        filename = fname;
        doc = new TiXmlDocument;
        doc->LoadFile(filename);
        fprintf(stderr, "Loaded document into TinyXML\n");

        //  Load map metadata
        fprintf(stderr, "Getting root element\n");
        TiXmlElement *root = doc->FirstChildElement();
        fprintf(stderr, "Getting root attributes\n");
        root->Attribute("width",        &width);
        root->Attribute("height",       &height);
        root->Attribute("tilewidth",    &tilewidth);
        root->Attribute("tileheight",   &tileheight);
        fprintf(stderr, "\n");

        /*fprintf(stderr, "Printing map\n");
        print();
        printf("\n");*/
        fprintf(stderr, "Loading tilesets with load_tilesets()\n");
        load_tilesets(root);
        fprintf(stderr, "Loading layers with load_tilelayers()\n");
        load_tilelayers(root);
        fprintf(stderr, "TileMap object constructor successfully completed\n");
        fprintf(stderr, "\n");
    }

    ~TileMap()
    {
        fprintf(stderr, "\nDeleting doc\n");
        delete doc;
        fprintf(stderr, "Deleting tilelayers\n");
        for (unsigned int i=0;i<numlayers;i++)
        {
            fprintf(stderr, "   Deleting layer %i\n", i);
            delete tilelayers[i];
        }
        fprintf(stderr, "Deleting layers array\n");
        free(tilelayers);
        fprintf(stderr, "Deleting tilesets\n");
        for (unsigned int i=0;i<numtilesets;i++)
        {
            fprintf(stderr, "   Deleting tileset %i\n", i);
            delete tilesets[i];
        }
        fprintf(stderr, "Deleting tilesets array\n");
        free(tilesets);
    }

    void load_tilesets(TiXmlElement *root)
    {
        fprintf(stderr, "Getting first tileset element\n");
        TiXmlElement *r = root->FirstChildElement("tileset");

        numtilesets=0;
        r = root->FirstChildElement("tileset");
        while (r!=NULL)
        {
            r = r->NextSiblingElement("tileset");
            numtilesets++;
            fprintf(stderr, "Another tileset found. %i tilesets\n", numtilesets);
        }

        fprintf(stderr, "Allocating tilesets array\n");
        tilesets = (TileSet **)malloc(sizeof(TileSet *)*numtilesets);
        fprintf(stderr, "Finding first tileset element again\n");
        r = root->FirstChildElement("tileset");
        fprintf(stderr, "Iterating over tilesets\n");
        for (int i=0;r!=NULL&&i<=numtilesets;i++)
        {
            fprintf(stderr, "   Creating new TileSet object\n");
            tilesets[i] = new TileSet(r);

            fprintf(stderr, "   Checking for next tileset element\n\n");
            r = r->NextSiblingElement("tileset");
        }
        /*fprintf(stderr, "Printing tilesets\n");
        for (int i=0;i<numtilesets;i++)
        {
            tilesets[i]->print();
        }*/
        fprintf(stderr, "load_tilesets() complete\n");
        fprintf(stderr, "\n");
    }

    void load_tilelayers(TiXmlElement *root)
    {
        numlayers=0;
        fprintf(stderr, "Getting first layer element\n");
        TiXmlElement *r = root->FirstChildElement("layer");
        while (r!=NULL)
        {
            r = r->NextSiblingElement("layer");
            numlayers++;
            fprintf(stderr, "Another layer found. %i layers\n", numlayers);
        }

        fprintf(stderr, "Allocating layers array\n");
        tilelayers = (TileLayer **)malloc(sizeof(TileLayer *)*numlayers);
        fprintf(stderr, "Finding first layer element again\n");
        r = root->FirstChildElement("layer");
        fprintf(stderr, "Iterating over layers\n");
        for (int i=0;r!=NULL&&i<=numlayers;i++)
        {
            fprintf(stderr, "   Creating new TileLayer object\n");
            tilelayers[i] = new TileLayer(r);

            fprintf(stderr, "   Checking for next layer element\n\n");
            r = r->NextSiblingElement("layer");
        }
        /*fprintf(stderr, "Printing layers\n");
        for (int i=0;i<numlayers;i++)
        {
            tilelayers[i]->print();
        }*/
        fprintf(stderr, "load_tilelayers complete.\n");
        fprintf(stderr, "\n");
    }

    void print()
    {
        const char * str = "------\nmap %s\n\ndimensions: (%i,%i) (tiles)\ntilesize: (%i,%i)\npixel dimensions: (%i,%i)\n%i layers\n%i tilesets\n------\n";
        //sizeof(array)/sizeof(array[0])
        printf(str, filename,width,height,tilewidth,tileheight,width*tilewidth,height*tileheight,numlayers,numtilesets);
    }
};
