/*
 * Open Surge Engine
 * quest.c - quest scene
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

#include <string.h>
#include "quest.h"
#include "level.h"
#include "../entities/player.h"
#include "../core/global.h"
#include "../core/util.h"
#include "../core/audio.h"
#include "../core/logfile.h"
#include "../core/storyboard.h"
#include "../core/nanocalc/nanocalc.h"
#include "../core/nanocalc/nanocalc_addons.h"

/* private data */
#define STACK_MAX 16
static int top = -1;
static struct {
    quest_t* current_quest;
    int current_level;
    int abort_quest;
} stack[STACK_MAX]; /* one can stack quests */




/* public scene functions */

/*
 * quest_init()
 * Initializes the quest scene. Remember to load
 * some quest (i.e., quest_run) before running this scene!
 */
void quest_init()
{
    if(top < 0)
        fatal_error("Must execute quest_run() before quest_init()");

    stack[top].abort_quest = FALSE;
    music_stop();
}


/*
 * quest_release()
 * Releases the quest scene
 */
void quest_release()
{
    unload_quest(stack[top].current_quest);
    if(0 >= --top) {
        /* scripting: reset global variables & arrays */
        symboltable_clear(symboltable_get_global_table());
        nanocalc_addons_resetarrays();
    }
}


/*
 * quest_render()
 * Actually, this function does nothing
 */
void quest_render()
{
}


/*
 * quest_update()
 * Updates the quest manager
 */
void quest_update()
{
    /* invalid quest */
    if(stack[top].current_quest->level_count == 0)
        fatal_error("Quest '%s' has no levels.", stack[top].current_quest->file);

    /* quest manager */
    if(stack[top].current_level < stack[top].current_quest->level_count && !stack[top].abort_quest) {
        /* next level... */
        level_setfile(stack[top].current_quest->level_path[stack[top].current_level]);
        scenestack_push(storyboard_get_scene(SCENE_LEVEL));
        stack[top].current_level++;
    }
    else {
        /* the user has cleared the quest! */
        scenestack_pop();
        return;
    }
}


/*
 * quest_run()
 * Executes the given quest
 */
void quest_run(quest_t *qst)
{
    top++;
    stack[top].current_quest = qst;
    stack[top].current_level = 0;

    player_set_lives(PLAYER_INITIAL_LIVES);
    player_set_score(0);

    logfile_message("Running quest %s, '%s'...", qst->file, qst->name);
}


/*
 * quest_setlevel()
 * Jumps to the given level, 0 <= lev < n
 */
void quest_setlevel(int lev)
{
    stack[top].current_level = max(0, lev);
}

/*
 * quest_abort()
 * Aborts the current quest
 */
void quest_abort()
{
    logfile_message("Quest aborted!");
    stack[top].abort_quest = TRUE;
}


/*
 * quest_getname()
 * Returns the name of the last running quest
 */
const char *quest_getname()
{
    return stack[top].current_quest->name;
}
