#ifndef _TILELAYER_H_
#define _TILELAYER_H_

#include "Definitions.h"

class TileLayer
{
private:
    void log(const char* instring, ...);

public:
    char * name;
    int width; //tiles
    int height;//tiles
    const char * compression; //either gzip or zlib.  If not set, no compression is used.
    const char * encoding;    //either base64 or csv. If not set, no encoding is used, plain XML tags.

    int ** tiles; // 2D Array of tile IDs

    //TileLayer(const char * csvstring)
    TileLayer(TiXmlElement *elem);
    ~TileLayer();

    void parse_data(TiXmlElement *elem);

    void print();
};
#endif //_TILELAYER_H_
