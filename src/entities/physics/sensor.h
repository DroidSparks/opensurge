/*
 * Open Surge Engine
 * physics/sensor.h - physics system: sensors
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

#ifndef _SENSOR_H
#define _SENSOR_H

#include "../../core/v2d.h"
#include "../../core/global.h"

/*
 * a sensor detects collisions between the
 * obstacle map and itself
 */
typedef struct sensor_t sensor_t;

/* forward declarations */
struct obstacle_t;
struct obstaclemap_t;
enum movmode_t;

/* create and destroy */
sensor_t* sensor_create_horizontal(int x1, int y, int x2, uint32 color); /* factory method: new horizontal sensor */
sensor_t* sensor_create_vertical(int x, int y1, int y2, uint32 color); /* factory method: new vertical sensor */
sensor_t* sensor_destroy(sensor_t *sensor);

/* public methods */
const struct obstacle_t* sensor_check(const sensor_t *sensor, v2d_t actor_position, enum movmode_t mm, const struct obstaclemap_t *obstaclemap); /* returns NULL if no obstacle was found */
void sensor_render(const sensor_t *sensor, v2d_t actor_position, enum movmode_t mm, v2d_t camera_position);
int sensor_get_x1(const sensor_t *sensor);
int sensor_get_y1(const sensor_t *sensor);
int sensor_get_x2(const sensor_t *sensor);
int sensor_get_y2(const sensor_t *sensor);
uint32 sensor_get_color(const sensor_t *sensor);

#endif
