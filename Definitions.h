
#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include <stdlib.h>
#include <zlib.h>

#include "allegro5/allegro.h"
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_image.h>

#include "tinyxml.h"
#include "tinystr.h"

class Entity;
struct Camera;
class TileMap;
class TileSet;

const float FPS = 60;
const int SCREEN_W = 640;
const int SCREEN_H = 480;
const int SPRITE_W = 23;
const int SPRITE_H = 31;
enum MYKEYS
{
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

#endif // _DEFINITIONS_H_
