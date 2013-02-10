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

/* scene methods */
void level_init(void *path_to_lev_file); /* pass an string containing the path to the .lev file */
void level_update();
void level_render();
void level_release();




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

/* read-only information */
const char* level_name();
int level_act();
const char* level_version();
const char* level_author();

/* load & save */
void level_change(const char* path_to_lev_file); /* change the stage. Useful if the .lev is active. */
int level_persist(); /* persists (saves) the current level */

/* cooperative play */
void level_change_player(struct player_t *new_player); /* character switching */
struct player_t* level_player(); /* active player */
void level_set_spawn_point(v2d_t newpos);

/* entities */
void level_create_particle(struct image_t *image, v2d_t position, v2d_t speed, int destroy_on_brick);
struct brick_t* level_create_brick(int type, v2d_t position);
struct item_t* level_create_item(int type, v2d_t position);
struct enemy_t* level_create_enemy(const char *name, v2d_t position);
void level_add_to_score(int score);
struct item_t* level_create_animal(v2d_t position);

/* camera */
void level_set_camera_focus(struct actor_t *act);
struct actor_t* level_get_camera_focus();
int level_is_camera_locked();
void level_lock_camera(int x1, int y1, int x2, int y2);
void level_unlock_camera();
int level_inside_screen(int x, int y, int w, int h);

/* editor */
int level_editmode();

/* dialogbox */
void level_call_dialogbox(const char *title, const char *message);
void level_hide_dialogbox();

/* music */
void level_override_music(struct sound_t *sample);
void level_restore_music();

/* management */
void level_clear(struct actor_t *end_sign);
int level_has_been_cleared();
void level_jump_to_next_stage();
void level_ask_to_leave();
void level_pause();
void level_restart();

/* water */
int level_waterlevel();
uint32 level_watercolor();
void level_set_waterlevel(int ycoord);
void level_set_watercolor(uint32 color);

/* misc */
v2d_t level_size();
float level_gravity();

/* quest stack (scripting) */
void level_push_quest(const char* path_to_qst_file);
void level_pop_quest();

#endif
