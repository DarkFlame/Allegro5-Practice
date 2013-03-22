#include "TileSet.h"

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
