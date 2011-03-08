/*
 * Open Surge Engine
 * explosion.c - explosion
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

#include "explosion.h"
#include "../../core/util.h"
#include "../player.h"
#include "../brick.h"
#include "../item.h"
#include "../enemy.h"
#include "../actor.h"

/* explosion class */
typedef struct explosion_t explosion_t;
struct explosion_t {
    item_t item; /* base class */
};

static void explosion_init(item_t *item);
static void explosion_release(item_t* item);
static void explosion_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void explosion_render(item_t* item, v2d_t camera_position);



/* public methods */
item_t* explosion_create()
{
    item_t *item = mallocx(sizeof(explosion_t));

    item->init = explosion_init;
    item->release = explosion_release;
    item->update = explosion_update;
    item->render = explosion_render;

    return item;
}


/* private methods */
void explosion_init(item_t *item)
{
    item->always_active = FALSE;
    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_EXPLOSION", 0));
}



void explosion_release(item_t* item)
{
    actor_destroy(item->actor);
}



void explosion_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    if(actor_animation_finished(item->actor))
        item->state = IS_DEAD;
}


void explosion_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

