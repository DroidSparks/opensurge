/*
 * Open Surge Engine
 * level.h - code for the game levels
 * Copyright (C) 2008-2012  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _LEVEL_H
#define _LEVEL_H

#include "../core/v2d.h"
#include "../core/global.h"

/* forward declarations */
struct image_t;
struct actor_t;
struct player_t;
struct brick_t;
struct brick_list_t;
struct item_t;
struct item_list_t;
struct enemy_t;
struct enemy_list_t;
struct sound_t;

/* use this before pushing the level scene into the stack */
/* multiple levels MUST NOT be pushed onto the scene stack */
void level_setfile(const char *level);

/* scene methods */
void level_init();
void level_update();
void level_render();
void level_release();

/* useful stuff */
const char* level_name();
int level_act();
const char* level_version();
const char* level_author();

void level_change_player(struct player_t *new_player); /* character switching */
struct player_t* level_player(); /* active player */

int level_persist(); /* persists (saves) the current level */

void level_create_particle(struct image_t *image, v2d_t position, v2d_t speed, int destroy_on_brick);
struct brick_t* level_create_brick(int type, v2d_t position);
struct item_t* level_create_item(int type, v2d_t position);
struct enemy_t* level_create_enemy(const char *name, v2d_t position);
void level_add_to_score(int score);
struct item_t* level_create_animal(v2d_t position);
void level_set_camera_focus(struct actor_t *act);
struct actor_t* level_get_camera_focus();
int level_editmode();
v2d_t level_size();
float level_gravity();
void level_override_music(struct sound_t *sample);
void level_set_spawn_point(v2d_t newpos);
void level_clear(struct actor_t *end_sign);
void level_add_to_secret_bonus(int value);
void level_call_dialogbox(const char *title, const char *message);
void level_hide_dialogbox();
void level_lock_camera(int x1, int y1, int x2, int y2);
void level_unlock_camera();
int level_is_camera_locked();
void level_restore_music();
int level_inside_screen(int x, int y, int w, int h);
int level_has_been_cleared();
void level_jump_to_next_stage();
void level_ask_to_leave();
void level_pause();
void level_restart();
int level_waterlevel();
uint32 level_watercolor();
void level_set_waterlevel(int ycoord);
void level_set_watercolor(uint32 color);

#endif
