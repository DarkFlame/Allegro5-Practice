#include "Entity.h"
#include "Structures.h"

void Entity::init(ALLEGRO_DISPLAY *disp, Camera* cam)
{
    display = disp;
    moved = false;

    pos = Vector2f(0.0f,0.0f);
    apos = pos;
    camera = cam;

    targetSpeed = Vector2f(0.0f,0.0f);
    curSpeed = Vector2f(0.0f,0.0f);
    direction = Vector2f(1.0f,1.0f);
    imgdir = 1;
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

    //load_image();
    generate_bitmap();
}
Entity::~Entity()
{

}

void Entity::set_mapmanager(MapManager *mapman)
{
    mapmanager = mapman;
}

void Entity::load_image()
{
    const char * filename = "data/images/playersprite.png";
    ALLEGRO_COLOR colorkey = al_map_rgb(255,0,255);
    bitmap = al_load_bitmap(filename);
    al_convert_mask_to_alpha(bitmap, colorkey);
}
int Entity::generate_bitmap()
{
    bitmap = al_create_bitmap(SPRITE_W, SPRITE_H);
    if (!bitmap) return -1;
    al_set_target_bitmap(bitmap);
    al_clear_to_color(al_map_rgb(255, 255, 255));
    al_set_target_bitmap(al_get_backbuffer(display));
    return 0;
}
void Entity::calculate_movement()
{
    direction = Vector2f(sign(targetSpeed.x - curSpeed.x), sign(targetSpeed.y - curSpeed.y));
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
}
void Entity::update(int mvkeys[4])
{
    //  Step X axis
    if (mvkeys[0] == ALLEGRO_KEY_RIGHT)
    {
        targetSpeed.x = +maxSpeed;
        imgdir = 1;
        moved = true;
    }
    else if (mvkeys[0] == ALLEGRO_KEY_LEFT)
    {
        targetSpeed.x = -maxSpeed;
        imgdir = -1;
        moved = true;
    }
    else
    {
        targetSpeed.x = 0;
        moved = false;
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
        moved = true;
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
    if (curSpeed.x!=0 || curSpeed.y!=0)
    {
        //fprintf(stderr, "Checking for collision. XDirection:%f\n",direction.x);
        TileMap *active_map = mapmanager->get_active_map();
        //--Get the sprite's pos+speed / tilesize

        //--How to get the row/column:
        //  pos.x+curSpeed.x)/active_map->tilewidth
        //  pos.y+curSpeed.y)/active_map->tileheight

        //--Cheack each side of the sprite
        //--Check forward-facing side (X)
        if (curSpeed.x < 0)
        {
            //--Facing left. Check that side
            int lowrow = (pos.y+curSpeed.y)/active_map->tileheight;
            int hirow = (pos.y+curSpeed.y+SPRITE_H)/active_map->tileheight;
            int currow = lowrow;
            while (currow <= hirow)
            {
                //--Iterate over each row and see if we collide.
                TileLayer *layer = active_map->get_layer_for_name("midground");
                int xtilepos = pos.x/active_map->tilewidth;
                bool solid = false;
                int xcheck = xtilepos;
                while (!solid)
                {
                    //fprintf(stderr, "Checking another tile  %i\n",xcheck);
                    if (layer->tiles[xcheck][currow] != 0)
                    {
                        //fprintf(stderr, "Found non-empty block at %i,%i\n",xcheck,currow);
                        //--Found a non-empty block. Must be solid. Collide.
                        if (pos.x+curSpeed.x < xcheck*active_map->tilewidth)
                        {
                            fprintf(stderr, "\n\ncolliding with non-empty  block\nxPos: %f xcheck: %i newxPos: %i\n",pos.x,xcheck, xcheck*active_map->tilewidth);
                            //-- The block is within the movement. Collide.
                            pos.x = xcheck*active_map->tilewidth;
                            fprintf(stderr, "Newpos: %f\n",pos.x);
                            apos.x = pos.x;
                            curSpeed.x = 0;
                            targetSpeed.x = 0;
                        }
                        else
                        {
                            //--The first block is too far away. Stop checking in this direction
                            break;
                        }
                    }

                    xcheck--;
                    if (xcheck <= 0)
                    {
                        //--Nothing to collide with
                        break;
                    }
                }
                currow++;
            }
        }
        else if (curSpeed.x > 0)
        {
            //--Facing right. Check that side
            int lowrow = (pos.y+curSpeed.y)/active_map->tileheight;
            int hirow = (pos.y+curSpeed.y+SPRITE_H)/active_map->tileheight;
        }

        //--Check forward-facing side (Y)
        if (curSpeed.y < 0)
        {
            //--Going up. Check the top
            int lowcolumn = (pos.x+curSpeed.x)/active_map->tilewidth;
            int hicolumn = (pos.x+curSpeed.x+SPRITE_W)/active_map->tilewidth;
        }
        //--Steps to check bottom collision:
        //1.Find the X columns the entity occupies
        //2.Iterate over those X columns
        //3. Starting at the player's Y tile, iterate over the Y tiles in a downward order
        //4. If a tile is solid, check to see if the player collides with it (step5), ortherwise, continue
        //5.  If it's solid, check the player's bottom Y coordinate against the tile's top Y coordinate.
        //      If the player's Y bottom coordinate > tile's Y top coordinate, collide.
        //6.  Adjust motion and position accordingly
        else if (curSpeed.y > 0)
        {
            //--Going down. Check the bottom
            //--Find the X columns the Entity occupies
            int lowcolumn = (pos.y+curSpeed.y)/active_map->tileheight;
            int hicolumn = (pos.y+curSpeed.y+SPRITE_H)/active_map->tileheight;
            int curcol = lowcolumn;

            //--Iterate over those occupied X columns
            while (curcol <= hicolumn)
            {
                //--Iterate over each row and see if we collide.
                TileLayer *layer = active_map->get_layer_for_name("midground");
                int ytilepos = pos.y/active_map->tileheight; //Player's top Y tile
                bool solid = false;
                int ycheck = ytilepos;

                //--Starting at the player's top Y tile, iterate over the Y tiles in a downward order
                while (!solid)
                {
                    fprintf(stderr, "\nChecking another tile y  %i\n",ycheck);
                    //--If the tile is solid...
                    if (layer->tiles[curcol][ycheck] != 0)
                    {
                        //--Check the player's bottom Y coordinate against the tile's top Y coordinate.
                        if (pos.y+curSpeed.y+SPRITE_H > ycheck*active_map->tileheight)
                        {
                            //--It did collide.
                            grounded = true; //--We hit the ground. So enable jumping
                            pos.y = (ycheck*active_map->tileheight)-SPRITE_H; //--The player's bottom = tile's top y
                            curSpeed.y = 0; //--Stop movement
                            targetSpeed.y = 0;
                        }

                        /*fprintf(stderr, "non-empty block at y %i,%i\n",ycheck,curcol);
                        //--Found a non-empty block. Must be solid. Check collide.
                        fprintf(stderr, "%f  >  %i  ?\n",pos.y+curSpeed.y+SPRITE_H,ycheck*active_map->tileheight);
                        if (pos.y+curSpeed.y+SPRITE_H > ycheck*active_map->tileheight)
                        {
                            fprintf(stderr, "yes\n");
                            fprintf(stderr, "yPos: %f newyPos: %i\n",pos.y, ycheck*active_map->tileheight);
                            //-- The block is within the movement. Collide.
                            pos.y = ycheck*active_map->tileheight;
                            fprintf(stderr, "Newpos: %f\n",pos.y);
                            apos.y = pos.y;
                            curSpeed.y = 0;
                            targetSpeed.y = 0;
                            ytilepos = pos.y/active_map->tileheight;
                            grounded = true;
                        }
                        else
                        {
                            fprintf(stderr, "no\n");
                            //--The first block is too far away. Stop checking in this direction
                            break;
                        }*/
                    }

                    ycheck++;
                    if (ycheck > layer->height)
                    {
                        //--Nothing to collide with
                        break;
                    }
                }
                curcol++;
            }
        }

        //======================================
        //--Collide with bottom pixel of screen.
        //======================================
        if (pos.y + curSpeed.y >= SCREEN_H-SPRITE_H)
        //if (pos.y + curSpeed.y >= 24*32)
        {
            curSpeed.y = 0;
            targetSpeed.y = 0;
            pos.y = SCREEN_H-SPRITE_H;
            //pos.y = 24*32;
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
        //else if (pos.x+curSpeed.x >= 64*32)
        {
            curSpeed.x = 0;
            targetSpeed.x = 0;
            pos.x = SCREEN_W-SPRITE_W;
            //pos.x = 64*32;
        }
    }


    //--Finally adjust position for drawing.
    pos = pos + curSpeed;
    //--Camera stuff. IGNORE BROKEN
    if (camera->stopped)
    {
        apos = apos + curSpeed;
        if (apos.y + curSpeed.y >= SCREEN_H-SPRITE_H)
        {
            apos.y = SCREEN_H-SPRITE_H;
        }
        if (apos.x + curSpeed.x <= 0)
        {
            apos.x = 0;
        }
        else if (apos.x+curSpeed.x >= SCREEN_W-SPRITE_W)
        {
            apos.x = SCREEN_W-SPRITE_W;
        }

    }
    else
    {
        apos.x = SCREEN_W/2-SPRITE_W/2;
        apos.y = SCREEN_H/2-SPRITE_H/2;
    }
    apos.x = pos.x+camera->x;
    apos.y = pos.y+camera->y;
    //apos = pos;
    //camera->x = pos.x; camera->y = pos.y;
}
void Entity::draw()
{
    if (imgdir < 0)
    {
        al_draw_bitmap(bitmap, apos.x, apos.y, ALLEGRO_FLIP_HORIZONTAL);
    }
    else
    {
        al_draw_bitmap(bitmap, apos.x, apos.y, 0);
    }
}

void Entity::set_pos(int newx, int newy)
{
    pos.x = newx;
    pos.y = newy;
}
