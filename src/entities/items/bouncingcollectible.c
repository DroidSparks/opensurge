/*
 * Open Surge Engine
 * bouncingcollectible.c - bouncing collectible
 * Copyright (C) 2011, 2014  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include <math.h>
#include "bouncingcollectible.h"
#include "../../scenes/level.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../core/audio.h"
#include "../../core/video.h"
#include "../../core/image.h"
#include "../../core/soundfactory.h"
#include "../player.h"
#include "../brick.h"
#include "../item.h"
#include "../enemy.h"
#include "../actor.h"

/* bouncingcollectible class */
typedef struct bouncingcollectible_t bouncingcollectible_t;
struct bouncingcollectible_t {
    item_t item; /* base class */
    int is_disappearing; /* is this bouncing collectible disappearing? */
    float life_time; /* life time (used to destroy the moving bouncing collectible after some time) */
};

static void bouncingcollectible_init(item_t *item);
static void bouncingcollectible_release(item_t* item);
static void bouncingcollectible_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void bouncingcollectible_render(item_t* item, v2d_t camera_position);

static int hit_test(int x, int y, const image_t *brk_image, int brk_x, int brk_y);


/* public methods */
item_t* bouncingcollectible_create()
{
    item_t *item = mallocx(sizeof(bouncingcollectible_t));

    item->init = bouncingcollectible_init;
    item->release = bouncingcollectible_release;
    item->update = bouncingcollectible_update;
    item->render = bouncingcollectible_render;

    return item;
}

void bouncingcollectible_set_speed(item_t *item, v2d_t speed)
{
    item->actor->speed = speed;
}



/* private methods */
void bouncingcollectible_init(item_t *item)
{
    bouncingcollectible_t *me = (bouncingcollectible_t*)item;

    item->always_active = FALSE;
    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = FALSE;
    item->actor = actor_create();

    me->is_disappearing = FALSE;
    me->life_time = 0.0f;

    actor_change_animation(item->actor, sprite_get_animation("SD_COLLECTIBLE", 0));
}



void bouncingcollectible_release(item_t* item)
{
    actor_destroy(item->actor);
}



void bouncingcollectible_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    float dt = timer_get_delta();
    bouncingcollectible_t *me = (bouncingcollectible_t*)item;
    actor_t *act = item->actor;
    sound_t *sfx = soundfactory_get("collectible");

    /* a player has just got this bouncing collectible */
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(
            me->life_time >= 1.067f && 
            !me->is_disappearing &&
            !player_is_dying(player) &&
            actor_collision(act, player->actor)
        ) {
            player_set_collectibles(player_get_collectibles() + 1);
            me->is_disappearing = TRUE;
            sound_stop(sfx);
            sound_play(sfx);
            break;
        }
    }

    /* disappearing animation... */
    if(me->is_disappearing) {
        item->bring_to_back = FALSE;
        actor_change_animation(act, sprite_get_animation("SD_COLLECTIBLE", 1));
        if(actor_animation_finished(act))
            item->state = IS_DEAD;
    }

    /* this ring is bouncing around... */
    else {
        /* in order to avoid too much processor load,
           we adopt this simplified platform system */
        int rx, ry, rw, rh, bx, by, bw, bh, j;
        const image_t *ri, *bi;
        brick_list_t *it;
        enum { NONE, FLOOR, RIGHTWALL, CEILING, LEFTWALL } bounce = NONE;

        ri = actor_image(act);
        rx = (int)(act->position.x - act->hot_spot.x);
        ry = (int)(act->position.y - act->hot_spot.y);
        rw = image_width(ri);
        rh = image_height(ri);

        /* who wants to live forever? */
        if((me->life_time += dt) > 4.267f)
            item->state = IS_DEAD;

        /* check for collisions */
        for(it = brick_list; it != NULL && bounce == NONE; it = it->next) {
            if(it->data->brick_ref->property != BRK_NONE) {
                bi = it->data->brick_ref->image;
                bx = it->data->x;
                by = it->data->y;
                bw = image_width(bi);
                bh = image_height(bi);

                if(rx<bx+bw && rx+rw>bx && ry<by+bh && ry+rh>by) {
                    if(image_pixelperfect_collision(ri, bi, rx, ry, bx, by)) {
                        if(hit_test(rx, ry+rh/2, bi, bx, by)) {
                            /* left wall */
                            bounce = LEFTWALL;
                            for(j=1; j<=bw; j++) {
                                if(!image_pixelperfect_collision(ri, bi, rx+j, ry, bx, by)) {
                                    act->position.x += j-1;
                                    break;
                                }
                            }
                        }
                        else if(hit_test(rx+rw-1, ry+rh/2, bi, bx, by)) {
                            /* right wall */
                            bounce = RIGHTWALL;
                            for(j=1; j<=bw; j++) {
                                if(!image_pixelperfect_collision(ri, bi, rx-j, ry, bx, by)) {
                                    act->position.x -= j-1;
                                    break;
                                }
                            }
                        }
                        else if(hit_test(rx+rw/2, ry, bi, bx, by)) {
                            /* ceiling */
                            bounce = CEILING;
                            for(j=1; j<=bh; j++) {
                                if(!image_pixelperfect_collision(ri, bi, rx, ry+j, bx, by)) {
                                    act->position.y += j-1;
                                    break;
                                }
                            }
                        }
                        else if(hit_test(rx+rw/2, ry+rh-1, bi, bx, by)) {
                            /* floor */
                            bounce = FLOOR;
                            for(j=1; j<=bh; j++) {
                                if(!image_pixelperfect_collision(ri, bi, rx, ry-j, bx, by)) {
                                    act->position.y -= j-1;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* bounce & gravity */
        switch(bounce) {
            case FLOOR:
                if(act->speed.y > 0.0f)
                    act->speed.y *= act->speed.y > 1.0f ? -0.75f : 0.0f;
                break;

            case RIGHTWALL:
                if(act->speed.x > 0.0f)
                    act->speed.x *= -0.25f;
                break;

            case LEFTWALL:
                if(act->speed.x < 0.0f)
                    act->speed.x *= -0.25f;
                break;

            case CEILING:
                if(act->speed.y < 0.0f)
                    act->speed.y *= -0.25f;
                break;

            default:
                act->speed.y += (0.09375f * 60.0f * 60.0f) * dt;
                break;
        }

        /* move */
        act->position.x += act->speed.x * dt;
        act->position.y += act->speed.y * dt;
    }
}


void bouncingcollectible_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}



/* (x,y) collides with the brick */
int hit_test(int x, int y, const image_t *brk_image, int brk_x, int brk_y)
{
    if(x >= brk_x && x < brk_x + image_width(brk_image) && y >= brk_y && y < brk_y + image_height(brk_image))
        return image_getpixel(brk_image, x - brk_x, y - brk_y) != video_get_maskcolor();
    else
        return FALSE;
}
