/*
 * Open Surge Engine
 * player_action.c - Makes the player perform some actions
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

#include "player_action.h"
#include "../player.h"
#include "../../core/util.h"
#include "../../scenes/level.h"

/* objectdecorator_playeraction_t class */
typedef struct objectdecorator_playeraction_t objectdecorator_playeraction_t;
struct objectdecorator_playeraction_t {
    objectdecorator_t base; /* inherits from objectdecorator_t */
    void (*update)(player_t*); /* strategy */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);

static objectmachine_t *make_decorator(objectmachine_t *decorated_machine, void (*update_strategy)(player_t*));

/* private strategies */
static void springfy(player_t *player);
static void roll(player_t *player);
static void strong(player_t *player);
static void weak(player_t *player);



/* public methods */

/* class constructor */
objectmachine_t* objectdecorator_springfyplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, springfy);
}

objectmachine_t* objectdecorator_rollplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, roll);
}

objectmachine_t* objectdecorator_strongplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, strong);
}

objectmachine_t* objectdecorator_weakplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, weak);
}





/* private methods */

objectmachine_t* make_decorator(objectmachine_t *decorated_machine, void (*update_strategy)(player_t*))
{
    objectdecorator_playeraction_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->update = update_strategy;

    return obj;
}


void init(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->init(decorated_machine);
}

void release(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->release(decorated_machine);
    free(obj);
}

void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_playeraction_t *me = (objectdecorator_playeraction_t*)obj;
    player_t *player = enemy_get_observed_player(obj->get_object_instance(obj));

    me->update(player);

    decorated_machine->update(decorated_machine, team, team_size, brick_list, item_list, object_list);
}

void render(objectmachine_t *obj, v2d_t camera_position)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->render(decorated_machine, camera_position);
}

/* private strategies */
void springfy(player_t *player)
{
    player_spring(player);
}

void roll(player_t *player)
{
    player_roll(player);
}

void strong(player_t *player)
{
    player->attacking = TRUE;
}

void weak(player_t *player)
{
    player->attacking = FALSE;
}
