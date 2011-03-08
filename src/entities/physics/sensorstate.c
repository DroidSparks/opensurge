/*
 * Open Surge Engine
 * physics/sensorstate.c - physics system: sensor state
 * Copyright (C) 2011  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include "sensorstate.h"
#include "physicsactor.h"
#include "obstacle.h"
#include "obstaclemap.h"
#include "../../core/util.h"
#include "../../core/video.h"

/* sensorstate class */
struct sensorstate_t
{
    const obstacle_t* (*check)(v2d_t,const obstaclemap_t*,int,int,int,int);
    void (*render)(v2d_t,v2d_t,int,int,int,int,uint32);
};

/* private stuff ;-) */
static const obstacle_t* check(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2, movmode_t mm);
static void render(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color);

static const obstacle_t* check_floormode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2);
static void render_floormode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color);
static const obstacle_t* check_rightwallmode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2);
static void render_rightwallmode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color);
static const obstacle_t* check_ceilingmode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2);
static void render_ceilingmode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color);
static const obstacle_t* check_leftwallmode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2);
static void render_leftwallmode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color);



/* public methods */
sensorstate_t* sensorstate_create_floormode()
{
    sensorstate_t *s = mallocx(sizeof *s);
    s->check = check_floormode;
    s->render = render_floormode;
    return s;
}

sensorstate_t* sensorstate_create_rightwallmode()
{
    sensorstate_t *s = mallocx(sizeof *s);
    s->check = check_rightwallmode;
    s->render = render_rightwallmode;
    return s;
}

sensorstate_t* sensorstate_create_ceilingmode()
{
    sensorstate_t *s = mallocx(sizeof *s);
    s->check = check_ceilingmode;
    s->render = render_ceilingmode;
    return s;
}

sensorstate_t* sensorstate_create_leftwallmode()
{
    sensorstate_t *s = mallocx(sizeof *s);
    s->check = check_leftwallmode;
    s->render = render_leftwallmode;
    return s;
}

sensorstate_t* sensorstate_destroy(sensorstate_t *sensorstate)
{
    free(sensorstate);
    return NULL;
}

const obstacle_t* sensorstate_check(const sensorstate_t *sensorstate, v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2)
{
    return sensorstate->check(actor_position, obstaclemap, x1, y1, x2, y2);
}

void sensorstate_render(const sensorstate_t *sensorstate, v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color)
{
    sensorstate->render(actor_position, camera_position, x1, y1, x2, y2, color);
}


/* private stuff */
const obstacle_t* check(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2, movmode_t mm)
{
    x1 += (int)actor_position.x;
    y1 += (int)actor_position.y;
    x2 += (int)actor_position.x;
    y2 += (int)actor_position.y;

    return obstaclemap_get_best_obstacle_at(obstaclemap, min(x1,x2), min(y1,y2), max(x1,x2), max(y1,y2), mm);
}

void render(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color)
{
    x1 += (int)actor_position.x;
    y1 += (int)actor_position.y;
    x2 += (int)actor_position.x;
    y2 += (int)actor_position.y;

    x1 -= (int)(camera_position.x - VIDEO_SCREEN_W/2);
    y1 -= (int)(camera_position.y - VIDEO_SCREEN_H/2);
    x2 -= (int)(camera_position.x - VIDEO_SCREEN_W/2);
    y2 -= (int)(camera_position.y - VIDEO_SCREEN_H/2);

    image_rectfill(video_get_backbuffer(), min(x1,x2), min(y1,y2), max(x1,x2), max(y1,y2), color);
}

const obstacle_t* check_floormode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2)
{
    return check(actor_position, obstaclemap, x1, y1, x2, y2, MM_FLOOR);
}

void render_floormode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color)
{
    render(actor_position, camera_position, x1, y1, x2, y2, color);
}

const obstacle_t* check_rightwallmode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2)
{
    return check(actor_position, obstaclemap, y1, -x1, y2, -x2, MM_RIGHTWALL);
}

void render_rightwallmode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color)
{
    render(actor_position, camera_position, y1, -x1, y2, -x2, color);
}

const obstacle_t* check_ceilingmode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2)
{
    return check(actor_position, obstaclemap, -x1, -y1, -x2, -y2, MM_CEILING);
}

void render_ceilingmode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color)
{
    render(actor_position, camera_position, -x1, -y1, -x2, -y2, color);
}

const obstacle_t* check_leftwallmode(v2d_t actor_position, const obstaclemap_t *obstaclemap, int x1, int y1, int x2, int y2)
{
    return check(actor_position, obstaclemap, -y1, x1, -y2, x2, MM_LEFTWALL);
}

void render_leftwallmode(v2d_t actor_position, v2d_t camera_position, int x1, int y1, int x2, int y2, uint32 color)
{
    render(actor_position, camera_position, -y1, x1, -y2, x2, color);
}
