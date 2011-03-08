/*
 * Open Surge Engine
 * intro.c - introduction scene
 * Copyright (C) 2008-2011  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include "intro.h"
#include "quest.h"
#include "../core/v2d.h"
#include "../core/timer.h"
#include "../core/scene.h"
#include "../core/storyboard.h"
#include "../core/video.h"
#include "../core/audio.h"
#include "../core/osspec.h"
#include "../core/font.h"
#include "../entities/background.h"


/* private data */
#define INTRO_TIMEOUT       4.0f
static float elapsed_time;
static void load_intro_quest();
static int must_fadein;
static font_t *fnt;

static char *text = 
 GAME_TITLE " version " GAME_VERSION_STRING "\n"
 "Copyright (C) " GAME_YEAR "  Open Surge Team\n"
 GAME_WEBSITE "\n"
 "\n"
 "This program is free software; you can redistribute it and/or modify\n"
 "it under the terms of the GNU General Public License as published by\n"
 "the Free Software Foundation; either version 2 of the License, or\n"
 "(at your option) any later version.\n"
 "\n"
 "This program is distributed in the hope that it will be useful,\n"
 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
 "GNU General Public License for more details.\n"
 "\n"
 "You should have received a copy of the GNU General Public License along\n"
 "with this program; if not, write to the Free Software Foundation, Inc.,\n"
 "51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n"
;


/* public functions */

/*
 * intro_init()
 * Initializes the introduction scene
 */
void intro_init()
{
    elapsed_time = 0.0f;
    must_fadein = TRUE;
    music_stop();

    fnt = font_create("menu.small");
    font_set_text(fnt, "<color=aaaaaa>%s</color>", text);
    font_set_position(fnt, v2d_new(5,5));
}


/*
 * intro_release()
 * Releases the introduction scene
 */
void intro_release()
{
    font_destroy(fnt);
}


/*
 * intro_update()
 * Updates the introduction scene
 */
void intro_update()
{
    elapsed_time += timer_get_delta();

    if(must_fadein) {
        fadefx_in(image_rgb(0,0,0), 1.0f);
        must_fadein = FALSE;
    }
    else if(elapsed_time >= INTRO_TIMEOUT) {
        if(fadefx_over()) {
            scenestack_pop();
            load_intro_quest();
            return;
        }
        fadefx_out(image_rgb(0,0,0), 1.0f);
    }
}



/*
 * intro_render()
 * Renders the introduction scene
 */
void intro_render()
{
    v2d_t camera = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    image_clear(video_get_backbuffer(), image_rgb(0,0,0));
    font_render(fnt, camera);
}





/* loads the introduction quest (used for cutscenes, etc) */
void load_intro_quest()
{
    char abs_path[1024];

    resource_filepath(abs_path, "quests/intro.qst", sizeof(abs_path), RESFP_READ);
    quest_run(load_quest(abs_path), FALSE);
    scenestack_push(storyboard_get_scene(SCENE_QUEST));
}
