/*
 * Open Surge Engine
 * renderqueue.h - render queue
 * Copyright (C) 2010, 2012  Alexandre Martins <alemartf(at)gmail(dot)com>
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
#include "../core/util.h"
#include "renderqueue.h"
#include "particle.h"
#include "player.h"
#include "brick.h"
#include "item.h"
#include "enemy.h"
#include "actor.h"

/* private stuff ;) */
typedef union renderable_t renderable_t;
union renderable_t {
    player_t *player;
    brick_t *brick;
    item_t *item;
    object_t *object;
};

typedef struct renderqueue_cell_t renderqueue_cell_t;
struct renderqueue_cell_t {
    renderable_t entity;
    float (*zindex)(renderable_t);
    void (*render)(renderable_t,v2d_t);
    int (*ypos)(renderable_t);
    int (*type)(renderable_t);
};

typedef struct renderqueue_t renderqueue_t;
struct renderqueue_t {
    renderqueue_cell_t cell;
    renderqueue_t *next; /* linked list */
};

static renderqueue_t* queue = NULL;
static int size = 0;
static v2d_t camera;

static int cmp_fun(const void *i, const void *j)
{
    const renderqueue_cell_t *a = (const renderqueue_cell_t*)i;
    const renderqueue_cell_t *b = (const renderqueue_cell_t*)j;

    if(fabs(a->zindex(a->entity) - b->zindex(b->entity)) < 1e-7) {
        if(a->type(a->entity) == b->type(b->entity))
            return a->ypos(a->entity) - b->ypos(b->entity);
        else
            return 0;
    }
    else if(a->zindex(a->entity) < b->zindex(b->entity))
        return -1;
    else
        return 1;
}

static float brick_zindex_offset(const brick_t *b)
{
    float s = 0.0f;

    /* a hackish solution... */
    switch(b->brick_ref->property) {
        case BRK_NONE:      s -= 0.00002f;
        case BRK_CLOUD:     s -= 0.00001f;
        case BRK_OBSTACLE:  s -= 0.00000f;
    }

    switch(b->layer) {
        case BRL_YELLOW:    s -= 0.0002f;
        case BRL_GREEN:     s += 0.0001f;
        case BRL_DEFAULT:   s += 0.0000f;
    }

    return s;
}

/* private strategies */
static float zindex_particles(renderable_t r) { return 1.0f; }
static float zindex_player(renderable_t r) { return player_is_dying(r.player) ? 1.0f : 0.5f; }
static float zindex_item(renderable_t r) { return 0.5f; }
static float zindex_object(renderable_t r) { return r.object->zindex; }
static float zindex_brick(renderable_t r) { return r.brick->brick_ref->zindex + brick_zindex_offset(r.brick); }

static void render_particles(renderable_t r, v2d_t camera_position) { particle_render_all(camera_position); }
static void render_player(renderable_t r, v2d_t camera_position) { player_render(r.player, camera_position); }
static void render_item(renderable_t r, v2d_t camera_position) { item_render(r.item, camera_position); }
static void render_object(renderable_t r, v2d_t camera_position) { enemy_render(r.object, camera_position); }
static void render_brick(renderable_t r, v2d_t camera_position) { brick_render(r.brick, camera_position); }

static int ypos_particles(renderable_t r) { return 0; }
static int ypos_player(renderable_t r) { return 0; /*(int)(r.player->actor->position.y);*/ }
static int ypos_item(renderable_t r) { return (int)(r.item->actor->position.y); }
static int ypos_object(renderable_t r) { return (int)(r.object->actor->position.y); }
static int ypos_brick(renderable_t r) { return r.brick->y; }

static int type_particles(renderable_t r) { return 0; }
static int type_player(renderable_t r) { return 1; }
static int type_item(renderable_t r) { return 2; }
static int type_object(renderable_t r) { return 3; }
static int type_brick(renderable_t r) { return 4; }




/* public interface */

/* starts a new rendering process */
void renderqueue_begin(v2d_t camera_position)
{
    /* initialize stuff */
    queue = NULL;
    size = 0;
    camera = camera_position;
}

/* finishes an existing rendering process, rendering everything */
void renderqueue_end()
{
    renderqueue_t *it, *next;
    renderqueue_cell_t *arr;
    int i = size;

    /* create a temporary array */
    arr = mallocx(size * sizeof *arr);
    for(it=queue; it; it=it->next)
        arr[--i] = it->cell;

    /* sort stuff. we need an stable sorting algorithm here. */
    merge_sort(arr, size, sizeof *arr, cmp_fun);

    /* render everything */
    for(i=0; i<size; i++)
        arr[i].render(arr[i].entity, camera);

    /* release the temporary array */
    free(arr);

    /* release stuff */
    for(it=queue; it; it=next) {
        next = it->next;
        free(it);
    }
    size = 0;
    queue = NULL;
}

/* enqueues entities */
void renderqueue_enqueue_brick(brick_t *brick)
{
    renderqueue_t *node = mallocx(sizeof *node);
    node->cell.entity.brick = brick;
    node->cell.zindex = zindex_brick;
    node->cell.render = render_brick;
    node->cell.ypos = ypos_brick;
    node->cell.type = type_brick;
    node->next = queue;
    queue = node;
    size++;
}

void renderqueue_enqueue_item(item_t *item)
{
    renderqueue_t *node = mallocx(sizeof *node);
    node->cell.entity.item = item;
    node->cell.zindex = zindex_item;
    node->cell.render = render_item;
    node->cell.ypos = ypos_item;
    node->cell.type = type_item;
    node->next = queue;
    queue = node;
    size++;
}

void renderqueue_enqueue_object(object_t *object)
{
    renderqueue_t *node = mallocx(sizeof *node);
    node->cell.entity.object = object;
    node->cell.zindex = zindex_object;
    node->cell.render = render_object;
    node->cell.ypos = ypos_object;
    node->cell.type = type_object;
    node->next = queue;
    queue = node;
    size++;
}

void renderqueue_enqueue_player(player_t *player)
{
    renderqueue_t *node = mallocx(sizeof *node);
    node->cell.entity.player = player;
    node->cell.zindex = zindex_player;
    node->cell.render = render_player;
    node->cell.ypos = ypos_player;
    node->cell.type = type_player;
    node->next = queue;
    queue = node;
    size++;
}

void renderqueue_enqueue_particles()
{
    renderqueue_t *node = mallocx(sizeof *node);
    node->cell.zindex = zindex_particles;
    node->cell.render = render_particles;
    node->cell.ypos = ypos_particles;
    node->cell.type = type_particles;
    node->next = queue;
    queue = node;
    size++;
}
