#include <stdio.h>
#include <allegro5/allegro.h>

//--TODO
//  Horizontal acceleration
//  Independent vertical speed
//   Jumping
//   Jump force
//   Variable jump height
//  Object collision

const float FPS = 60;
const int SCREEN_W = 640;
const int SCREEN_H = 480;
const int SPRITE_W = 32;
const int SPRITE_H = 64;
enum MYKEYS {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

class Entity{
    public:
    float gravity;
    float h_speed;
    int width;
    int height;
    int x;
    int y;
    ALLEGRO_BITMAP *bitmap;
    ALLEGRO_DISPLAY *display;

    void set_pos(int newx, int newy){x=newx;y=newy;}

    void init(ALLEGRO_DISPLAY *disp, const int w, const int h);
    int generate_bitmap();
    void update(bool key[4]);
    void draw();
    void destroy(){al_destroy_bitmap(bitmap);}
};

void Entity::init(ALLEGRO_DISPLAY *disp, const int w, const int h){
    width = w;
    height = h;
    display = disp;
    gravity = 4.0;
    h_speed = 6.0;
};
int Entity::generate_bitmap(){
    bitmap = al_create_bitmap(SPRITE_W, SPRITE_H);
    if (!bitmap) return -1;
    al_set_target_bitmap(bitmap);
    al_clear_to_color(al_map_rgb(255, 0, 0));
    al_set_target_bitmap(al_get_backbuffer(display));
    return 0;
};
void Entity::update(bool key[4]){
    if(key[KEY_UP] && y >= h_speed) {
            y -= h_speed;
        }

        if(key[KEY_DOWN] && y <= SCREEN_H - SPRITE_H - h_speed) {
            y += h_speed;
        }

        if(key[KEY_LEFT] && x >= h_speed) {
            x -= h_speed;
        }

        if(key[KEY_RIGHT] && x <= SCREEN_W - SPRITE_W - h_speed) {
            x += h_speed;
        }

        if (y < SCREEN_H - SPRITE_H-3.0) {
            y += gravity;
        }
};
void Entity::draw(){
    al_draw_bitmap(bitmap, x, y, 0);
};

int main(int argc, char **argv)
{
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    Entity player;

    float sprite_x = SCREEN_W / 2.0 - SPRITE_W / 2.0;
    float sprite_y = SCREEN_H / 2.0 - SPRITE_H / 2.0;
    bool key[4] = { false, false, false, false };
    bool redraw = true;
    bool doexit = false;

    if(!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    if(!al_install_keyboard()) {
        fprintf(stderr, "failed to initialize the keyboard!\n");
        return -1;
    }

    //--Calculate how long each frame should last in milliseconds, then create
    //  a timer which ticks at that interval
    timer = al_create_timer(1.0 / FPS);
    if(!timer) {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    /*sprite = al_create_bitmap(SPRITE_W, SPRITE_H);
    if(!sprite) {
        fprintf(stderr, "failed to create sprite bitmap!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    //--Colorize the bitmap as solid red for placeholder purposes
    al_set_target_bitmap(sprite);
    al_clear_to_color(al_map_rgb(255, 0, 0));
    al_set_target_bitmap(al_get_backbuffer(display));*/

    //--Initialize player entity
    player.init(display,sprite_x,sprite_y);
    if (player.generate_bitmap() == -1) return -1;
    player.set_pos(SCREEN_W/2.0-SPRITE_W/2.0,SCREEN_H/2.0-SPRITE_H/2.0);

    event_queue = al_create_event_queue();
    if(!event_queue) {
        fprintf(stderr, "failed to create event_queue!\n");
        player.destroy();
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
        if(ev.type == ALLEGRO_EVENT_TIMER) {
            player.update(key);

            redraw = true;
          }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
        //--Store keypress info
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_W:
                    key[KEY_UP] = true;
                    break;

                case ALLEGRO_KEY_S:
                    key[KEY_DOWN] = true;
                    break;

                case ALLEGRO_KEY_A:
                    key[KEY_LEFT] = true;
                    break;

                case ALLEGRO_KEY_D:
                    key[KEY_RIGHT] = true;
                    break;
            }
        }
        else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_W:
                    key[KEY_UP] = false;
                    break;

                case ALLEGRO_KEY_S:
                    key[KEY_DOWN] = false;
                    break;

                case ALLEGRO_KEY_A:
                    key[KEY_LEFT] = false;
                    break;

                case ALLEGRO_KEY_D:
                    key[KEY_RIGHT] = false;
                    break;

                case ALLEGRO_KEY_ESCAPE:
                    doexit = true;
                    break;
            }
        }

        //--The screen has updated and needs to be redrawn
        //--Draw Logic--//
        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;

            al_clear_to_color(al_map_rgb(0,0,0));

            player.draw();

            al_flip_display();
        }
    }

    //--You could let Allegro do this automatically, but it's allegedly faster
    //  if you do it manually
    player.destroy();
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
