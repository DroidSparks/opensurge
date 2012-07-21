/*
 * Open Surge Engine
 * brick.c - brick module
 * Copyright (C) 2008-2010, 2012  Alexandre Martins <alemartf(at)gmail(dot)com>
 * http://opensnc.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <math.h>
#include "brick.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "actor.h"
#include "collisionmask.h"
#include "../scenes/level.h"
#include "../core/global.h"
#include "../core/video.h"
#include "../core/stringutil.h"
#include "../core/logfile.h"
#include "../core/osspec.h"
#include "../core/util.h"
#include "../core/timer.h"
#include "../core/audio.h"
#include "../core/soundfactory.h"
#include "../core/nanoparser/nanoparser.h"


/* private data */
#define BRKDATA_MAX                 16384 /* this engine supports up to BRKDATA_MAX bricks per theme */
static int brickdata_count; /* size of brickdata[] */
static brickdata_t* brickdata[BRKDATA_MAX]; /* brick data */

/* private functions */
static void brick_animate(brick_t *brk);
static brickdata_t* brickdata_new();
static brickdata_t* brickdata_delete(brickdata_t *obj);
static void validate_brickdata(const brickdata_t *obj);
static int traverse(const parsetree_statement_t *stmt);
static int traverse_brick_attributes(const parsetree_statement_t *stmt, void *brickdata);

/* misc */
#define BRB_FALL_TIME               1.0 /* time in seconds before a BRB_FALL gets destroyed */


/* public functions */




/* ========== brick theme interface ================ */

/*
 * brickdata_load()
 * Loads all the brick theme from a file
 */
void brickdata_load(const char *filename)
{
    int i;
    char abs_path[1024];
    parsetree_program_t *tree;

    logfile_message("brickdata_load('%s')", filename);
    resource_filepath(abs_path, filename, sizeof(abs_path), RESFP_READ);

    brickdata_count = 0;
    for(i=0; i<BRKDATA_MAX; i++) 
        brickdata[i] = NULL;

    tree = nanoparser_construct_tree(abs_path);
    nanoparser_traverse_program(tree, traverse);
    tree = nanoparser_deconstruct_tree(tree);

    if(brickdata_count == 0)
        fatal_error("FATAL ERROR: no bricks have been defined in \"%s\"", filename);

    logfile_message("Creating collision masks...");
    for(i=0; i<brickdata_count; i++) {
        if(brickdata[i] != NULL && brickdata[i]->collisionmask == NULL)
            brickdata[i]->collisionmask = collisionmask_create_from_sprite(brickdata[i]->data);
    }

    logfile_message("brickdata_load('%s') ok!", filename);
}



/*
 * brickdata_unload()
 * Unloads brick data
 */
void brickdata_unload()
{
    int i;

    logfile_message("brickdata_unload()");

    for(i=0; i<brickdata_count; i++)
        brickdata[i] = brickdata_delete(brickdata[i]);
    brickdata_count = 0;

    logfile_message("brickdata_unload() ok");
}


/*
 * brickdata_get()
 * Gets a brickdata_t* object
 */
brickdata_t *brickdata_get(int id)
{
    id = clip(id, 0, brickdata_count-1);
    return brickdata[id];
}


/*
 * brickdata_size()
 * How many bricks are loaded?
 */
int brickdata_size()
{
    return brickdata_count;
}




/* ========= brick interface ============== */

/*
 * brick_create()
 * Spawns a new brick
 */
brick_t* brick_create(int id)
{
    brick_t *b = mallocx(sizeof *b);
    int i;

    b->brick_ref = brickdata_get(id);
    b->animation_frame = 0;
    b->enabled = TRUE;
    b->state = BRS_IDLE;
    b->layer = BRL_DEFAULT;

    for(i=0; i<BRICK_MAXVALUES; i++)
        b->value[i] = 0.0f;

    return b;
}


/*
 * brick_destroy()
 * Destroys an existing brick instace
 */
brick_t* brick_destroy(brick_t *brk)
{
    free(brk);
    return NULL;
}


/*
 * brick_update()
 * Updates a brick
 */
void brick_update(brick_t *brk, player_t** team, int team_size, brick_list_t *brick_list, item_list_t *item_list, enemy_list_t *enemy_list)
{
    int i;

    if((brk == NULL) || (brk->brick_ref == NULL))
        return;

    switch(brk->brick_ref->behavior) {
        /* breakable bricks */
        case BRB_BREAKABLE: {
            int brkw = image_width(brk->brick_ref->image);
            int brkh = image_height(brk->brick_ref->image);
            float a[4], b[4] = { brk->x, brk->y, brk->x + brkw, brk->y + brkh };

            for(i=0; i<team_size; i++) {
                a[0] = team[i]->actor->position.x - team[i]->actor->hot_spot.x - 3;
                a[1] = team[i]->actor->position.y - team[i]->actor->hot_spot.y - 3;
                a[2] = a[0] + image_width(actor_image(team[i]->actor)) + 6;
                a[3] = a[1] + image_height(actor_image(team[i]->actor)) + 6;

                if((team[i]->attacking || player_is_rolling(team[i])) && bounding_box(a,b)) {
                    /* particles */
                    int bi, bj, bh, bw;
                    bw = max(brk->brick_ref->behavior_arg[0], 1);
                    bh = max(brk->brick_ref->behavior_arg[1], 1);
                    for(bi=0; bi<bw; bi++) {
                        for(bj=0; bj<bh; bj++) {
                            v2d_t brkpos = v2d_new(brk->x + (bi*brkw)/bw, brk->y + (bj*brkh)/bh);
                            v2d_t brkspeed = v2d_new(-team[i]->actor->speed.x*0.3, -100-random(50));
                            image_t *brkimg = image_create(brkw/bw, brkh/bh);

                            if(fabs(brkspeed.x) > EPSILON)
                                brkspeed.x += (brkspeed.x>0?1:-1) * random(50);

                            image_blit(brk->brick_ref->image, brkimg, (bi*brkw)/bw, (bj*brkh)/bh, 0, 0, brkw/bw, brkh/bh);
                            level_create_particle(brkimg, brkpos, brkspeed, FALSE);
                        }
                    }

                    /* bye bye, brick! */
                    sound_play( soundfactory_get("break") );
                    brk->state = BRS_DEAD;
                }
            }
            break;
        }

        /* falling bricks */
        case BRB_FALL: {
            int i;
            int brkw = image_width(brk->brick_ref->image);
            int brkh = image_height(brk->brick_ref->image);
            float a[4], b[4] = { brk->x, brk->y, brk->x + brkw, brk->y + brkh/2 };
            int bb = FALSE;

            for(i=0; i<team_size; i++) {
                a[0] = team[i]->actor->position.x - team[i]->actor->hot_spot.x - 3;
                a[1] = team[i]->actor->position.y - team[i]->actor->hot_spot.y + image_height(actor_image(team[i]->actor))/2;
                a[2] = a[0] + image_width(actor_image(team[i]->actor)) + 6;
                a[3] = a[1] + image_height(actor_image(team[i]->actor))/2 + 6;
                bb = bb || bounding_box(a, b);
            }
            
            if(brk->state == BRS_IDLE && bb)
                brk->state = BRS_ACTIVE;

            if((brk->state == BRS_ACTIVE) && ((brk->value[1] += timer_get_delta()) >= BRB_FALL_TIME)) {
                int bi, bj, bw, bh;
                int right_oriented = ((int)brk->brick_ref->behavior_arg[2] != 0);
                image_t *brkimg = brk->brick_ref->image;

                /* particles */
                bw = max(brk->brick_ref->behavior_arg[0], 1);
                bh = max(brk->brick_ref->behavior_arg[1], 1);
                for(bi=0; bi<bw; bi++) {
                    for(bj=0; bj<bh; bj++) {
                        v2d_t piecepos = v2d_new(brk->x + (bi*image_width(brkimg))/bw, brk->y + (bj*image_height(brkimg))/bh);
                        v2d_t piecespeed = v2d_new(0, 20+bj*20+ (right_oriented?bi:bw-bi)*20);
                        image_t *piece = image_create(image_width(brkimg)/bw, image_height(brkimg)/bh);

                        image_blit(brkimg, piece, (bi*image_width(brkimg))/bw, (bj*image_height(brkimg))/bh, 0, 0, image_width(piece), image_height(piece));
                        level_create_particle(piece, piecepos, piecespeed, FALSE);
                    }
                }

                /* bye, brick! :] */
                sound_play( soundfactory_get("break") );
                brk->state = BRS_DEAD;
            }
            break;
        }

        /* moveable bricks */
        case BRB_CIRCULAR: {
            int brkw = image_width(brk->brick_ref->image);
            int brkh = image_height(brk->brick_ref->image);
            float rx, ry, sx, sy, ph, t;
            float a[4], b[4];
            int i;

            t = (brk->value[0] += timer_get_delta()); /* elapsed time */
            rx = brk->brick_ref->behavior_arg[0]; /* x-dist */
            ry = brk->brick_ref->behavior_arg[1]; /* y-dist */
            sx = brk->brick_ref->behavior_arg[2] * (2.0f * PI); /* x-speed */
            sy = brk->brick_ref->behavior_arg[3] * (2.0f * PI); /* x-speed */
            ph = brk->brick_ref->behavior_arg[4] * PI/180.0f; /* initial phase */

            brk->x = brk->sx + round(rx*cos(sx*t+ph));
            brk->y = brk->sy + round(ry*sin(sy*t+ph));

            if(brk->brick_ref->property == BRK_NONE)
                break;

            for(i=0; i<team_size; i++) {
                a[0] = team[i]->actor->position.x - team[i]->actor->hot_spot.x - 3;
                a[1] = team[i]->actor->position.y - team[i]->actor->hot_spot.y - 3;
                a[2] = a[0] + image_width(actor_image(team[i]->actor)) + 6;
                a[3] = a[1] + image_height(actor_image(team[i]->actor)) + 6;

                b[0] = brk->x;
                b[1] = brk->y;
                b[2] = b[0] + brkw;
                b[3] = b[1] + brkh;

                team[i]->on_moveable_platform = FALSE;
                if(!player_is_dying(team[i]) && !player_is_getting_hit(team[i]) && bounding_box(a,b)) {
                    brick_t *down = NULL, *left = NULL, *right = NULL;
                    int cloud = brk->brick_ref->property == BRK_CLOUD;
                    actor_sensors(team[i]->actor, brick_list, NULL, NULL, &right, NULL, &down, NULL, &left, NULL);
                    if((cloud && down == brk) || (!cloud && (down == brk || left == brk || right == brk))) {
                        team[i]->on_moveable_platform = TRUE;
                        team[i]->actor->position = v2d_add(team[i]->actor->position, v2d_multiply(brick_moveable_platform_offset(brk), timer_get_delta()));
                    }
                }
            }
            break;
        }

        /* static bricks */
        default: {
            break;
        }
    }
}


/*
 * brick_render()
 * Renders a brick
 */
void brick_render(brick_t *brk, v2d_t camera_position)
{
    brick_animate(brk);

    if(brk->layer == BRL_DEFAULT || !level_editmode())
        image_draw(brick_image(brk), video_get_backbuffer(), brk->x-((int)camera_position.x-VIDEO_SCREEN_W/2), brk->y-((int)camera_position.y-VIDEO_SCREEN_H/2), IF_NONE);
    else
        image_draw_lit(brick_image(brk), video_get_backbuffer(), brk->x-((int)camera_position.x-VIDEO_SCREEN_W/2), brk->y-((int)camera_position.y-VIDEO_SCREEN_H/2), bricklayer2color(brk->layer), 0.5f, IF_NONE);
}







/*
 * brick_render_path()
 * Renders the path of a brick (if it's a moveable platform)
 */
void brick_render_path(const brick_t *brk, v2d_t camera_position)
{
    float oldx = 0.0f, oldy = 0.0f, x = 0.0f, y = 0.0f, t = 0.0f;
    float rx, ry, sx, sy, ph, off;
    int w = image_width(brick_image(brk));
    int h = image_height(brick_image(brk));
    v2d_t topleft = v2d_subtract(camera_position, v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2));

    switch(brk->brick_ref->behavior) {
        case BRB_CIRCULAR:
            rx = brk->brick_ref->behavior_arg[0];             /* x-dist */
            ry = brk->brick_ref->behavior_arg[1];             /* y-dist */
            sx = brk->brick_ref->behavior_arg[2] * (2*PI);    /* x-speed */
            sy = brk->brick_ref->behavior_arg[3] * (2*PI);    /* y-speed */
            ph = brk->brick_ref->behavior_arg[4] * PI/180.0;  /* initial phase */

            off = sx*t+ph;
            while(sx*t+ph < 2*PI + off) {
                x = brk->sx + round(rx*cos(sx*t+ph));
                y = brk->sy + round(ry*sin(sy*t+ph));

                if(t > 0.0f)
                    image_line(video_get_backbuffer(), (int)(oldx-topleft.x+w/2), (int)(oldy-topleft.y+h/2), (int)(x-topleft.x+w/2), (int)(y-topleft.y+h/2), image_rgb(255,0,0));

                oldx = x;
                oldy = y;
                t += 2*PI / 60.0f;
            }

            t = 0.0f;
            x = brk->sx + round(rx*cos(sx*t+ph));
            y = brk->sy + round(ry*sin(sy*t+ph));
            image_line(video_get_backbuffer(), (int)(oldx-topleft.x+w/2), (int)(oldy-topleft.y+h/2), (int)(x-topleft.x+w/2), (int)(y-topleft.y+h/2), image_rgb(255,0,0));

        default:
            break;
    }
}


/*
 * brick_moveable_platform_offset()
 * Moveable platforms must move actors on top of them.
 * Returns a delta_space vector.
 */
v2d_t brick_moveable_platform_offset(const brick_t *brk)
{
    float t, rx, ry, sx, sy, ph;

    if((brk == NULL) || (brk->brick_ref == NULL))
        return v2d_new(0,0);

    t = brk->value[0]; /* elapsed time */
    switch(brk->brick_ref->behavior) {
        case BRB_CIRCULAR:
            rx = brk->brick_ref->behavior_arg[0];             /* x-dist */
            ry = brk->brick_ref->behavior_arg[1];             /* y-dist */
            sx = brk->brick_ref->behavior_arg[2] * (2*PI);    /* x-speed */
            sy = brk->brick_ref->behavior_arg[3] * (2*PI);    /* y-speed */
            ph = brk->brick_ref->behavior_arg[4] * PI/180.0;  /* initial phase */

            /* take the derivative. e.g.,
               d[ sx + A*cos(PI*t) ]/dt = -A*PI*sin(PI*t) */
            return v2d_new( (-rx*sx)*sin(sx*t+ph), (ry*sy)*cos(sy*t+ph) );

        default:
            return v2d_new(0,0);
    }
}



/*
 * brick_image()
 * Returns the image of an (animated?) brick
 */
const image_t *brick_image(const brick_t *brk)
{
    return brk->brick_ref->image;
}



/*
 * brick_collisionmask()
 * Returns the collision mask of a brick
 */
const collisionmask_t *brick_collisionmask(const brick_t *brk)
{
    return brk->brick_ref->collisionmask;
}



/*
 * brick_get_property_name()
 * Returns the name of a given brick property
 */
const char* brick_get_property_name(brickproperty_t property)
{
    switch(property) {
        case BRK_NONE:
            return "PASSABLE";

        case BRK_OBSTACLE:
            return "OBSTACLE";

        case BRK_CLOUD:
            return "CLOUD";

        default:
            return "Unknown";
    }
}



/*
 * brick_get_behavior_name()
 * Returns the name of a given brick behavior
 */
const char* brick_get_behavior_name(brickbehavior_t behavior)
{
    switch(behavior) {
        case BRB_DEFAULT:
            return "DEFAULT";

        case BRB_CIRCULAR:
            return "CIRCULAR";

        case BRB_BREAKABLE:
            return "BREAKABLE";

        case BRB_FALL:
            return "FALL";

        default:
            return "Unknown";
    }
}

/* utilities */
uint32 bricklayer2color(bricklayer_t layer)
{
    switch(layer) {
        case BRL_GREEN:     return image_rgb(0,255,0);
        case BRL_YELLOW:    return image_rgb(255,255,0);
        default:            return image_rgb(255,255,255);
    }
}

const char* bricklayer2colorname(bricklayer_t layer)
{
    switch(layer) {
        case BRL_GREEN:     return "green";
        case BRL_YELLOW:    return "yellow";
        default:            return "default";
    }
}

bricklayer_t colorname2bricklayer(const char *name)
{
    if(str_icmp(name, "green") == 0)
        return BRL_GREEN;
    else if(str_icmp(name, "yellow") == 0)
        return BRL_YELLOW;
    else
        return BRL_DEFAULT;
}



/* === private stuff === */

/* Animates a brick */
void brick_animate(brick_t *brk)
{
    spriteinfo_t *sprite = brk->brick_ref->data;

    if(sprite != NULL) { /* if brk is not a fake brick */
        int loop = sprite->animation_data[0]->repeat;
        int f, c = sprite->animation_data[0]->frame_count;

        if(!loop)
            brk->animation_frame = min(c-1, brk->animation_frame + sprite->animation_data[0]->fps * timer_get_delta());
        else
            brk->animation_frame = (int)(sprite->animation_data[0]->fps * (timer_get_ticks() * 0.001f)) % c;

        f = clip((int)brk->animation_frame, 0, c-1);
        brk->brick_ref->image = sprite->frame_data[ sprite->animation_data[0]->data[f] ];
    }
}



/* new brick theme */
brickdata_t* brickdata_new()
{
    int i;
    brickdata_t *obj = mallocx(sizeof *obj);

    obj->data = NULL;
    obj->image = NULL;
    obj->collisionmask = NULL;
    obj->property = BRK_NONE;
    obj->angle = 0;
    obj->behavior = BRB_DEFAULT;
    obj->zindex = 0.5f;

    for(i=0; i<BRICKBEHAVIOR_MAXARGS; i++)
        obj->behavior_arg[i] = 0.0f;

    return obj;
}

/* delete brick theme */
brickdata_t* brickdata_delete(brickdata_t *obj)
{
    if(obj != NULL) {
        if(obj->data != NULL)
            spriteinfo_destroy(obj->data);
        collisionmask_destroy(obj->collisionmask);
        free(obj);
    }

    return NULL;
}

/* validates a brick theme */
void validate_brickdata(const brickdata_t *obj)
{
    if(obj->data == NULL)
        fatal_error("Can't load bricks: all bricks must have a sprite!");
}

/* traverses a .brk file */
int traverse(const parsetree_statement_t *stmt)
{
    const char *identifier;
    const parsetree_parameter_t *param_list;
    const parsetree_parameter_t *p1, *p2;
    int brick_id;

    identifier = nanoparser_get_identifier(stmt);
    param_list = nanoparser_get_parameter_list(stmt);

    if(str_icmp(identifier, "brick") == 0) {
        p1 = nanoparser_get_nth_parameter(param_list, 1);
        p2 = nanoparser_get_nth_parameter(param_list, 2);

        nanoparser_expect_string(p1, "Can't load bricks: brick number must be provided");
        nanoparser_expect_program(p2, "Can't load bricks: brick attributes must be provided");

        brick_id = atoi(nanoparser_get_string(p1));
        if(brick_id < 0 || brick_id >= BRKDATA_MAX)
            fatal_error("Can't load bricks: brick number must be in range 0..%d", BRKDATA_MAX-1);

        if(brickdata[brick_id] != NULL)
            brickdata[brick_id] = brickdata_delete(brickdata[brick_id]);

        brickdata_count = max(brickdata_count, brick_id+1);
        brickdata[brick_id] = brickdata_new();
        nanoparser_traverse_program_ex(nanoparser_get_program(p2), (void*)brickdata[brick_id], traverse_brick_attributes);
        validate_brickdata(brickdata[brick_id]);
        brickdata[brick_id]->image = brickdata[brick_id]->data->frame_data[0];
    }
    else
        fatal_error("Can't load bricks: unknown identifier '%s'", identifier);

    return 0;
}

/* traverses a brick { ... } block */
int traverse_brick_attributes(const parsetree_statement_t *stmt, void *brickdata)
{
    const char *identifier;
    const parsetree_parameter_t *param_list;
    const parsetree_parameter_t *p1, *pj;
    brickdata_t *dat = (brickdata_t*)brickdata;
    const char *type;
    int j;

    identifier = nanoparser_get_identifier(stmt);
    param_list = nanoparser_get_parameter_list(stmt);

    if(str_icmp(identifier, "type") == 0) {
        p1 = nanoparser_get_nth_parameter(param_list, 1);
        nanoparser_expect_string(p1, "Can't read brick attributes: must specify brick type");
        type = nanoparser_get_string(p1);

        if(str_icmp(type, "OBSTACLE") == 0)
            dat->property = BRK_OBSTACLE;
        else if(str_icmp(type, "PASSABLE") == 0)
            dat->property = BRK_NONE;
        else if(str_icmp(type, "CLOUD") == 0)
            dat->property = BRK_CLOUD;
        else
            fatal_error("Can't read brick attributes: unknown brick type '%s'", type);
    }
    else if(str_icmp(identifier, "behavior") == 0) {
        p1 = nanoparser_get_nth_parameter(param_list, 1);
        nanoparser_expect_string(p1, "Can't read brick attributes: must specify brick behavior");
        type = nanoparser_get_string(p1);

        if(str_icmp(type, "DEFAULT") == 0)
            dat->behavior = BRB_DEFAULT;
        else if(str_icmp(type, "CIRCULAR") == 0)
            dat->behavior = BRB_CIRCULAR;
        else if(str_icmp(type, "BREAKABLE") == 0)
            dat->behavior = BRB_BREAKABLE;
        else if(str_icmp(type, "FALL") == 0)
            dat->behavior = BRB_FALL;
        else
            fatal_error("Can't read brick attributes: unknown brick type '%s'", type);

        for(j=0; j<BRICKBEHAVIOR_MAXARGS; j++) {
            pj = nanoparser_get_nth_parameter(param_list, 2+j);
            dat->behavior_arg[j] = atof(nanoparser_get_string(pj));
        }
    }
    else if(str_icmp(identifier, "angle") == 0) {
        p1 = nanoparser_get_nth_parameter(param_list, 1);
        nanoparser_expect_string(p1, "Can't read brick attributes: must specify brick angle, a number between 0 and 359");
        dat->angle = ((atoi(nanoparser_get_string(p1)) % 360) + 360) % 360;
    }
    else if(str_icmp(identifier, "zindex") == 0) {
        p1 = nanoparser_get_nth_parameter(param_list, 1);
        nanoparser_expect_string(p1, "Can't read brick attributes: zindex must be a number between 0.0 and 1.0");
        dat->zindex = clip(atof(nanoparser_get_string(p1)), 0.0f, 1.0f);
    }
    else if(str_icmp(identifier, "collision_mask") == 0) {
        p1 = nanoparser_get_nth_parameter(param_list, 1);
        nanoparser_expect_program(p1, "Can't read brick attributes: collision_mask expects a block");
        if(dat->collisionmask != NULL)
            collisionmask_destroy(dat->collisionmask);
        dat->collisionmask = collisionmask_create_from_parsetree(nanoparser_get_program(p1));
    }
    else if(str_icmp(identifier, "sprite") == 0) {
        p1 = nanoparser_get_nth_parameter(param_list, 1);
        nanoparser_expect_program(p1, "Can't read brick attributes: a sprite block must be specified");
        if(dat->data != NULL)
            spriteinfo_destroy(dat->data);
        dat->data = spriteinfo_create(nanoparser_get_program(p1));
    }
    else
        fatal_error("Can't read brick attributes: unkown identifier '%s'", identifier);

    return 0;
}

