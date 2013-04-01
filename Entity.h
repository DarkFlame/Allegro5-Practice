#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <stdio.h>
#include "allegro5/allegro.h"

#include "TileSet.h"
#include "TileLayer.h"
#include "TileMap.h"
#include "Structures.h"
#include "Common.h"
#include "MapManager.h"

class MapManager;

class Entity
{
public:
    int width;
    int height;
    Vector2f pos; // virtual pos
    Vector2f apos; //onscreen pos
    ALLEGRO_BITMAP *bitmap;
    ALLEGRO_DISPLAY *display;
    Camera* camera;
    MapManager *mapmanager;
    TileMap* active_map;
    TileLayer* collide_layer;
    int imgdir;
    bool moved;

    Vector2f targetSpeed;
    Vector2f curSpeed;
    Vector2f direction;
    Vector2f acceleration;
    float maxSpeed;
    float terminalSpeed;
    float jumpForce;
    float wallJumpForce;
    int timeSinceJump;
    int maxJumpTime;

    bool grounded;
    bool wallgrounded;

    void set_pos(int newx, int newy);
    void set_pos(Vector2f newpos);

    ~Entity();
    void init(ALLEGRO_DISPLAY *disp, Camera* cam);
    void set_mapmanager(MapManager *mapman);
    void load_image();
    int generate_bitmap();
    void calculate_movement();
    bool get_clip_at(int x, int y);
    void update(int mvkeys[2]);
    void updatealt(int mvkeys[4], bool key[4]);
    void draw();
};
#endif // _ENTITY_H_
