#include <stdio.h>
#include <cmath>
#include <allegro5/allegro.h>
#include "tinyxml.h"
#include "tinystr.h"

#include "tmxloader.cpp"

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
enum MYKEYS {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

int sign(int i){
    if (i>0) return +1;
    else if (i<0) return -1;
    else return 0;
}

class Vector2f{
    public:
    float x;
    float y;

    Vector2f(){
        x = 0;
        y = 0;
    };
    Vector2f(float nx, float ny){
        x = nx;
        y = ny;
    };

    Vector2f operator*(Vector2f param){
        return Vector2f(x*param.x, y*param.y);
    };
    Vector2f operator/(Vector2f param){
        return Vector2f(x/param.x, y/param.y);
    };
    Vector2f operator+(Vector2f param){
        return Vector2f(x+param.x, y+param.y);
    };
    Vector2f operator-(Vector2f param){
        return Vector2f(x-param.x, y-param.y);
    };
    void operator+=(Vector2f param){
        x = x + param.x;
        y = y + param.y;
    };
    void operator-=(Vector2f param){
        x = x - param.x;
        y = y - param.y;
    };

    float angle(Vector2f other){
        return atan2(x-other.x, y-other.y);
    };
    float length(){return sqrt(pow(x,2)+pow(y,2));};
    float lengthSquared(){return pow(x,2)+pow(y,2);};
};

class Entity{
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

    ~Entity(){
        al_destroy_bitmap(bitmap);
    };
    void init(ALLEGRO_DISPLAY *disp, const int x, const int y);
    int generate_bitmap();
    void calculate_movement();
    void update(int mvkeys[2], bool key[4]);
    void updatealt(int mvkeys[4], bool key[4]);
    void draw();
};

void Entity::init(ALLEGRO_DISPLAY *disp, const int x, const int y){
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

int Entity::generate_bitmap(){
    bitmap = al_create_bitmap(SPRITE_W, SPRITE_H);
    if (!bitmap) return -1;
    al_set_target_bitmap(bitmap);
    al_clear_to_color(al_map_rgb(255, 0, 0));
    al_set_target_bitmap(al_get_backbuffer(display));
    return 0;
};
void Entity::calculate_movement(){
    Vector2f direction = Vector2f(sign(targetSpeed.x - curSpeed.x), sign(targetSpeed.y - curSpeed.y));
    curSpeed += acceleration * direction;
    if (sign(targetSpeed.x - curSpeed.x) != direction.x)
        curSpeed.x = targetSpeed.x;
    if (sign(targetSpeed.y - curSpeed.y) != direction.y)
        curSpeed.y = targetSpeed.y;
}
void Entity::updatealt(int mvkeys[2], bool key[4]){
    if (mvkeys[0] == ALLEGRO_KEY_RIGHT){
        targetSpeed.x = +maxSpeed;
        moved = true;
    }
    else if (mvkeys[0] == ALLEGRO_KEY_LEFT){
        targetSpeed.x = -maxSpeed;
        moved = true;
    }
    else {
        targetSpeed.x = 0;
        moved = false;
    }

    if ((curSpeed.x != 0) || (curSpeed.y != 0)){
        if (pos.y+SPRITE_H>=SCREEN_H){
            targetSpeed.y = 0; // Touching the bottom. Collide
            curSpeed.y = 0;
            pos.y = SCREEN_H-SPRITE_H;
            timeSinceJump=0;
            grounded = true;
        }
    }

    if (grounded){
        if (key[KEY_UP]){
            targetSpeed.y = -jumpForce; //If the jump button is pressed AND within jump time
            curSpeed.y = -jumpForce;
            timeSinceJump++;
            grounded = false;
        }
        else {
            targetSpeed.y = 0;
            curSpeed.y = 0;
            timeSinceJump = 0;
        }
    }
    else{
        if (!key[KEY_UP] || timeSinceJump >= maxJumpTime){
            targetSpeed.y = terminalSpeed;
        }
        else {
            timeSinceJump++;
        }
    }

    calculate_movement();

    pos = pos + curSpeed;

    if (pos.x <= 0){
        targetSpeed.x = 0;
        curSpeed.x = 0;
        pos.x = 0;
        //pos.y = pos.y - curSpeed.y/1.25;
        wallgrounded = true;
    }
    else if (pos.x+SPRITE_W>=SCREEN_W){
        targetSpeed.x = 0;
        curSpeed.x = 0;
        pos.x = SCREEN_W-SPRITE_W;
        //pos.y = pos.y - curSpeed.y/1.25;
        wallgrounded = true;
    }
    else wallgrounded = false;
};
void Entity::update(int mvkeys[4], bool key[4]){

    //  Step X axis
    if (mvkeys[0] == ALLEGRO_KEY_RIGHT){
        targetSpeed.x = +maxSpeed;
    }
    else if (mvkeys[0] == ALLEGRO_KEY_LEFT){
        targetSpeed.x = -maxSpeed;
    }
    else{
        targetSpeed.x = 0;
    }

    //  Step Y axis
    if (grounded){
        timeSinceJump = 0;
        if (mvkeys[3] == 1){
            // The player pushed the jump key.
            curSpeed.y = -jumpForce;
            targetSpeed.y = -jumpForce;
            timeSinceJump++;
            grounded = false;
        }
    }
    else{
        if (mvkeys[3] == 0 || timeSinceJump >= maxJumpTime){
            // Either the timer ran out or the key was lifted.
            targetSpeed.y = terminalSpeed;
            timeSinceJump = maxJumpTime+1; // The key was lifted before the timer ended. Set it to the max jump time so they can't jump again until they land.
        }
        else{
            // Jump key is still down and timer not done.
            timeSinceJump++;
            targetSpeed.y = -jumpForce;
        }
    }

    calculate_movement();

    //  Collision
    // Collide with bottom pixel of screen.
    if (pos.y + curSpeed.y >= SCREEN_H-SPRITE_H){
        curSpeed.y = 0;
        targetSpeed.y = 0;
        pos.y = SCREEN_H-SPRITE_H;
        grounded = true;
    }
    // Collide with edges of screen.
    if (pos.x + curSpeed.x <= 0){
        curSpeed.x = 0;
        targetSpeed.x = 0;
        pos.x = 0;
    }
    else if (pos.x+curSpeed.x >= SCREEN_W-SPRITE_W){
        curSpeed.x = 0;
        targetSpeed.x = 0;
        pos.x = SCREEN_W-SPRITE_W;
    }

    // Finally adjust position for drawing.
    pos = pos + curSpeed;
};
void Entity::draw(){
    al_draw_bitmap(bitmap, pos.x, pos.y, 0);
};

class Camera{
public:
    Vector2f offset;
    Vector2f view_size;
};



int main(int argc, char **argv){
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    Entity *player = new Entity();

    float sprite_x = SCREEN_W / 2.0 - SPRITE_W / 2.0;
    float sprite_y = SCREEN_H / 2.0 - SPRITE_H / 2.0;
    bool key[4] = { false, false, false, false };
    int mvkeys[2] = { NULL, NULL };

    TileMap map;

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

    //--Initialize player entity
    player->init(display,sprite_x,sprite_y);
    //player.init(display,sprite_x,sprite_y);
    if (player->generate_bitmap() == -1) return -1;
    player->set_pos(SCREEN_W/2.0-SPRITE_W/2.0,SCREEN_H/2.0-SPRITE_H/2.0);

    map.load_from_file("testmap32.tmx");

    event_queue = al_create_event_queue();
    if(!event_queue) {
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
        if(ev.type == ALLEGRO_EVENT_TIMER) {
            player->update(mvkeys, key);
            //player->updatealt(mvkeys, key);

            redraw = true;
          }
        else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
        //--Store keypress info
        else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            //printf("\n\nKEYCODE: %i\n\n", ev.keyboard.keycode);
            switch(ev.keyboard.keycode) {
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
                    if (mvkeys[0] == NULL){
                        mvkeys[0] = ALLEGRO_KEY_LEFT;
                    }
                    else{
                        //mvkeys[1] = ALLEGRO_KEY_LEFT;
                        mvkeys[1] = mvkeys[0];
                        mvkeys[0] = ALLEGRO_KEY_LEFT;
                    }
                    break;

                case ALLEGRO_KEY_RIGHT:
                    key[KEY_RIGHT] = true;
                    if (mvkeys[0] == NULL){
                        mvkeys[0] = ALLEGRO_KEY_RIGHT;
                    }
                    else{
                        //mvkeys[1] = ALLEGRO_KEY_RIGHT;
                        mvkeys[1] = mvkeys[0];
                        mvkeys[0] = ALLEGRO_KEY_RIGHT;
                    }
                    break;
            }
        }
        else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch(ev.keyboard.keycode) {
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
                    if (mvkeys[0] == ALLEGRO_KEY_LEFT){
                        mvkeys[0] = mvkeys[1];
                        mvkeys[1] = NULL;
                    }
                    else if (mvkeys[1] == ALLEGRO_KEY_LEFT){
                        mvkeys[1] = NULL;
                    }
                    break;

                case ALLEGRO_KEY_RIGHT:
                    key[KEY_RIGHT] = false;
                    if (mvkeys[0] == ALLEGRO_KEY_RIGHT){
                        mvkeys[0] = mvkeys[1];
                        mvkeys[1] = NULL;

                    }
                    else if (mvkeys[1] == ALLEGRO_KEY_RIGHT){
                        mvkeys[1] = NULL;
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
        if(redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;

            al_clear_to_color(al_map_rgb(50,50,50));

            player->draw();

            al_flip_display();
        }
    }

    //--You could let Allegro do this automatically, but it's allegedly faster
    //  if you do it manually
    delete player;
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
