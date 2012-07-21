/*
 * Open Surge Engine
 * collisionmask.h - collision mask
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

#ifndef _COLLISIONMASK_H
#define _COLLISIONMASK_H

struct image_t;
struct collisionmask_t;
struct parsetree_program_t;
struct spriteinfo_t;
typedef struct collisionmask_t collisionmask_t;

/* creates a new collision mask */
collisionmask_t *collisionmask_create(const struct image_t *image, int x, int y, int width, int height);
collisionmask_t *collisionmask_create_from_parsetree(const struct parsetree_program_t *block);
collisionmask_t *collisionmask_create_from_sprite(const struct spriteinfo_t *sprite);

/* destroys an existing collision mask */
collisionmask_t *collisionmask_destroy(collisionmask_t *cm);

/* check for collision */
int collisionmask_check(collisionmask_t *cm, int x, int y);

/* returns the collision mask as an image, where each pixel is:
       magenta if non-solid
       non-magenta if solid
*/
const struct image_t *collisionmask_image(const collisionmask_t *cm);

#endif
