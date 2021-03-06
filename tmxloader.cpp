#include "tmxLoader.h"

//--Map file basic structure
//  Map             (version, orientation, width, height, tilewidth, tileheight)
//      Tileset     (firstgid, name, tilewidth, tileheight)
//          Image   (source, width, height)
//      Layer       (name, width, height)
//          Data    (encoding, compression)

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



void TileSet::log(const char* instring, ...)
{
    //--Logs instring to stderr with some default formatting
    char buffer[256];

    fprintf(stderr,"[TileSet] ");

    va_list args;
    va_start (args, instring);
    vsprintf(buffer, instring, args);
    fprintf(stderr,buffer);
    va_end (args);
    fprintf(stderr,"\n");
}

TileSet::TileSet(TiXmlElement* element)
{
    log("Setting values from attributes");
    name = element->Attribute("name");
    element->Attribute("firstgid",      &firstgid);
    element->Attribute("tilewidth",     &tilewidth);
    element->Attribute("tileheight",    &tileheight);
    element->Attribute("spacing",       &spacing);
    element->Attribute("margin",        &margin);
    element->Attribute("tileoffsetx",   &tileoffsetx);
    element->Attribute("tileoffsety",   &tileoffsety);

    log("Getting image element");
    TiXmlElement * imageelem = element->FirstChildElement("image");
    //image_source = {0};
    strncpy(image_source, trim_filename(imageelem->Attribute("source")),256);
    log("Found image source: '%s'",image_source);
    imageelem->Attribute("width",&imgwidth);
    imageelem->Attribute("height", &imgheight);

    const char * trans = imageelem->Attribute("trans");
    if (trans)
        colorkey = al_color_html(trans);
        log("Colorkey set");

    load_image(image_source);
}
TileSet::~TileSet()
{
    log( "Deleting TileSet");
    //delete image;
}
char* TileSet::trim_filename(const char * filename)
{
    char fnamebuf[256] = {0};
    char fnamecpy[256] = {0};
    const char * datastring = "data";
    //--Remove the first two characters in filename
    strncpy(fnamecpy,filename,256);
    unsigned int i;
    for (i=2;i<strlen(filename);i++)
    {
        fnamecpy[i-2] = filename[i];
    }
    strncpy(fnamebuf,datastring,10);
    strncpy(&fnamebuf[strlen(datastring)],fnamecpy,256);
    fnamebuf[strlen(fnamebuf)-2] = 0; // For some reason it duplicates the last two characters, so here's a hack for that. TODO
    log("Altering filename from '%s' to '%s'",filename,fnamebuf);
    return fnamebuf;
}
void TileSet::load_image(const char * filename)
{
    //--Load the image into *image
    log("Loading tileset image '%s'",filename);
    image = al_load_bitmap(filename);
    if (image == NULL)
    {
        log("ERROR image file '%s' not found.",filename);
    }
    al_convert_mask_to_alpha(image, colorkey);
}
void TileSet::print()
{
    const char * str = "------\ntileset '%s'\n\nfirst tile ID: %i\ntilesize: (%i,%i)\ntileoffset: (%i,%i)\nspacing:%i\nmargin:%i\n\nimage source: '%s'\nimage dimensions: (%i,%i)\n------\n";
    printf(str, name,firstgid,tilewidth,tileheight,tileoffsetx,tileoffsety,spacing,margin, image_source,imgwidth,imgheight);
}


void TileLayer::log(const char* instring, ...)
{
    //--Logs instring to stderr with some default formatting
    char buffer[256];

    fprintf(stderr,"[TileLayer] ");

    va_list args;
    va_start (args, instring);
    vsprintf(buffer, instring, args);
    fprintf(stderr,buffer);
    va_end (args);
    fprintf(stderr,"\n");
}

//TileLayer(const char * csvstring)
TileLayer::TileLayer(TiXmlElement *elem)
{
    log("Initializing layer object");
    log( "Getting attributes");
    elem->Attribute("width",    &width);
    elem->Attribute("height",   &height);
    name = NULL;
    ResetString(name, elem->Attribute("name"));

    log( "Allocating tiles array");
    tiles = (int **)malloc(sizeof(int *) * width);
    log( "Populating tiles array");
    for (int x=0;x<width;x++)
    {
        tiles[x] = (int *)malloc(sizeof(int) * height);
    }

    log( "Calling parse_data()");
    parse_data(elem);
    log( "TileLayer constructor successfully completed");
    log( "");
}
TileLayer::~TileLayer()
{
    log("Deleting layer data");
    for (int x=0;x<width;x++)
    {
        free(tiles[x]);
    }
    log("Deleting layer data array");
    free(tiles);
}

void TileLayer::parse_data(TiXmlElement *elem)
{
    log("Creating data element object");
    TiXmlElement *data = elem->FirstChildElement("data");
    compression = data->Attribute("compression");
    encoding = data->Attribute("encoding");

    log( "Getting text for data");
    const char* csvstring = data->GetText();
    char idbuffer[32];
    int bufslot = 0; // The index of idbuffer where the first null-termination lies
    unsigned int strindex = 0;
    log( "Beginning to parse data");
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
    log( "Data successfully parsed");
}

void TileLayer::print()
{
    const char * str = "\n------\nTile Layer '%s'\ndimensions:(%i,%i)\n%s encoding, %s compression\n";
    printf(str, name,width,height,encoding,compression);

    printf("  ");
    for (int x=0;x<width;x++)
        printf("- ");
    printf("\n\n");

    for (int y=0;y<height;y++)
    {
        printf("| ");
        for (int x=0;x<width;x++)
        {
            if (tiles[x][y] != 0)
                printf("%i ",tiles[x][y]);
            else
                printf(" ");
        }
        printf(" |\n");
    }
    printf("\n  ");
    for (int x=0;x<width;x++)
        printf("- ");
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

// Reads the file in question into TinyXML DOM format
TileLayer::TileMap(const char * fname)
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

TileLayer::~TileMap()
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

void TileLayer::load_tilesets(TiXmlElement *root)
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

void TileLayer::load_tilelayers(TiXmlElement *root)
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

TileSet * TileLayer::get_tileset_for_id(int id)
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

void TileLayer::print()
{
    const char * str = "------\nmap %s\n\ndimensions: (%i,%i) (tiles)\ntilesize: (%i,%i)\npixel dimensions: (%i,%i)\n%i layers\n%i tilesets\n------\n";
    //sizeof(array)/sizeof(array[0])
    printf(str, filename,width,height,tilewidth,tileheight,width*tilewidth,height*tileheight,numlayers,numtilesets);
    printf("\n");
}
