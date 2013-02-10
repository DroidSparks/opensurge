/*
 * Open Surge Engine
 * quest.c - quest scene
 * Copyright (C) 2010, 2012-2013  Alexandre Martins <alemartf(at)gmail(dot)com>
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
#include "../core/util.h"
#include "../core/quest.h"
#include "../core/logfile.h"
#include "../core/storyboard.h"
/*#include "../core/nanocalc/nanocalc.h"
#include "../core/nanocalc/nanocalc_addons.h"*/

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
 * some quest (i.e., quest_push) before running this scene!
 */
void quest_init()
{
    if(top < 0)
        fatal_error("quest_init() error: empty quest stack");
}


/*
 * quest_release()
 * Releases the quest scene
 */
void quest_release()
{
    unload_quest(stack[top--].current_quest);
    /*if(0 == top) {
        symboltable_clear(symboltable_get_global_table());
        nanocalc_addons_resetarrays();
    }*/
}


/*
 * quest_render()
 * Actually, this function does nothing
 */
void quest_render()
{
    ;
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
        level_setfile(stack[top].current_quest->level_path[stack[top].current_level++]);
        scenestack_push(storyboard_get_scene(SCENE_LEVEL));
    }
    else {
        /* the user has cleared (or exited) the quest! */
        if(stack[top].abort_quest)
            logfile_message("Quest '%s' has been aborted. Popping...", stack[top].current_quest->file);
        else
            logfile_message("Quest '%s' has been cleared! Popping...", stack[top].current_quest->file);

        scenestack_pop();
        return;
    }
}














/*
 * quest_setfile()
 * Pushes the given quest onto the quest stack
 */
void quest_setfile(const char *filepath)
{
    if(++top >= STACK_MAX)
        fatal_error("The quest stack can't hold more than %d quests.", STACK_MAX);

    stack[top].current_quest = load_quest(filepath);
    stack[top].current_level = 0;
    stack[top].abort_quest = FALSE;

    logfile_message("Pushing quest '%s' ('%s') onto the quest stack...", stack[top].current_quest->file, stack[top].current_quest->name);
}

/*
 * quest_abort()
 * Aborts the current quest, popping it from the stack
 */
void quest_abort()
{
    if(top >= 0)
        stack[top].abort_quest = TRUE;
}













/*
 * quest_setlevel()
 * Jumps to the given level, 0 <= lev < n
 */
void quest_setlevel(int lev)
{
    if(top >= 0)
        stack[top].current_level = max(0, lev);
}


/*
 * quest_getname()
 * Returns the name of the last running quest
 */
const char *quest_getname()
{
    return top >= 0 ? stack[top].current_quest->name : "null";
}
