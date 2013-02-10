/*
 * Open Surge Engine
 * menu.c - menu scene
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "menu.h"
#include "quest.h"
#include "../core/quest.h"
#include "../core/scene.h"
#include "../core/storyboard.h"
#include "../core/global.h"
#include "../core/osspec.h"
#include "../core/v2d.h"
#include "../core/logfile.h"
#include "../core/fadefx.h"
#include "../core/video.h"
#include "../core/audio.h"
#include "../core/timer.h"
#include "../core/lang.h"
#include "../core/soundfactory.h"
#include "../core/font.h"
#include "../entities/actor.h"
#include "../entities/background.h"



/* private data */
#define DEFAULT_QUEST   "quests/default.qst"
#define MENU_MUSICFILE  "musics/theme_song.ogg"
#define MENU_BGFILE     "themes/menu.bg"
#define FADEIN_TIME     0.5f
#define FADEOUT_TIME    0.5f

/* menu screens */
static input_t *input;
static scene_t *jump_to;
static bgtheme_t *bgtheme;


/* main menu */
#define MENU_MAXOPTIONS 3
static float start_time;
static char menu[MENU_MAXOPTIONS][64];
static int menuopt; /* current option */
static font_t *menufnt[MENU_MAXOPTIONS];
static actor_t *arrow;
static int quit;

/* marquee */
static struct marquee_t {
    image_t *background;
    font_t *font;
} marquee;

static void marquee_init();
static void marquee_release();
static void marquee_update();
static void marquee_render();




/* private functions */
static void select_option(int opt);
static void game_start(const char *quest_path);






/* public functions */

/*
 * menu_init()
 * Initializes the menu
 */
void menu_init()
{
    int j;

    /* initializing... */
    quit = FALSE;
    start_time = timer_get_ticks()*0.001;
    jump_to = NULL;
    input = input_create_user(NULL);

    /* background init */
    bgtheme = background_load(MENU_BGFILE);

    /* main menu */
    menuopt = 0;
    arrow = actor_create();
    actor_change_animation(arrow, sprite_get_animation("SD_GUIARROW", 0));

    lang_getstring("MENU_1PGAME", menu[0], sizeof(menu[0]));
    lang_getstring("MENU_OPTIONS", menu[1], sizeof(menu[1]));
    lang_getstring("MENU_EXIT", menu[2], sizeof(menu[2]));

    for(j=0; j<MENU_MAXOPTIONS; j++) {
        menufnt[j] = font_create("menu.main");
        font_set_text(menufnt[j], "%s", menu[j]);
        font_set_position(menufnt[j], v2d_new((VIDEO_SCREEN_W - font_get_textsize(menufnt[j]).x)/2, 167+12*j));
    }

    /* marquee */
    marquee_init();
}


/*
 * menu_update()
 * Updates the menu
 */
void menu_update()
{
    int j;
    float t = timer_get_ticks() * 0.001f;

    /* music */
    if(!music_is_playing())
        music_play( music_load(MENU_MUSICFILE) , INFINITY);

    /* game start */
    if(jump_to != NULL && fadefx_over()) {
        music_stop();
        scenestack_push(jump_to);
        jump_to = NULL;
        return;
    }

    /* quit game */
    if(quit && fadefx_over()) {
        game_quit();
        return;
    }

    /* background movement */
    background_update(bgtheme);

    /* menu programming */
    if(jump_to || quit)
        return;

    /* current option */
    arrow->position.x = font_get_position(menufnt[menuopt]).x - 20 + 3*cos(2*PI * t);
    arrow->position.y = font_get_position(menufnt[menuopt]).y - 1;

    for(j=0; j<MENU_MAXOPTIONS; j++)
        font_set_text(menufnt[j], (j==menuopt) ? "<color=ffff00>%s</color>" : "%s", menu[j]);

    if(input_button_pressed(input, IB_UP)) {
        sound_play( soundfactory_get("choose") );
        menuopt--;
    }
    if(input_button_pressed(input, IB_DOWN)) {
        sound_play( soundfactory_get("choose") );
        menuopt++;
    }

    menuopt = (menuopt%MENU_MAXOPTIONS + MENU_MAXOPTIONS) % MENU_MAXOPTIONS;

    /* select the option */
    if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
        sound_play( soundfactory_get("select") );
        select_option(menuopt);
        return;
    }

    /* marquee */
    marquee_update();
}



/*
 * menu_render()
 * Renders the menu
 */
void menu_render()
{
    int i;
    v2d_t camera = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    /* don't draw anything. We're leaving... */
    if(quit && fadefx_over())
        return;

    /* background */
    background_render_bg(bgtheme, camera);
    background_render_fg(bgtheme, camera);

    /* menu */
    for(i=0; i<MENU_MAXOPTIONS; i++)
        font_render(menufnt[i], camera);
    actor_render(arrow, camera);


    /* marquee */
    marquee_render();
}




/*
 * menu_release()
 * Releases the menu
 */
void menu_release()
{
    int j;

    /* no more music... */
    music_stop();
    music_unref(MENU_MUSICFILE);

    /* main menu stuff */
    for(j=0; j<MENU_MAXOPTIONS; j++)
        font_destroy(menufnt[j]);

    /* background */
    bgtheme = background_unload(bgtheme);

    /* misc */
    actor_destroy(arrow);
    input_destroy(input);

    /* marquee */
    marquee_release();
}








/* private functions */


/* the player selected some option at the main menu */
void select_option(int opt)
{
    char abs_path[1024];

    switch(opt) {
        /* START GAME */
        case 0:
            resource_filepath(abs_path, DEFAULT_QUEST, sizeof(abs_path), RESFP_READ);
            game_start(abs_path);
            return;

        /* OPTIONS */
        case 1:
            jump_to = storyboard_get_scene(SCENE_OPTIONS);
            fadefx_out(image_rgb(0,0,0), FADEOUT_TIME);
            return;

        /* EXIT */
        case 2:
            quit = TRUE;
            fadefx_out(image_rgb(0,0,0), FADEOUT_TIME);
            return;
    }
}


/* closes the menu and starts the game. Call return after this. */
void game_start(const char *quest_path)
{
    quest_setfile(quest_path);
    jump_to = storyboard_get_scene(SCENE_QUEST);
    fadefx_out(image_rgb(0,0,0), FADEOUT_TIME);
}




/* ------ marquee programming -------- */

void marquee_init()
{
    int h;

    marquee.font = font_create("menu.main.marquee");
    font_set_text(marquee.font, "%s v%s - %s - %s", GAME_TITLE, GAME_VERSION_STRING, GAME_WEBSITE, GAME_YEAR);

    h = (int)(1.5f * font_get_textsize(marquee.font).y);
    font_set_position(marquee.font, v2d_new(VIDEO_SCREEN_W, VIDEO_SCREEN_H-h*0.83333f));

    marquee.background = image_create(VIDEO_SCREEN_W, h);
    image_clear(marquee.background, image_rgb(0,0,0));
}

void marquee_release()
{
    image_destroy(marquee.background);
    font_destroy(marquee.font);
}

void marquee_update()
{
    v2d_t v = font_get_position(marquee.font);
    float dt = timer_get_delta();

    v.x -= 50.0f * dt;
    if(v.x < -font_get_textsize(marquee.font).x)
        v.x = VIDEO_SCREEN_W;

    font_set_position(marquee.font, v);
}

void marquee_render()
{
    v2d_t camera = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    image_draw_trans(marquee.background, video_get_backbuffer(), 0, VIDEO_SCREEN_H - image_height(marquee.background), 0.5f, IF_NONE);
    /*image_draw(marquee.background, video_get_backbuffer(), 0, VIDEO_SCREEN_H - image_height(marquee.background), IF_NONE);*/
    font_render(marquee.font, camera);
}

