#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include "allegro5/allegro.h"
#include "tinyxml.h"
#include "tinystr.h"

#include "tmxloader.cpp"

#include "Common.h"

//--TODO
//  Map importing
//  Map blitting
//  Object collision
//  Camera movement

const float FPS = 60;
const int SCREEN_W = 640;
const int SCREEN_H = 480;
const int SPRITE_W = 25;
const int SPRITE_H = 52;
enum MYKEYS
{
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

int sign(int i)
{
    if (i>0) return +1;
    else if (i<0) return -1;
    else return 0;
}

class Vector2f
{
    public:
    float x;
    float y;

    Vector2f()
    {
        x = 0;
        y = 0;
    };
    Vector2f(float nx, float ny)
    {
        x = nx;
        y = ny;
    };

    Vector2f operator*(Vector2f param)
    {
        return Vector2f(x*param.x, y*param.y);
    };
    Vector2f operator/(Vector2f param)
    {
        return Vector2f(x/param.x, y/param.y);
    };
    Vector2f operator+(Vector2f param)
    {
        return Vector2f(x+param.x, y+param.y);
    };
    Vector2f operator-(Vector2f param)
    {
        return Vector2f(x-param.x, y-param.y);
    };
    void operator+=(Vector2f param)
    {
        x = x + param.x;
        y = y + param.y;
    };
    void operator-=(Vector2f param)
    {
        x = x - param.x;
        y = y - param.y;
    };

    float angle(Vector2f other)
    {
        return atan2(x-other.x, y-other.y);
    };
    float length(){return sqrt(pow(x,2)+pow(y,2));};
    float lengthSquared(){return pow(x,2)+pow(y,2);};
};

class Entity
{
public:
    int width;
    int height;
    Vector2f pos;
    ALLEGRO_BITMAP *bitmap;
    ALLEGRO_DISPLAY *display;

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
    bool moved;

    void set_pos(int newx, int newy){pos.x = newx; pos.y = newy;}
    void set_pos(Vector2f newpos){pos=newpos;}

    ~Entity()
    {
        al_destroy_bitmap(bitmap);
    };
    void init(ALLEGRO_DISPLAY *disp, const int x, const int y);
    int generate_bitmap();
    void calculate_movement();
    void update(int mvkeys[2], bool key[4]);
    void updatealt(int mvkeys[4], bool key[4]);
    void draw();
};

void Entity::init(ALLEGRO_DISPLAY *disp, const int x, const int y)
{
    display = disp;

    pos = Vector2f(0.0f,0.0f);

    targetSpeed = Vector2f(0.0f,0.0f);
    curSpeed = Vector2f(0.0f,0.0f);
    direction = Vector2f(1.0f,1.0f);
    //acceleration = Vector2f(0.35f,0.5f);

    maxSpeed = 4.0f;
    terminalSpeed=10.5f;
    jumpForce=11.f;
    wallJumpForce=6.f;
    timeSinceJump=0;
    maxJumpTime=10;

    grounded = false;
    wallgrounded = false;
    moved = true;

    //acceleration = Vector2f(maxSpeed,terminalSpeed); //disable acceleration
    acceleration = Vector2f(maxSpeed,0.85f); //disable acceleration with gravity
};

int Entity::generate_bitmap()
{
    bitmap = al_create_bitmap(SPRITE_W, SPRITE_H);
    if (!bitmap) return -1;
    al_set_target_bitmap(bitmap);
    al_clear_to_color(al_map_rgb(255, 0, 0));
    al_set_target_bitmap(al_get_backbuffer(display));
    return 0;
};
void Entity::calculate_movement()
{
    Vector2f direction = Vector2f(sign(targetSpeed.x - curSpeed.x), sign(targetSpeed.y - curSpeed.y));
    curSpeed += acceleration * direction;
    if (sign(targetSpeed.x - curSpeed.x) != direction.x)
        curSpeed.x = targetSpeed.x;
    if (sign(targetSpeed.y - curSpeed.y) != direction.y)
        curSpeed.y = targetSpeed.y;
}
void Entity::updatealt(int mvkeys[2], bool key[4])
{
    if (mvkeys[0] == ALLEGRO_KEY_RIGHT)
    {
        targetSpeed.x = +maxSpeed;
        moved = true;
    }
    else if (mvkeys[0] == ALLEGRO_KEY_LEFT)
    {
        targetSpeed.x = -maxSpeed;
        moved = true;
    }
    else
    {
        targetSpeed.x = 0;
        moved = false;
    }

    if ((curSpeed.x != 0) || (curSpeed.y != 0))
    {
        if (pos.y+SPRITE_H>=SCREEN_H){
            targetSpeed.y = 0; // Touching the bottom. Collide
            curSpeed.y = 0;
            pos.y = SCREEN_H-SPRITE_H;
            timeSinceJump=0;
            grounded = true;
        }
    }

    if (grounded)
    {
        if (key[KEY_UP]){
            targetSpeed.y = -jumpForce; //If the jump button is pressed AND within jump time
            curSpeed.y = -jumpForce;
            timeSinceJump++;
            grounded = false;
        }
        else
        {
            targetSpeed.y = 0;
            curSpeed.y = 0;
            timeSinceJump = 0;
        }
    }
    else
    {
        if (!key[KEY_UP] || timeSinceJump >= maxJumpTime)
        {
            targetSpeed.y = terminalSpeed;
        }
        else
        {
            timeSinceJump++;
        }
    }

    calculate_movement();

    pos = pos + curSpeed;

    if (pos.x <= 0)
    {
        targetSpeed.x = 0;
        curSpeed.x = 0;
        pos.x = 0;
        //pos.y = pos.y - curSpeed.y/1.25;
        wallgrounded = true;
    }
    else if (pos.x+SPRITE_W>=SCREEN_W)
    {
        targetSpeed.x = 0;
        curSpeed.x = 0;
        pos.x = SCREEN_W-SPRITE_W;
        //pos.y = pos.y - curSpeed.y/1.25;
        wallgrounded = true;
    }
    else wallgrounded = false;
};
void Entity::update(int mvkeys[4], bool key[4])
{

    //  Step X axis
    if (mvkeys[0] == ALLEGRO_KEY_RIGHT)
    {
        targetSpeed.x = +maxSpeed;
    }
    else if (mvkeys[0] == ALLEGRO_KEY_LEFT)
    {
        targetSpeed.x = -maxSpeed;
    }
    else
    {
        targetSpeed.x = 0;
    }

    //  Step Y axis
    if (grounded)
    {
        timeSinceJump = 0;
        if (mvkeys[3] == 1)
        {
            // The player pushed the jump key.
            curSpeed.y = -jumpForce;
            targetSpeed.y = -jumpForce;
            timeSinceJump++;
            grounded = false;
        }
    }
    else
    {
        if (mvkeys[3] == 0 || timeSinceJump >= maxJumpTime)
        {
            // Either the timer ran out or the key was lifted.
            targetSpeed.y = terminalSpeed;
            timeSinceJump = maxJumpTime+1; // The key was lifted before the timer ended. Set it to the max jump time so they can't jump again until they land.
        }
        else
        {
            // Jump key is still down and timer not done.
            timeSinceJump++;
            targetSpeed.y = -jumpForce;
        }
    }

    calculate_movement();

    //  Collision
    // Collide with bottom pixel of screen.
    if (pos.y + curSpeed.y >= SCREEN_H-SPRITE_H)
    {
        curSpeed.y = 0;
        targetSpeed.y = 0;
        pos.y = SCREEN_H-SPRITE_H;
        grounded = true;
    }
    // Collide with edges of screen.
    if (pos.x + curSpeed.x <= 0)
    {
        curSpeed.x = 0;
        targetSpeed.x = 0;
        pos.x = 0;
    }
    else if (pos.x+curSpeed.x >= SCREEN_W-SPRITE_W)
    {
        curSpeed.x = 0;
        targetSpeed.x = 0;
        pos.x = SCREEN_W-SPRITE_W;
    }

    // Finally adjust position for drawing.
    pos = pos + curSpeed;
};
void Entity::draw()
{
    al_draw_bitmap(bitmap, pos.x, pos.y, 0);
};


struct Camera
{
    int x,y;
};

class MapManager
{
private:
    Camera camera;
    TileMap * maps[64]; //Static array. REALLOC IF NEEDED
    int total_maps;
    TileSet * tilesets[256]; //Static array. REALLOC IF NEEDED
    int total_tilesets;
    TileMap * active_map;

    void log(const char* instring, ...)
    {
        //--Logs instring to stderr with some default formatting
        char buffer[256];

        fprintf(stderr,"[MapManager] ");

        va_list args;
        va_start (args, instring);
        vsprintf(buffer, instring, args);
        fprintf(stderr,buffer);
        va_end (args);
        fprintf(stderr,"\n");
    }

    void load_map(TileMap* map_to_load)
    {
        //--Loads the map's tilesets (If not already)
        log("Loading map '%s'",map_to_load->filename);
    }

public:
    MapManager()
    {
        //--Constructor
        for (int i=0;i<63;i++)
        {
            maps[i] = NULL;
        }
        for (int i=0;i<255;i++)
        {
            tilesets[i] = NULL;
        }
        total_maps = 0;
        total_tilesets = 0;
        reset_camera();
    }
    ~MapManager()
    {
        //--Destroys all maps at the end of the program
        log("Destructor beginning");
        for (int i=0;i<63;i++)
        {
            if (maps[i] != NULL)
            {
                log("Deleting map '%s'",maps[i]->filename);
                delete maps[i];
            }
        }
        /*log("Deleting maps array");
        delete maps;*/
        log("Destructor completed");
    }

    void reset_camera(int x=0, int y=0)
    {
        //--Resets the camera to position x,y
        log("Reset the camera to (%i,%i)",x,y);
        camera.x=x;
        camera.y=y;
    }
    void add_map(const char * filename)
    {
        //--Adds map_to_add to the maps to manage
        maps[total_maps] = new TileMap(filename);
        log("Added new map to Map Manager '%s'",maps[total_maps]->filename);
        load_map(maps[total_maps]);
        total_maps++;
    }
    void reload_map(const char * filename)
    {
        //--Reloads all data related to the map at filename
        log("Reloading map '%s'",filename);
        load_map(get_map(filename));
    }
    TileMap * get_map(const char * mapname)
    {
        //--Iterates over maps and returns the first TileMap with the name mapname
        int i = 0;
        int result = strncmp(mapname,maps[i]->filename, 256);
        if (result==0)
        {
            return maps[i];
        }
        while (result != 0)
        {
            i++;
            if (i>=total_maps)
            {
                return NULL;
            }
            result = strncmp(mapname,maps[i]->filename, 256);
        }
        return maps[i-1];
    }
    void set_active_map(const char * mapname)
    {
        //--Switches maps to the map of name mapname
        //active_map = mapname;
        TileMap* mapbuf = get_map(mapname);
        if (!mapbuf)
        {
            log("Map '%s' does not exist. Could not set active.",mapname);
            return;
        }
        active_map = mapbuf;
        log("Set active map to '%s'",mapname);
        reset_camera();
    }

    void draw()
    {
        //--Draws the active map to target
        //--DRAWING THE TILESET FOR PLACEHOLDER
        al_draw_bitmap(active_map->tilesets[0]->image,0,0,0);
        //--For each layer in the current map
        for (int layerindex=0;layerindex<active_map->numlayers;layerindex++)
        {
            log("Iterating over a layer");
            //--For each row in the current layer on the current map
            for (int y=0;y<active_map->tilelayers[layerindex]->height;y++)
            {
                log("Starting y iteration");
                for (int x=0;x<active_map->tilelayers[layerindex]->width;x++)
                {
                    log("Starting x iteration");
                    //--ID is active_map->tilelayers[layerindex]->tiles[x][y]
                    TileLayer * tlbuf = active_map->tilelayers[layerindex];
                    log("Got tilelayer %s",tlbuf->name);
                    log("Getting tile at (%i,%i)",x,y);
                    int id = tlbuf->tiles[x][y];
                    log("Getting id for %i",id);
                    TileSet * tsbuf = active_map->get_tileset_for_id(id);
                    log("Got tileset %s",tsbuf->name);
                    //--Get the location of the tile on the tileset image
                    int toffset = active_map->tilelayers[layerindex]->tiles[x][y] - tsbuf->firstgid;
                    log("toffset is %i",toffset);
                    int sx,sy;
                    //--Wrap toffset on the x
                    if (toffset < tsbuf->imgwidth/tsbuf->tilewidth)
                    {
                        //--It's in the first row, this is easy.
                        sx = toffset*tsbuf->tilewidth;
                        sy = 0;
                    }
                    else
                    {
                        int times = 0;
                        while (toffset > tsbuf->imgwidth/tsbuf->tilewidth)
                        {
                            toffset -= tsbuf->imgwidth/tsbuf->tilewidth;
                            times++;
                        }
                        sx = toffset;
                        sy = tsbuf->tileheight*times;
                    }
                    al_draw_bitmap_region(tsbuf->image, sx,sy, tsbuf->tilewidth,tsbuf->tileheight, x*active_map->tilewidth,y*active_map->tileheight, 0);
                }
            }
        }
    }
    void update()
    {
        //--Possibly unneeded. Calls game logic for each map
        //  might have another function for the manager itself such
        //  as camera movement.
    }
};

int main(int argc, char **argv)
{
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    Entity *player = new Entity();

    float sprite_x = SCREEN_W / 2.0 - SPRITE_W / 2.0;
    float sprite_y = SCREEN_H / 2.0 - SPRITE_H / 2.0;
    bool key[4] = { false, false, false, false };
    int mvkeys[2] = { 0, 0 };

    bool redraw = true;
    bool doexit = false;

    if(!al_init())
    {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    if(!al_init_image_addon())
    {
        fprintf(stderr, "failed to initialize image addon!\n");
        return -1;
    }

    if(!al_install_keyboard())
    {
        fprintf(stderr, "failed to initialize the keyboard!\n");
        return -1;
    }

    //--Calculate how long each frame should last in milliseconds, then create
    //  a timer which ticks at that interval
    timer = al_create_timer(1.0 / FPS);
    if(!timer)
    {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display)
    {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    //--Initialize player entity
    player->init(display,sprite_x,sprite_y);
    //player.init(display,sprite_x,sprite_y);
    if (player->generate_bitmap() == -1) return -1;
    player->set_pos(SCREEN_W/2.0-SPRITE_W/2.0,SCREEN_H/2.0-SPRITE_H/2.0);

    MapManager* mapmanager = new MapManager();
    mapmanager->add_map("data/levels/outside.tmx");
    mapmanager->set_active_map("data/levels/outside.tmx");
    //TileMap *testmap = new TileMap("data/levels/outside.tmx");

    event_queue = al_create_event_queue();
    if(!event_queue)
    {
        fprintf(stderr, "failed to create event_queue!\n");
        delete player;
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(0,0,0));

    al_flip_display();

    al_start_timer(timer);

    while(!doexit)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        //--Game Logic--//
        if(ev.type == ALLEGRO_EVENT_TIMER)
        {
            player->update(mvkeys, key);
            //player->updatealt(mvkeys, key);

            redraw = true;
          }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break;
        }
        //--Store keypress info
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            //printf("\n\nKEYCODE: %i\n\n", ev.keyboard.keycode);
            switch(ev.keyboard.keycode)
            {
                case ALLEGRO_KEY_UP:
                    key[KEY_UP] = true;
                    mvkeys[3] = 1;
                    break;

                case ALLEGRO_KEY_DOWN:
                    key[KEY_DOWN] = true;
                    mvkeys[4] = 1;
                    break;

                case ALLEGRO_KEY_LEFT:
                    key[KEY_LEFT] = true;
                    if (!mvkeys[0])
                    {
                        mvkeys[0] = ALLEGRO_KEY_LEFT;
                    }
                    else
                    {
                        //mvkeys[1] = ALLEGRO_KEY_LEFT;
                        mvkeys[1] = mvkeys[0];
                        mvkeys[0] = ALLEGRO_KEY_LEFT;
                    }
                    break;

                case ALLEGRO_KEY_RIGHT:
                    key[KEY_RIGHT] = true;
                    if (!mvkeys[0])
                    {
                        mvkeys[0] = ALLEGRO_KEY_RIGHT;
                    }
                    else
                    {
                        //mvkeys[1] = ALLEGRO_KEY_RIGHT;
                        mvkeys[1] = mvkeys[0];
                        mvkeys[0] = ALLEGRO_KEY_RIGHT;
                    }
                    break;
            }
        }
        else if(ev.type == ALLEGRO_EVENT_KEY_UP)
        {
            switch(ev.keyboard.keycode)
            {
                case ALLEGRO_KEY_UP:
                    key[KEY_UP] = false;
                    mvkeys[3] = 0;
                    break;

                case ALLEGRO_KEY_DOWN:
                    key[KEY_DOWN] = false;
                    mvkeys[4] = 0;
                    break;

                case ALLEGRO_KEY_LEFT:
                    key[KEY_LEFT] = false;
                    if (mvkeys[0] == ALLEGRO_KEY_LEFT)
                    {
                        mvkeys[0] = mvkeys[1];
                        mvkeys[1] = 0;
                    }
                    else if (mvkeys[1] == ALLEGRO_KEY_LEFT)
                    {
                        mvkeys[1] = 0;
                    }
                    break;

                case ALLEGRO_KEY_RIGHT:
                    key[KEY_RIGHT] = false;
                    if (mvkeys[0] == ALLEGRO_KEY_RIGHT)
                    {
                        mvkeys[0] = mvkeys[1];
                        mvkeys[1] = 0;

                    }
                    else if (mvkeys[1] == ALLEGRO_KEY_RIGHT)
                    {
                        mvkeys[1] = 0;
                    }
                    break;

                case ALLEGRO_KEY_ESCAPE:
                    doexit = true;
                    break;
            }
        }

        //printf("\n[%i, %i]\n\n", mvkeys[0], mvkeys[1]);

        //--The screen has updated and needs to be redrawn
        //--Draw Logic--//
        if(redraw && al_is_event_queue_empty(event_queue))
        {
            redraw = false;

            al_clear_to_color(al_map_rgb(50,50,50));

            player->draw();
            mapmanager->draw();

            al_flip_display();
        }
    }

    //--You could let Allegro do this automatically, but it's allegedly faster
    //  if you do it manually
    delete player;
    //delete testmap;
    delete mapmanager;
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
