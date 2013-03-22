#include "TileLayer.h"

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
