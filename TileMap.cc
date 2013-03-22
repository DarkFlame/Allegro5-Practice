#include "TileMap.h"

void TileMap::TileMap::log(const char* instring, ...)
{
    //--Logs instring to stderr with some default formatting
    char buffer[256];

    fprintf(stderr,"[TileMap] ");

    va_list args;
    va_start (args, instring);
    vsprintf(buffer, instring, args);
    fprintf(stderr,buffer);
    va_end (args);
    fprintf(stderr,"\n");
}

// Reads the file in question into TinyXML DOM format
TileMap::TileMap(const char * fname)
{
    log("Loading XML document '%s'", fname);
    filename = fname;
    doc = new TiXmlDocument;
    doc->LoadFile(filename);
    log( "Loaded document into TinyXML");

    //  Load map metadata
    log( "Getting root element");
    TiXmlElement *root = doc->FirstChildElement();
    log( "Getting root attributes");
    root->Attribute("width",        &width);
    root->Attribute("height",       &height);
    root->Attribute("tilewidth",    &tilewidth);
    root->Attribute("tileheight",   &tileheight);

    //log( "Printing map");
    //print();
    log( "Loading tilesets with load_tilesets()");
    load_tilesets(root);
    log( "Loading layers with load_tilelayers()");
    load_tilelayers(root);
    log( "TileMap object constructor successfully completed\n");
}

TileMap::~TileMap()
{
    log( "Deleting doc");
    delete doc;
    log( "Deleting tilelayers");
    for (int i=0;i<numlayers;i++)
    {
        log( "Deleting layer %i", i);
        delete tilelayers[i];
    }
    log( "Deleting layers array");
    free(tilelayers);
    log( "Deleting tilesets");
    for (int i=0;i<numtilesets;i++)
    {
        log( "Deleting tileset %i", i);
        delete tilesets[i];
    }
    log( "Deleting tilesets array");
    free(tilesets);
    log("TileMap destructor complete.");
}

void TileMap::load_tilesets(TiXmlElement *root)
{
    log( "Getting first tileset element");
    TiXmlElement *r = root->FirstChildElement("tileset");

    numtilesets=0;
    r = root->FirstChildElement("tileset");
    while (r!=NULL)
    {
        r = r->NextSiblingElement("tileset");
        numtilesets++;
        log( "Another tileset found. %i tilesets", numtilesets);
    }

    log( "Allocating tilesets array");
    tilesets = (TileSet **)malloc(sizeof(TileSet *)*numtilesets);
    log( "Finding first tileset element again");
    r = root->FirstChildElement("tileset");
    log( "Iterating over tilesets");
    for (int i=0;r!=NULL&&i<=numtilesets;i++)
    {
        log( "Creating new TileSet object");
        tilesets[i] = new TileSet(r);

        log( "Checking for next tileset element\n");
        r = r->NextSiblingElement("tileset");
    }
    /*log( "Printing tilesets");
    for (int i=0;i<numtilesets;i++)
    {
        tilesets[i]->print();
    }*/
    log( "load_tilesets() complete\n");
}

void TileMap::load_tilelayers(TiXmlElement *root)
{
    numlayers=0;
    log( "Getting first layer element");
    TiXmlElement *r = root->FirstChildElement("layer");
    while (r!=NULL)
    {
        r = r->NextSiblingElement("layer");
        numlayers++;
        log( "Another layer found. %i layers", numlayers);
    }

    log( "Allocating layers array");
    tilelayers = (TileLayer **)malloc(sizeof(TileLayer *)*numlayers);
    log( "Finding first layer element again");
    r = root->FirstChildElement("layer");
    log( "Iterating over layers");
    for (int i=0;r!=NULL&&i<=numlayers;i++)
    {
        log( "Creating new TileLayer object");
        tilelayers[i] = new TileLayer(r);

        log( "Checking for next layer element\n");
        r = r->NextSiblingElement("layer");
    }
    /*log( "Printing layers");
    for (int i=0;i<numlayers;i++)
    {
        tilelayers[i]->print();
    }*/
    log( "load_tilelayers complete.");
    log( "\n");
}

TileSet * TileMap::get_tileset_for_id(int id)
{
    //log( "Searching for tileset with tile ID %i", id);
    if (id==0) //0 is transparent. No tileset.
    {
        return NULL;
    }
    if (id==1)
    {
        return tilesets[0];
    }

    //--Iterate over tilesets until id < firstgid
    //    If it's the last tileset in the list, check the number of tiles and return it if it fits.
    //    Otherwise, return NULL
    //  decrement index by 1 and return

    int i = 0;
    while (id > tilesets[i]->firstgid)
    {
        //log("Next tileset (%i)",i);
        i++;
        if (i+1>numtilesets)
        {
            //log("Last tileset");
            i--;
            //--Check to see if the tile ID fits on the tileset
            int tilesinset =
                (tilesets[i]->imgwidth/tilesets[i]->tilewidth)*(tilesets[i]->imgheight/tilesets[i]->tileheight);

            //log("%i tiles in set",tilesinset);
            if (id <= tilesinset+tilesets[i]->firstgid)
            {
                //log("In the last tileset. returning tileset %s",tilesets[i]->name);
                return tilesets[i];
            }
            else
            {
                log("INVALID TILE ID. RETURNING NULL");
                return NULL;
            }
        }
    }
    i--;
    //log("Tile %i found on tileset %s",id,tilesets[i]->name);
    return tilesets[i];
}

void TileMap::print()
{
    const char * str = "------\nmap %s\n\ndimensions: (%i,%i) (tiles)\ntilesize: (%i,%i)\npixel dimensions: (%i,%i)\n%i layers\n%i tilesets\n------\n";
    //sizeof(array)/sizeof(array[0])
    printf(str, filename,width,height,tilewidth,tileheight,width*tilewidth,height*tileheight,numlayers,numtilesets);
    printf("\n");
}
