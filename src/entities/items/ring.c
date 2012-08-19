/*
 * Open Surge Engine
 * ring.c - rings
 * Copyright (C) 2010, 2011  Alexandre Martins <alemartf(at)gmail(dot)com>
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
#include "ring.h"
#include "../../scenes/level.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../core/audio.h"
#include "../../core/soundfactory.h"
#include "../player.h"
#include "../brick.h"
#include "../item.h"
#include "../enemy.h"
#include "../actor.h"

/* ring class */
typedef struct ring_t ring_t;
struct ring_t {
    item_t item; /* base class */
    int is_disappearing; /* is this ring disappearing? */
};

static void ring_init(item_t *item);
static void ring_release(item_t* item);
static void ring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void ring_render(item_t* item, v2d_t camera_position);



/* public methods */
item_t* ring_create()
{
    item_t *item = mallocx(sizeof(ring_t));

    item->init = ring_init;
    item->release = ring_release;
    item->update = ring_update;
    item->render = ring_render;

    return item;
}



/* private methods */
void ring_init(item_t *item)
{
    ring_t *me = (ring_t*)item;

    item->always_active = FALSE;
    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->is_disappearing = FALSE;
    actor_change_animation(item->actor, sprite_get_animation("SD_RING", 0));
    actor_synchronize_animation(item->actor, TRUE);
}



void ring_release(item_t* item)
{
    actor_destroy(item->actor);
}



void ring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    float dt = timer_get_delta();
    ring_t *me = (ring_t*)item;
    actor_t *act = item->actor;
    sound_t *sfx = soundfactory_get("ring");

    /* a player has just got this ring */
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(
            !me->is_disappearing &&
            !player_is_dying(player) &&
            actor_collision(act, player->actor)
        ) {
            player_set_rings(player_get_rings() + 1);
            me->is_disappearing = TRUE;
            item->bring_to_back = FALSE;
            sound_stop(sfx);
            sound_play(sfx);
            break;
        }
    }

    /* disappearing animation... */
    if(me->is_disappearing) {
        actor_change_animation(act, sprite_get_animation("SD_RING", 1));
        if(actor_animation_finished(act))
            item->state = IS_DEAD;
    }

    /* this ring is being attracted by the thunder shield */
    else {
        float mindist = 160.0f;
        player_t *attracted_by = NULL;

        for(i=0; i<team_size; i++) {
            player_t *player = team[i];
            if(player->shield_type == SH_THUNDERSHIELD) {
                float d = v2d_magnitude(v2d_subtract(act->position, player->actor->position));
                if(d < mindist) {
                    attracted_by = player;
                    mindist = d;
                }
            }
        }

        if(attracted_by != NULL) {
            float speed = 320.0f;
            v2d_t diff = v2d_subtract(attracted_by->actor->position, act->position);
            v2d_t d = v2d_multiply(v2d_normalize(diff), speed);
            act->position.x += d.x * dt;
            act->position.y += d.y * dt;
        }
    }
}


void ring_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

