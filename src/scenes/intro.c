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
#include "../core/fadefx.h"
#include "../core/video.h"
#include "../core/audio.h"
#include "../core/input.h"
#include "../core/osspec.h"
#include "../core/font.h"
#include "../entities/background.h"


/* private data */
#define INTRO_TIMEOUT       4.0f
#define INTRO_QUEST         "quests/intro.qst"
static float elapsed_time;
static void load_intro_quest();
static int must_fadein;
static image_t* bg;
static input_t* in;

static char *text = 
 "<color=eabd85>"
 GAME_TITLE " version " GAME_VERSION_STRING
 "</color>\n"
 "<color=cdc3b6>"
 "Copyright (C) " GAME_YEAR "  Open Surge Team\n"
 GAME_WEBSITE "\n"
 "</color>\n"
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
 "You should have received a copy of the GNU General Public License\n"
 "along with this program; if not, write to the Free Software Foundation,\n"
 "Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n"
 "\n"
 "The following game was built using the Open Surge Engine.\n"
;

static image_t* create_background();

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
    bg = create_background();
    in = input_create_user(NULL);
}


/*
 * intro_release()
 * Releases the introduction scene
 */
void intro_release()
{
    input_destroy(in);
    image_destroy(bg);
}


/*
 * intro_update()
 * Updates the introduction scene
 */
void intro_update()
{
    elapsed_time += timer_get_delta();
    if(!fadefx_is_fading() && !must_fadein && (input_button_pressed(in, IB_FIRE3) || input_button_pressed(in, IB_FIRE4)))
        elapsed_time += INTRO_TIMEOUT;

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

    if(music_is_playing())
        music_stop();
}



/*
 * intro_render()
 * Renders the introduction scene
 */
void intro_render()
{
    image_blit(bg, video_get_backbuffer(), 0, 0, 0, 0, image_width(bg), image_height(bg));
}





/* loads the introduction quest (used for cutscenes, etc) */
void load_intro_quest()
{
    char abs_path[1024];
    resource_filepath(abs_path, INTRO_QUEST, sizeof(abs_path), RESFP_READ);
    scenestack_push(storyboard_get_scene(SCENE_MENU));
    quest_run(load_quest(abs_path));
    scenestack_push(storyboard_get_scene(SCENE_QUEST));
}

/* creates the background */
image_t* create_background()
{
    image_t* img = image_create(VIDEO_SCREEN_W, VIDEO_SCREEN_H);
    font_t *fnt = font_create("menu.small");
    v2d_t camera = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    image_clear(video_get_backbuffer(), image_rgb(0,0,0));
    font_set_text(fnt, "%s", text);
    font_set_position(fnt, v2d_new(5,5));
    font_render(fnt, camera);
    image_blit(video_get_backbuffer(), img, 0, 0, 0, 0, image_width(video_get_backbuffer()), image_height(video_get_backbuffer()));

    font_destroy(fnt);
    return img;
}
