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
        elem->Attribute("width",    &width);
        elem->Attribute("height",   &height);
        name = NULL;
        ResetString(name, elem->Attribute("name"));

        tiles = (int **)malloc(sizeof(int *) * width);
        for (int x=0;x<=width;x++)
        {
            tiles[x] = (int *)malloc(sizeof(int) * height);
        }

        TiXmlElement *data = elem->FirstChildElement("data");
        compression = data->Attribute("compression");
        encoding = data->Attribute("encoding");

        const char* csvstring = data->GetText();
        char idbuffer[32];
        int bufslot = 0; // The index of idbuffer where the first null-termination lies
        unsigned int strindex = 0;
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
        filename = fname;
        doc = new TiXmlDocument;
        doc->LoadFile(filename);

        //  Load map metadata
        TiXmlElement *root = doc->FirstChildElement();
        root->Attribute("width",        &width);
        root->Attribute("height",       &height);
        root->Attribute("tilewidth",    &tilewidth);
        root->Attribute("tileheight",   &tileheight);

        print();
        printf("\n");
        load_tilesets(root);
        load_tilelayers(root);
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
        TiXmlElement *r = root->FirstChildElement("tileset");

        numtilesets=0;
        r = root->FirstChildElement("tileset");
        while (r!=NULL)
        {
            r = r->NextSiblingElement("tileset");
            numtilesets++;
        }

        tilesets = (TileSet **)malloc(sizeof(TileSet *)*numtilesets);
        r = root->FirstChildElement("tileset");
        for (int i=0;r!=NULL&&i<=numtilesets;i++)
        {
            tilesets[i] = new TileSet(r);
            //tsbuf.print();

            r = r->NextSiblingElement("tileset");
        }
        for (int i=0;i<numtilesets;i++)
        {
            tilesets[i]->print();
        }
    }

    void load_tilelayers(TiXmlElement *root)
    {
        TileLayer *tlbuf;

        numlayers=0;
        TiXmlElement *r = root->FirstChildElement("layer");
        while (r!=NULL)
        {
            r = r->NextSiblingElement("layer");
            numlayers++;
        }

        tilelayers = (TileLayer **)malloc(sizeof(TileLayer *)*numlayers);
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
        const char * str = "------\nmap %s\n\ndimensions: (%i,%i) (tiles)\ntilesize: (%i,%i)\npixel dimensions: (%i,%i)\n%i layers\n%i tilesets\n------\n";
        //sizeof(array)/sizeof(array[0])
        printf(str, filename,width,height,tilewidth,tileheight,width*tilewidth,height*tileheight,numlayers,numtilesets);
    }
};
