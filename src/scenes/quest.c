/*
 * Open Surge Engine
 * quest.c - quest scene
 * Copyright (C) 2010  Alexandre Martins <alemartf(at)gmail(dot)com>
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
#include "../core/logfile.h"
#include "../core/storyboard.h"

/* private data */
#define QUESTVALUE_MAX              3
static quest_t* current_quest;
static int current_level;
static int abort_quest;
static int go_back_to_menu;
static float questvalue[QUESTVALUE_MAX];
static char lastname[512] = "NO_QUEST_NAME";




/* public scene functions */

/*
 * quest_init()
 * Initializes the quest scene. Remember to load
 * some quest before running this scene!
 */
void quest_init()
{
    int i;

    abort_quest = FALSE;
    for(i=0; i<QUESTVALUE_MAX; i++)
        questvalue[i] = 0;
}


/*
 * quest_release()
 * Releases the quest scene
 */
void quest_release()
{
    unload_quest(current_quest);
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
    if(current_quest->level_count == 0) {
        logfile_message("Quest '%s' has no levels.", current_quest->file);

        if(go_back_to_menu) {
            scenestack_pop();
            scenestack_push(storyboard_get_scene(SCENE_MENU));
        }
        else
            game_quit();

        return;
    }

    /* quest manager */
    if(current_level < current_quest->level_count && !abort_quest) {
        /* next level... */
        level_setfile(current_quest->level_path[current_level]);
        scenestack_push(storyboard_get_scene(SCENE_LEVEL));
        current_level++;
    }
    else {
        /* the user has cleared the quest! */
        scenestack_pop();
        if(go_back_to_menu) /* if it's not a standalone quest */
            scenestack_push(storyboard_get_scene(SCENE_MENU));

        return;
    }
}


/*
 * quest_run()
 * Executes the given quest
 */
void quest_run(quest_t *qst, int standalone_quest)
{
    current_quest = qst;
    strcpy(lastname, qst->name);
    go_back_to_menu = !standalone_quest;
    player_set_lives(PLAYER_INITIAL_LIVES);
    player_set_score(0);
    logfile_message("Running quest %s, '%s'...", qst->file, qst->name);
    quest_setlevel(0);
}


/*
 * quest_setlevel()
 * Jumps to the given level, 0 <= lev < n
 */
void quest_setlevel(int lev)
{
    current_level = max(0, lev);
}

/*
 * quest_abort()
 * Aborts the current quest
 */
void quest_abort()
{
    logfile_message("Quest aborted!");
    abort_quest = TRUE;
}


/*
 * quest_getname()
 * Returns the name of the last running quest
 */
const char *quest_getname()
{
    return lastname;
}




/* quest values */

/*
 * quest_setvalue()
 * Sets a new value to a quest value
 *
 * key = QUESTVALUE_*
 * value = new value
 */
void quest_setvalue(questvalue_t key, float value)
{
    int k = clip((int)key, 0, QUESTVALUE_MAX-1);
    questvalue[k] = value;
}


/*
 * quest_getvalue()
 * Returns a quest value
 */
float quest_getvalue(questvalue_t key)
{
    int k = clip((int)key, 0, QUESTVALUE_MAX-1);
    return questvalue[k];
}

