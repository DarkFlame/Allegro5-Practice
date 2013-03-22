#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include "allegro5/allegro.h"
#include "tinyxml.h"
#include "tinystr.h"

#include "TileSet.h"
#include "TileLayer.h"
#include "TileMap.h"
#include "Entity.h"
#include "MapManager.h"
#include "Structures.h"
#include "Common.h"

//--TODO
//  Object collision
//  Camera movement

const float FPS = 60;
const int SCREEN_W = 640;
const int SCREEN_H = 480;
const int SPRITE_W = 23;
const int SPRITE_H = 31;
enum MYKEYS
{
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
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

    //--Create a camera object for everything to use
    fprintf(stderr, "Creating camera\n");
    Camera* camera = new Camera();
    //--Initialize player entity
    player->init(display,camera,sprite_x,sprite_y);
    //player.init(display,sprite_x,sprite_y);
    //if (player->generate_bitmap() == -1) return -1;
    player->set_pos(SCREEN_W/2.0-SPRITE_W/2.0,SCREEN_H/2.0-SPRITE_H/2.0);

    MapManager* mapmanager = new MapManager(camera, player);
    mapmanager->add_map("data/levels/outside.tmx");
    mapmanager->set_active_map("data/levels/outside.tmx");
    mapmanager->reset_camera(0,64);
    player->set_mapmanager(mapmanager);
    //fprintf(stderr, "Tileset for ID %i is %s\n",52,mapmanager->get_active_map()->get_tileset_for_id(19)->name);
    //TileMap *testmap = new TileMap("data/levels/outside.tmx");

    event_queue = al_create_event_queue();
    if(!event_queue)
    {
        fprintf(stderr, "failed to create event_queue!\n");
        delete player;
        delete mapmanager;
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
            mapmanager->update();
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

            mapmanager->drawbg();
            player->draw();
            mapmanager->drawmg();
            mapmanager->drawfg();

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
