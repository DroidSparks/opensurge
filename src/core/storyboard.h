/*
 * Open Surge Engine
 * storyboard.h - storyboard (stores the scenes of the game)
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

#ifndef _STORYBOARD_H
#define _STORYBOARD_H

/* available scenes */
typedef enum scenetype_t {
    SCENE_INTRO,
    SCENE_LEVEL,
    SCENE_PAUSE,
    SCENE_GAMEOVER,
    SCENE_QUEST,
    SCENE_CONFIRMBOX,
    SCENE_LANGSELECT,
    SCENE_CREDITS,
    SCENE_CREDITS2,
    SCENE_OPTIONS,
    SCENE_STAGESELECT,
    SCENE_QUESTSELECT,
    SCENE_EDITORHELP
} scenetype_t;

/* Storyboard */
void storyboard_init();
void storyboard_release();
struct scene_t* storyboard_get_scene(scenetype_t type);

#endif
