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
    active_map = mapmanager->get_active_map();
    collide_layer = active_map->get_layer_for_name("midground");
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
bool Entity::get_clip_at(int x, int y)
{

}
void Entity::update(int mvkeys[4])
{
    TileMap* active_map = mapmanager->get_active_map();
    //--Update function--
    //--Steps X axis, then Y axis after.
    //--Takes keyboard input for both axes and responds accordingly.

    //--First step X axis--
    //--Get the input for left and right keys and set the target speed accordingly
    //--If there is input, then set moved to true
    if (mvkeys[0] == ALLEGRO_KEY_RIGHT)
    {
        targetSpeed.x = +maxSpeed;
        imgdir = 1; //--To flip the image for movement
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
        //--No button pressed, slow down (Or stop if no accel)
        targetSpeed.x = 0;
        moved = false;
    }

    //--Collision--
    //--Check if the player has moved. If not, then skip the rest.
    //--Get the leading edge on the X axis
    //--Iterate through all the horizontal rows of tiles that the bounding box intersects with
    //--Find the closest obstacle in that direction
    //--If the distance between the pos and the closest obstacle is less than the distance between the pos and the pos+Xspeed
    //  then set the pos to the closest obstacle. Otherwise, set it to pos+Xspeed.

    if (moved) //--TODO Make a better check here
    {
        int leadingx;
        //--Get the leading edge
        if (targetSpeed.x < 0)
        {
            //--Moving left. Left edge is leading
            leadingx = pos.x;
        }
        else if (targetSpeed.x > 0)
        {
            //--Moving right. RIght edge is leading
            leadingx = pos.x+SPRITE_W;
        }

        //--Get the index of the top tile that the player intersects with
        //--Get the collision layer from the map
        TileLayer* layer = active_map->get_layer_for_name("midground");
        //--The Y coordinate of the player's top divided by the height of the tiles will return an int of the top tile
        int toptilei = pos.y/active_map->tileheight; //  (fraction is dropped in int division)
        int rowstoiter = SPRITE_H/active_map->tileheight+1; //  How many rows to iterate over, based on the height of the player divided by the tile height
        int closestobst = NULL;
        for (int y=toptilei; y<toptilei+rowstoiter;y++)
        {
            //--This is a single iteration of the Y rows. Now iterate over the X
            //--Find the closest obstacle in this row
            int x = leadingx/active_map->tilewidth;
            while (layer->tiles[x][y] != 0)
            {
                if (x<0||x>active_map->width)
                {
                    //--Hit the edge of the map with no obstacles in this row.
                    break;
                }
                x++;
            }
            fprintf(stderr, "Found an obstacle in row %i at x %i\n",y,x);

            //--Figure out if this obstacle is closer than the previous one already found.
            //--x is the left side of the nearest obstacle in this row.
            if (targetSpeed.x < 0)
            {
                //--Check to see if x is farther right than the previous closestobst
                if (x > closestobst)
                {
                    closestobst = x;
                }
            }
            else if (targetSpeed.x > 0)
            {
                //--Check to see if x is farther left than the previous closestobst
                if (x < closestobst)
                {
                    closestobst = x;
                }
            }

            //--Check to see if targetSpeed.x will overshoot past closestobst.
            //--If so, set targetSpeed.x to 0 and pos.x to closestobst+active_map->tilewidth (If going right, omit the tilewidth addition)
            //--If not, calculate curSpeed.x from targetSpeed.x and add curSpeed.x to pos.x
            bool overshot = false;
            if (targetSpeed.x < 0)
            {
                if (targetSpeed.x+pos.x < closestobst+active_map->tilewidth)
                {
                    targetSpeed.x = 0;
                    pos.x = closestobst+active_map->tilewidth;
                }
            }
            else if (targetSpeed.x > 0)
            {
                if (targetSpeed.x+pos.x > closestobst)
                {
                    targetSpeed.x = 0;
                    pos.x = closestobst;
                }
            }
        }

        //--Collision has been checked. Now calculate curSpeed.x and step pos.x accordingly.
        //--Remember, if we collided, then targetSpeed.x has been reset to 0, so curSpeed.x will also be set to 0
        calculate_movement();
        pos.x = pos.x + curSpeed.x;
        pos.y = pos.y + curSpeed.y;
    }

    //--Example--
    //   0 1 2 3 4 5 6 7 8 9
    //  0[][][][][][][][][][]
    //  1[][][][][]    p
    //  2[][][][][]    l
    //  3[][][][][][]  r
    //  4[][][][][][][][][][]

    //--Explanation--
    //--If plr is player and is moving left, then get the Xcoordinate of the left side of plr first
    //--Then figure out that plr intersects with rows 1,2, and 3 (But not 0 or 4). Iterate through rows 1,2, and 3
    //  and check to see if the tile is an obstacle. Start from the player's leading edge and work in the direction
    //  of movement.
    //--As soon as an obstacle is found, move to the next row
    //--Once the row iteration is complete, Check to see which row has the closest obstacle (row 3 in this case). If they are all the same distance,
    //  just pick the first one or something
    //--Check the speed+pos against the obstacle's coordinate. Whichever one is closer to the (UNMODIFIED) pos,
    //  set pos.x to that coordinate.


    //--Then do the same for the Y axis--

    ////////////////
    //==OLD CODE==//
    ////////////////
    /*
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

    //--Calculate the speed (For acceleration)
    calculate_movement();

    //  Collision
    //--Check for movement first
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

                        fprintf(stderr, "non-empty block at y %i,%i\n",ycheck,curcol);
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
                        }
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
    */
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
