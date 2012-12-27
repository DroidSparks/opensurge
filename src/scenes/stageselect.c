/*
 * Open Surge Engine
 * stageselect.c - stage selection screen
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
#include <math.h>
#include <ctype.h>
#include "stageselect.h"
#include "options.h"
#include "level.h"
#include "../entities/player.h"
#include "../core/nanocalc/nanocalc.h"
#include "../core/util.h"
#include "../core/scene.h"
#include "../core/storyboard.h"
#include "../core/v2d.h"
#include "../core/osspec.h"
#include "../core/stringutil.h"
#include "../core/logfile.h"
#include "../core/video.h"
#include "../core/audio.h"
#include "../core/lang.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../core/soundfactory.h"
#include "../core/nanoparser/nanoparser.h"
#include "../core/font.h"
#include "../entities/actor.h"
#include "../entities/background.h"



/* stage data */
typedef struct {
    char filepath[1024]; /* absolute filepath */
    char name[128]; /* stage name */
    int act; /* act number */
    int requires[3]; /* required version */
} stagedata_t;

static stagedata_t* stagedata_load(const char *filename);
static void stagedata_unload(stagedata_t *s);
static int traverse(const parsetree_statement_t *stmt, void *stagedata);



/* private data */
#define STAGE_BGFILE             "themes/levelselect.bg"
#define STAGE_MAXPERPAGE         8
#define STAGE_MAX                1024 /* can't have more than STAGE_MAX levels installed */
static font_t *title; /* title */
static font_t *msg; /* message */
static font_t *page; /* page number */
static actor_t *icon; /* cursor icon */
static input_t *input; /* input device */
static float scene_time; /* scene time, in seconds */
static bgtheme_t *bgtheme; /* current background */
static enum { STAGESTATE_NORMAL, STAGESTATE_QUIT, STAGESTATE_PLAY, STAGESTATE_FADEIN } state; /* finite state machine */
static stagedata_t *stage_data[STAGE_MAX]; /* vector of stagedata_t* */
static int stage_count; /* length of stage_data[] */
static int option; /* current option: 0 .. stage_count - 1 */
static font_t **stage_label; /* vector */
static int enable_debug = FALSE; /* debug mode????????? must start out as false. */



/* private functions */
static void load_stage_list();
static void unload_stage_list();
static int dirfill(const char *filename, void *param);
static int sort_cmp(const void *a, const void *b);






/* public functions */

/*
 * stageselect_init()
 * Initializes the scene
 */
void stageselect_init()
{
    option = 0;
    scene_time = 0;
    state = STAGESTATE_NORMAL;
    input = input_create_user(NULL);

    title = font_create("menu.title");
    font_set_text(title, lang_get("STAGESELECT_TITLE"));
    font_set_position(title, v2d_new((VIDEO_SCREEN_W - font_get_textsize(title).x)/2, 10));

    msg = font_create("menu.text");
    font_set_text(msg, lang_get("STAGESELECT_MSG"));
    font_set_position(msg, v2d_new(10, VIDEO_SCREEN_H - font_get_textsize(msg).y * 1.5f));

    page = font_create("menu.text");
    font_set_text(page, lang_get("STAGESELECT_PAGE"), 0, 0);
    font_set_position(page, v2d_new(VIDEO_SCREEN_W - font_get_textsize(page).x - 10, font_get_position(msg).y));

    icon = actor_create();
    actor_change_animation(icon, sprite_get_animation("SD_GUIARROW", 0));

    load_stage_list();
    bgtheme = background_load(STAGE_BGFILE);

    fadefx_in(image_rgb(0,0,0), 1.0);
}


/*
 * stageselect_release()
 * Releases the scene
 */
void stageselect_release()
{
    enable_debug = FALSE;

    bgtheme = background_unload(bgtheme);
    unload_stage_list();

    actor_destroy(icon);
    font_destroy(title);
    font_destroy(msg);
    font_destroy(page);
    input_destroy(input);
}


/*
 * stageselect_update()
 * Updates the scene
 */
void stageselect_update()
{
    int pagenum, maxpages;
    float dt = timer_get_delta();
    scene_time += dt;

    /* background movement */
    background_update(bgtheme);

    /* menu option */
    icon->position = font_get_position(stage_label[option]);
    icon->position.x += -20 + 3*cos(2*PI * scene_time);

    /* page number */
    pagenum = option/STAGE_MAXPERPAGE + 1;
    maxpages = stage_count/STAGE_MAXPERPAGE + ((stage_count%STAGE_MAXPERPAGE == 0) ? 0 : 1);
    font_set_text(page, lang_get("STAGESELECT_PAGE"), pagenum, maxpages);
    font_set_position(page, v2d_new(VIDEO_SCREEN_W - font_get_textsize(page).x - 10, font_get_position(page).y));

    /* music */
    if(state == STAGESTATE_PLAY) {
        if(!fadefx_is_fading()) {
            music_stop();
            music_unref(OPTIONS_MUSICFILE);
        }
    }
    else if(!music_is_playing()) {
        music_t *m = music_load(OPTIONS_MUSICFILE);
        music_play(m, INFINITY);
    }

    /* finite state machine */
    switch(state) {
        /* normal mode (menu) */
        case STAGESTATE_NORMAL: {
            if(!fadefx_is_fading()) {
                if(input_button_pressed(input, IB_DOWN)) {
                    option = (option+1) % stage_count;
                    sound_play( soundfactory_get("choose") );
                }
                if(input_button_pressed(input, IB_UP)) {
                    option = (((option-1) % stage_count) + stage_count) % stage_count;
                    sound_play( soundfactory_get("choose") );
                }
                if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                    logfile_message("Loading level \"%s\", \"%s\"", stage_data[option]->name, stage_data[option]->filepath);
                    level_setfile(stage_data[option]->filepath);
                    sound_play( soundfactory_get("select") );
                    state = STAGESTATE_PLAY;
                }
                if(input_button_pressed(input, IB_FIRE4)) {
                    sound_play( soundfactory_get("return") );
                    state = STAGESTATE_QUIT;
                }
            }
            break;
        }

        /* fade-out effect (quit this screen) */
        case STAGESTATE_QUIT: {
            if(fadefx_over()) {
                scenestack_pop();
                return;
            }
            fadefx_out(image_rgb(0,0,0), 1.0);
            break;
        }

        /* fade-out effect (play a level) */
        case STAGESTATE_PLAY: {
            if(fadefx_over()) {
                symboltable_clear(symboltable_get_global_table()); /* scripting: reset global variables */
                player_set_lives(PLAYER_INITIAL_LIVES);
                player_set_score(0);
                scenestack_push(storyboard_get_scene(SCENE_LEVEL));
                state = STAGESTATE_FADEIN;
                return;
            }
            fadefx_out(image_rgb(0,0,0), 1.0);
            break;
        }

        /* fade-in effect (after playing a level) */
        case STAGESTATE_FADEIN: {
            fadefx_in(image_rgb(0,0,0), 1.0);
            state = STAGESTATE_NORMAL;
            break;
        }
    }
}



/*
 * stageselect_render()
 * Renders the scene
 */
void stageselect_render()
{
    int i;
    v2d_t cam = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    background_render_bg(bgtheme, cam);
    background_render_fg(bgtheme, cam);

    font_render(title, cam);
    font_render(msg, cam);
    font_render(page, cam);

    for(i=0; i<stage_count; i++) {
        if(i/STAGE_MAXPERPAGE == option/STAGE_MAXPERPAGE) {
            if(stage_data[i]->act > 0 && !enable_debug)
                font_set_text(stage_label[i], (option==i) ? "<color=$COLOR_MENUSELECTEDOPTION>%s - %s %d</color>" : "%s - %s %d", stage_data[i]->name, lang_get("STAGESELECT_ACT"), stage_data[i]->act);
            else
                font_set_text(stage_label[i], (option==i) ? "<color=$COLOR_MENUSELECTEDOPTION>%s</color>" : "%s", stage_data[i]->name);
            font_render(stage_label[i], cam);
        }
    }

    actor_render(icon, cam);
}


/*
 * stageselect_enable_debug()
 * Enables or disables debug mode. When in debug mode, every installed level is displayed.
 * Call this before starting this scene.
 */
void stageselect_enable_debug(int enable)
{
    enable_debug = enable;
}




/* private methods */


/* loads the stage list from the level/ folder */
void load_stage_list()
{
    int i, j;
    int max_paths;
    char path[] = "levels/*.lev";
    char abs_path[2][1024];

    video_display_loading_screen();
    logfile_message("load_stage_list()");

    /* official and $HOME files */
    absolute_filepath(abs_path[0], path, sizeof(abs_path[0]));
    home_filepath(abs_path[1], path, sizeof(abs_path[1]));
    max_paths = (strcmp(abs_path[0], abs_path[1]) == 0) ? 1 : 2;

    /* loading data */
    stage_count = 0;
    for(j=0; j<max_paths; j++)
        foreach_file(abs_path[j], dirfill, NULL, enable_debug);
    qsort(stage_data, stage_count, sizeof(stagedata_t*), sort_cmp);

    /* fatal error */
    if(stage_count == 0)
        fatal_error("FATAL ERROR: no level files were found! Please reinstall the game.");
    else
        logfile_message("%d levels found.", stage_count);

    /* other stuff */
    stage_label = mallocx(stage_count * sizeof(font_t**));
    for(i=0; i<stage_count; i++) {
        stage_label[i] = font_create("menu.text");
        font_set_position(stage_label[i], v2d_new(25, 50 + 20 * (i % STAGE_MAXPERPAGE)));
    }
}



/* unloads the stage list */
void unload_stage_list()
{
    int i;

    logfile_message("unload_stage_list()");

    for(i=0; i<stage_count; i++) {
        font_destroy(stage_label[i]);
        stagedata_unload(stage_data[i]);
    }

    free(stage_label);
    stage_count = 0;
}


/* callback that fills stage_data[] */
int dirfill(const char *filename, void *param)
{
    int ver, subver, wipver;
    stagedata_t *s;

    /* can't have more than STAGE_MAX levels installed */
    if(stage_count >= STAGE_MAX)
        return 0;

    s = stagedata_load(filename);
    if(s != NULL) {
        ver = s->requires[0];
        subver = s->requires[1];
        wipver = s->requires[2];

        if(game_version_compare(ver, subver, wipver) >= 0) {
            stage_data[ stage_count++ ] = s;
            if(enable_debug) { /* debug mode: changing the names... */
                char abs_path[1024];
                absolute_filepath(abs_path, "levels/", sizeof(abs_path));
                snprintf(s->name, sizeof(s->name), "%s", s->filepath + strlen(abs_path));
            }
        }
        else {
            logfile_message("Warning: level file \"%s\" (requires: %d.%d.%d) isn't compatible with this version of the game (%d.%d.%d)", filename, ver, subver, wipver, GAME_VERSION, GAME_SUB_VERSION, GAME_WIP_VERSION);
            stagedata_unload(s);
        }
    }

    return 0;
}

/* comparator */
int sort_cmp(const void *a, const void *b)
{
    stagedata_t *s[2] = { *((stagedata_t**)a), *((stagedata_t**)b) };
    int r = str_icmp(s[0]->name, s[1]->name);
    return (r == 0) ? (s[0]->act - s[1]->act) : r;
}


/* stagedata_t constructor. Returns NULL if filename is not a level. */
stagedata_t* stagedata_load(const char *filename)
{
    parsetree_program_t *prog;
    stagedata_t* s = mallocx(sizeof *s);

    resource_filepath(s->filepath, (char*)filename, sizeof(s->filepath), RESFP_READ);
    strcpy(s->name, "Untitled");
    s->act = 1;
    s->requires[0] = 0;
    s->requires[1] = 0;
    s->requires[2] = 0;

    prog = nanoparser_construct_tree(s->filepath);
    nanoparser_traverse_program_ex(prog, (void*)s, traverse);
    prog = nanoparser_deconstruct_tree(prog);

    /* invalid level! */
    if(s->requires[0] <= 0 && s->requires[1] <= 0 && s->requires[2] <= 0) {
        logfile_message("Warning: load_stage_data(\"%s\") - invalid level. filepath: \"%s\"", filename, s->filepath);
        free(s);
        s = NULL;
    }

    return s;
}

/* stagedata_t destructor */
void stagedata_unload(stagedata_t *s)
{
    free(s);
}

/* traverses a line of the level */
int traverse(const parsetree_statement_t *stmt, void *stagedata)
{
    stagedata_t *s = (stagedata_t*)stagedata;
    const char *id = nanoparser_get_identifier(stmt);
    const parsetree_parameter_t *param_list = nanoparser_get_parameter_list(stmt);
    const char *val = nanoparser_get_string(nanoparser_get_nth_parameter(param_list, 1));

    if(str_icmp(id, "name") == 0)
        str_cpy(s->name, val, sizeof(s->name));
    else if(str_icmp(id, "act") == 0)
        s->act = atoi(val);
    else if(str_icmp(id, "requires") == 0)
        sscanf(val, "%d.%d.%d", &(s->requires[0]), &(s->requires[1]), &(s->requires[2]));
    else if(str_icmp(id, "brick") == 0 || str_icmp(id, "item") == 0 || str_icmp(id, "enemy") == 0 || str_icmp(id, "object") == 0) /* optimization */
        return 1; /* stop the enumeration */

    return 0;
}
