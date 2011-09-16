/*
 * Open Surge Engine
 * nanocalcext.c - nanocalc extensions
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

#include <time.h>
#include <math.h>
#include "nanocalc/nanocalc.h"
#include "nanocalcext.h"
#include "video.h"
#include "audio.h"
#include "timer.h"
#include "../scenes/level.h"
#include "../entities/actor.h"
#include "../entities/player.h"
#include "../entities/enemy.h"

/* private stuff ;) */
#define PLAYER (enemy_get_observed_player(target))
static object_t *target; /* target object */
static const struct tm* timeinfo() { time_t raw = time(NULL); return localtime(&raw); }

/* BIFs */
static float f_elapsed_time() { return 0.001f * timer_get_ticks(); } /* elapsed time, in seconds */
static float f_dt() { return timer_get_delta(); } /* time difference between 2 consecutive frames */
static float f_fps() { return (float)timer_get_fps(); } /* frames per second */
static float f_collectibles() { return (float)player_get_rings(); } /* number of collectibles */
static float f_lives() { return (float)player_get_lives(); } /* number of lives */
static float f_score() { return (float)player_get_score(); } /* returns the score */
static float f_gravity() { return level_gravity(); } /* returns the gravity strength */
static float f_act() { return (float)level_act(); } /* returns the current act number */
static float f_xpos() { return target->actor->position.x; } /* x-position of the target object */
static float f_ypos() { return target->actor->position.y; } /* x-position of the target object */
static float f_hotspot_x() { return target->actor->hot_spot.x; } /* x-position of the hotspot */
static float f_hotspot_y() { return target->actor->hot_spot.y; } /* y-position of the hotspot */
static float f_alpha() { return target->actor->alpha; } /* alpha of the target object */
static float f_angle() { return target->actor->angle * 180.0f / PI; } /* angle of the target object */
static float f_scale_x() { return target->actor->scale.x; } /* scale x */
static float f_scale_y() { return target->actor->scale.y; } /* scale y */
static float f_animation_frame() { return floor(target->actor->animation_frame); } /* animation frame */
static float f_animation_speed_factor() { return target->actor->animation_speed_factor; }
static float f_animation_repeats() { return target->actor->animation->repeat ? 1.0f : 0.0f; }
static float f_animation_fps() { return (float)target->actor->animation->fps; }
static float f_animation_frame_count() { return (float)target->actor->animation->frame_count; }
static float f_zindex() { return target->zindex; }
static float f_spawnpoint_x() { return target->actor->spawn_point.x; }
static float f_spawnpoint_y() { return target->actor->spawn_point.y; }
static float f_screen_width() { return (float)VIDEO_SCREEN_W; }
static float f_screen_height() { return (float)VIDEO_SCREEN_H; }
static float f_width() { return actor_image(target->actor)->w; }
static float f_height() { return actor_image(target->actor)->h; }
static float f_direction() { return target->actor->mirror & IF_HFLIP ? -1.0f : 1.0f; }
static float f_player_xpos() { return PLAYER->actor->position.x; }
static float f_player_ypos() { return PLAYER->actor->position.y; }
static float f_player_spawnpoint_x() { return PLAYER->actor->spawn_point.x; }
static float f_player_spawnpoint_y() { return PLAYER->actor->spawn_point.y; }
static float f_player_xspeed() { return PLAYER->actor->speed.x; }
static float f_player_yspeed() { return PLAYER->actor->speed.y; }
static float f_player_angle() { return PLAYER->actor->angle * 180.0f / PI; }
static float f_player_direction() { return PLAYER->actor->mirror & IF_HFLIP ? -1.0f : 1.0f; }
static float f_music_volume() { return music_get_volume(); }
static float f_music_is_playing() { return music_is_playing() ? 1.0f : 0.0f; }
static float f_date_sec() { return (float)(timeinfo()->tm_sec); }  /* seconds after the minute; range: 0-59 */
static float f_date_min() { return (float)(timeinfo()->tm_min); }  /* minutes after the hour; range: 0-59 */
static float f_date_hour() { return (float)(timeinfo()->tm_hour); } /* hours since midnight; range: 0-23 */
static float f_date_mday() { return (float)(timeinfo()->tm_mday); } /* days of the months; range: 1-31 */
static float f_date_mon() { return (float)(timeinfo()->tm_mon); }  /* months since Janurary; range: 0-11 */
static float f_date_year() { return (float)(timeinfo()->tm_year); } /* years since 1900 */
static float f_date_wday() { return (float)(timeinfo()->tm_wday); } /* days since Sunday; range: 0-6 */
static float f_date_yday() { return (float)(timeinfo()->tm_yday); } /* days since January 1st; range: 0-365 */
static float f_music_duration() { return music_duration(); }



/*
 * nanocalcext_register_bifs()
 * Registers a lot of useful built-in functions in nanocalc
 */
void nanocalcext_register_bifs()
{
    nanocalc_register_bif_arity0("elapsed_time", f_elapsed_time);
    nanocalc_register_bif_arity0("dt", f_dt);
    nanocalc_register_bif_arity0("fps", f_fps);
    nanocalc_register_bif_arity0("collectibles", f_collectibles);
    nanocalc_register_bif_arity0("lives", f_lives);
    nanocalc_register_bif_arity0("score", f_score);
    nanocalc_register_bif_arity0("gravity", f_gravity);
    nanocalc_register_bif_arity0("act", f_act);
    nanocalc_register_bif_arity0("xpos", f_xpos);
    nanocalc_register_bif_arity0("ypos", f_ypos);
    nanocalc_register_bif_arity0("hotspot_x", f_hotspot_x);
    nanocalc_register_bif_arity0("hotspot_y", f_hotspot_y);
    nanocalc_register_bif_arity0("alpha", f_alpha);
    nanocalc_register_bif_arity0("angle", f_angle);
    nanocalc_register_bif_arity0("scale_x", f_scale_x);
    nanocalc_register_bif_arity0("scale_y", f_scale_y);
    nanocalc_register_bif_arity0("direction", f_direction);
    nanocalc_register_bif_arity0("animation_frame", f_animation_frame);
    nanocalc_register_bif_arity0("animation_speed_factor", f_animation_speed_factor);
    nanocalc_register_bif_arity0("animation_repeats", f_animation_repeats);
    nanocalc_register_bif_arity0("animation_fps", f_animation_fps);
    nanocalc_register_bif_arity0("animation_frame_count", f_animation_frame_count);
    nanocalc_register_bif_arity0("zindex", f_zindex);
    nanocalc_register_bif_arity0("spawnpoint_x", f_spawnpoint_x);
    nanocalc_register_bif_arity0("spawnpoint_y", f_spawnpoint_y);
    nanocalc_register_bif_arity0("player_xpos", f_player_xpos);
    nanocalc_register_bif_arity0("player_ypos", f_player_ypos);
    nanocalc_register_bif_arity0("player_spawnpoint_x", f_player_spawnpoint_x);
    nanocalc_register_bif_arity0("player_spawnpoint_y", f_player_spawnpoint_y);
    nanocalc_register_bif_arity0("player_xspeed", f_player_xspeed);
    nanocalc_register_bif_arity0("player_yspeed", f_player_yspeed);
    nanocalc_register_bif_arity0("player_angle", f_player_angle);
    nanocalc_register_bif_arity0("player_direction", f_player_direction);
    nanocalc_register_bif_arity0("screen_width", f_screen_width);
    nanocalc_register_bif_arity0("screen_height", f_screen_height);
    nanocalc_register_bif_arity0("width", f_width);
    nanocalc_register_bif_arity0("height", f_height);
    nanocalc_register_bif_arity0("music_volume", f_music_volume);
    nanocalc_register_bif_arity0("music_is_playing", f_music_is_playing);
    nanocalc_register_bif_arity0("date_sec", f_date_sec);
    nanocalc_register_bif_arity0("date_min", f_date_min);
    nanocalc_register_bif_arity0("date_hour", f_date_hour);
    nanocalc_register_bif_arity0("date_mday", f_date_mday);
    nanocalc_register_bif_arity0("date_mon", f_date_mon);
    nanocalc_register_bif_arity0("date_year", f_date_year);
    nanocalc_register_bif_arity0("date_wday", f_date_wday);
    nanocalc_register_bif_arity0("date_yday", f_date_yday);
    nanocalc_register_bif_arity0("music_duration", f_music_duration);

    target = NULL;
}


/*
 * nanocalcext_set_target_object()
 * Defines a target object, used in some built-in functions called by nanocalc
 */
void nanocalcext_set_target_object(object_t *o)
{
    target = o;
}

