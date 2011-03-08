/*
 * Open Surge Engine
 * physics/obstacle.h - physics system: obstacles
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

#ifndef _OBSTACLE_H
#define _OBSTACLE_H

#include "../../core/v2d.h"

/*
 * an obstacle may be anything "physical": a non-passable brick,
 * built-in item or custom object. The Physics engine works with
 * obstacles only.
 */
typedef struct obstacle_t obstacle_t;

/* auxiliary enumeration for obstacle_get_height_at() */
typedef enum obstaclebaselevel_t obstaclebaselevel_t;
enum obstaclebaselevel_t {
    FROM_BOTTOM,
    FROM_LEFT,
    FROM_TOP,
    FROM_RIGHT
};

/* forward declarations */
struct image_t;

/* create and destroy */
obstacle_t* obstacle_create_solid(const struct image_t *image, int angle, v2d_t position);
obstacle_t* obstacle_create_oneway(const struct image_t *image, int angle, v2d_t position);
obstacle_t* obstacle_destroy(obstacle_t *obstacle);

/* public methods */
v2d_t obstacle_get_position(const obstacle_t *obstacle); /* position */
int obstacle_is_solid(const obstacle_t *obstacle); /* is it solid or oneway? */
int obstacle_get_width(const obstacle_t *obstacle); /* width of the bounding box */
int obstacle_get_height(const obstacle_t *obstacle); /* height of the bounding box */
int obstacle_get_angle(const obstacle_t *obstacle); /* angle */
int obstacle_get_height_at(const obstacle_t *obstacle, int position_on_base_axis, obstaclebaselevel_t base_level); /* height map */
const struct image_t* obstacle_get_image(const obstacle_t *obstacle); /* the image */

#endif
