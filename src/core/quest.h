/*
 * Open Surge Engine
 * quest.h - quest module
 * Copyright (C) 2008-2010  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _QUEST_H
#define _QUEST_H

struct image_t;
#define QUEST_MAXLEVELS 1024

/*
   quest_t* contains all the data relevant to
   a quest (including name, author and list of
   levels), but it does nothing for itself.

   The quest scene is used to dispatch the
   player to the correct levels.
   (see ../scenes/quest.h)
*/

/* quest structure */
typedef struct quest_t quest_t;
struct quest_t {
    /* meta data */
    char *file; /* file (absolute path) */
    char *name; /* quest name */
    char *author; /* author */
    char *version; /* version string */
    char *description; /* description */
    struct image_t *image; /* thumbnail */
    int is_hidden; /* this quest should not be shown in the custom quests menu */

    /* quest data */
    int level_count; /* how many levels? */
    char *level_path[QUEST_MAXLEVELS]; /* relative paths of the levels */
};

quest_t *load_quest(const char *abs_path);
quest_t *unload_quest(quest_t *qst);

#endif
