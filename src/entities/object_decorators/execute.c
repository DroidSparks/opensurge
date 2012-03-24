/*
 * Open Surge Engine
 * execute.c - Executes some state immediately
 * Copyright (C) 2012  Alexandre Martins <alemartf(at)gmail(dot)com>
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
#include "execute.h"
#include "../object_vm.h"
#include "../../core/util.h"
#include "../../core/stringutil.h"

/* objectdecorator_executebase_t class */
typedef struct objectdecorator_executebase_t objectdecorator_executebase_t;
struct objectdecorator_executebase_t {
    objectdecorator_t base; /* inherits from objectdecorator_t */
    char *state_name; /* state to be called */
    void (*callback)(objectdecorator_executebase_t*,object_t*,player_t**,int,brick_list_t*,item_list_t*,object_list_t*); /* abstract method */
    void (*destructor)(objectdecorator_executebase_t*); /* abstract method */
};

/* derived classes */
typedef struct objectdecorator_execute_t objectdecorator_execute_t;
struct objectdecorator_execute_t {
    objectdecorator_executebase_t base;
};
static void objectdecorator_execute_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void objectdecorator_execute_destructor(objectdecorator_executebase_t *ex);

typedef struct objectdecorator_executeif_t objectdecorator_executeif_t;
struct objectdecorator_executeif_t {
    objectdecorator_executebase_t base;
    expression_t *condition;
};
static void objectdecorator_executeif_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void objectdecorator_executeif_destructor(objectdecorator_executebase_t *ex);

typedef struct objectdecorator_executeunless_t objectdecorator_executeunless_t;
struct objectdecorator_executeunless_t {
    objectdecorator_executebase_t base;
    expression_t *condition;
};
static void objectdecorator_executeunless_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void objectdecorator_executeunless_destructor(objectdecorator_executebase_t *ex);

typedef struct objectdecorator_executewhile_t objectdecorator_executewhile_t;
struct objectdecorator_executewhile_t {
    objectdecorator_executebase_t base;
    expression_t *condition;
};
static void objectdecorator_executewhile_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void objectdecorator_executewhile_destructor(objectdecorator_executebase_t *ex);

typedef struct objectdecorator_executefor_t objectdecorator_executefor_t;
struct objectdecorator_executefor_t {
    objectdecorator_executebase_t base;
    expression_t *initial, *condition, *iteration;
};
static void objectdecorator_executefor_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void objectdecorator_executefor_destructor(objectdecorator_executebase_t *ex);

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);



/* public methods */
objectmachine_t* objectdecorator_execute_new(objectmachine_t *decorated_machine, const char *state_name)
{
    objectdecorator_execute_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;
    objectdecorator_executebase_t *_me = (objectdecorator_executebase_t*)me;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    _me->state_name = str_dup(state_name);
    _me->callback = objectdecorator_execute_callback;
    _me->destructor = objectdecorator_execute_destructor;

    return obj;
}

objectmachine_t* objectdecorator_executeif_new(objectmachine_t *decorated_machine, const char *state_name, expression_t* condition)
{
    objectdecorator_executeif_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;
    objectdecorator_executebase_t *_me = (objectdecorator_executebase_t*)me;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    _me->state_name = str_dup(state_name);
    _me->callback = objectdecorator_executeif_callback;
    _me->destructor = objectdecorator_executeif_destructor;
    me->condition = condition;

    return obj;
}

objectmachine_t* objectdecorator_executeunless_new(objectmachine_t *decorated_machine, const char *state_name, expression_t* condition)
{
    objectdecorator_executeunless_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;
    objectdecorator_executebase_t *_me = (objectdecorator_executebase_t*)me;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    _me->state_name = str_dup(state_name);
    _me->callback = objectdecorator_executeunless_callback;
    _me->destructor = objectdecorator_executeunless_destructor;
    me->condition = condition;

    return obj;
}

objectmachine_t* objectdecorator_executewhile_new(objectmachine_t *decorated_machine, const char *state_name, expression_t* condition)
{
    objectdecorator_executewhile_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;
    objectdecorator_executebase_t *_me = (objectdecorator_executebase_t*)me;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    _me->state_name = str_dup(state_name);
    _me->callback = objectdecorator_executewhile_callback;
    _me->destructor = objectdecorator_executewhile_destructor;
    me->condition = condition;

    return obj;
}

objectmachine_t* objectdecorator_executefor_new(objectmachine_t *decorated_machine, const char *state_name, expression_t* initial, expression_t* condition, expression_t* iteration)
{
    objectdecorator_executefor_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;
    objectdecorator_executebase_t *_me = (objectdecorator_executebase_t*)me;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    _me->state_name = str_dup(state_name);
    _me->callback = objectdecorator_executefor_callback;
    _me->destructor = objectdecorator_executefor_destructor;
    me->initial = initial;
    me->condition = condition;
    me->iteration = iteration;

    return obj;
}


/* private methods */
void init(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->init(decorated_machine);
}

void release(objectmachine_t *obj)
{
    objectdecorator_executebase_t *me = (objectdecorator_executebase_t*)obj;
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    me->destructor(me);
    free(me->state_name);

    decorated_machine->release(decorated_machine);
    free(obj);
}

void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_executebase_t *me = (objectdecorator_executebase_t*)obj;
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    object_t *object = obj->get_object_instance(obj);

    me->callback(me, object, team, team_size, brick_list, item_list, object_list);

    decorated_machine->update(decorated_machine, team, team_size, brick_list, item_list, object_list);
}

void render(objectmachine_t *obj, v2d_t camera_position)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->render(decorated_machine, camera_position);
}

/* private */
void objectdecorator_execute_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectmachine_t *other_state = objectvm_get_state_by_name(obj->vm, ex->state_name);
    other_state->update(other_state, team, team_size, brick_list, item_list, object_list);
}

void objectdecorator_execute_destructor(objectdecorator_executebase_t *ex)
{
    ;
}

void objectdecorator_executeif_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_executeif_t *me = (objectdecorator_executeif_t*)ex;
    objectmachine_t *other_state = objectvm_get_state_by_name(obj->vm, ex->state_name);

    if(fabs(expression_evaluate(me->condition)) >= 1e-5)
        other_state->update(other_state, team, team_size, brick_list, item_list, object_list);
}

void objectdecorator_executeif_destructor(objectdecorator_executebase_t *ex)
{
    objectdecorator_executeif_t *me = (objectdecorator_executeif_t*)ex;
    expression_destroy(me->condition);
}

void objectdecorator_executeunless_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_executeunless_t *me = (objectdecorator_executeunless_t*)ex;
    objectmachine_t *other_state = objectvm_get_state_by_name(obj->vm, ex->state_name);

    if(!(fabs(expression_evaluate(me->condition)) >= 1e-5))
        other_state->update(other_state, team, team_size, brick_list, item_list, object_list);
}

void objectdecorator_executeunless_destructor(objectdecorator_executebase_t *ex)
{
    objectdecorator_executeunless_t *me = (objectdecorator_executeunless_t*)ex;
    expression_destroy(me->condition);
}

void objectdecorator_executewhile_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_executewhile_t *me = (objectdecorator_executewhile_t*)ex;
    objectmachine_t *other_state = objectvm_get_state_by_name(obj->vm, ex->state_name);

    while(fabs(expression_evaluate(me->condition)) >= 1e-5)
        other_state->update(other_state, team, team_size, brick_list, item_list, object_list);
}

void objectdecorator_executewhile_destructor(objectdecorator_executebase_t *ex)
{
    objectdecorator_executewhile_t *me = (objectdecorator_executewhile_t*)ex;
    expression_destroy(me->condition);
}

void objectdecorator_executefor_callback(objectdecorator_executebase_t *ex, object_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_executefor_t *me = (objectdecorator_executefor_t*)ex;
    objectmachine_t *other_state = objectvm_get_state_by_name(obj->vm, ex->state_name);

    expression_evaluate(me->initial);
    while(fabs(expression_evaluate(me->condition)) >= 1e-5) {
        other_state->update(other_state, team, team_size, brick_list, item_list, object_list);
        expression_evaluate(me->iteration);
    }
}

void objectdecorator_executefor_destructor(objectdecorator_executebase_t *ex)
{
    objectdecorator_executefor_t *me = (objectdecorator_executefor_t*)ex;
    expression_destroy(me->initial);
    expression_destroy(me->condition);
    expression_destroy(me->iteration);
}