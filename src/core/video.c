/*
 * Open Surge Engine
 * video.c - video manager
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

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <png.h>
#include <allegro.h>
#include <loadpng.h>
#include <jpgalleg.h>
#include "2xsai/2xsai.h"
#include "video.h"
#include "timer.h"
#include "logfile.h"
#include "util.h"




/* private stuff */
#define IMAGE2BITMAP(img)       (*((BITMAP**)(img)))   /* whoooa, this is crazy stuff */


/* video manager */
#define FILTER_2XSAI            0
#define FILTER_SUPEREAGLE       1
static image_t *video_buffer;
static image_t *window_surface, *window_surface_half;
static int video_smooth;
static int video_resolution;
static int video_fullscreen;
static int video_showfps;
static void fast2x_blit(image_t *src, image_t *dest);
static void filter_blit(image_t *src, image_t *dest, int filter);
static void window_switch_in();
static void window_switch_out();
static int window_active = TRUE;
static void draw_to_screen(image_t *img);
static void setup_color_depth(int bpp);

/* playarea video size (usually, 320x240) */
static const v2d_t default_playarea_size = { 320, 240 }; /* this is set on stone! TODO: load these parameters from a file? */
static v2d_t playarea_size = { 320, 240 }; /* represents the size of the playarea. This may change (eg, is the user on the level editor?) */

/* Fade-in & fade-out */
#define FADEFX_NONE            0
#define FADEFX_IN              1
#define FADEFX_OUT             2
static int fadefx_type;
static int fadefx_end;
static uint32 fadefx_color;
static float fadefx_elapsed_time;
static float fadefx_total_time;

/* Video Message */
#define VIDEOMSG_TIMEOUT       5000
static uint32 videomsg_endtime;
static char videomsg_data[512];

/* Loading screen */
#define LOADINGSCREEN_FILE     "images/loading.png"



/* video manager */

/*
 * video_init()
 * Initializes the video manager
 */
void video_init(const char *window_title, int resolution, int smooth, int fullscreen, int bpp)
{
    logfile_message("video_init()");
    setup_color_depth(bpp);

    /* initializing addons */
    logfile_message("Initializing JPGalleg...");
    jpgalleg_init();
    logfile_message("Initializing loadpng...");
    loadpng_init();

    /* video init */
    video_buffer = NULL;
    window_surface = window_surface_half = NULL;
    video_changemode(resolution, smooth, fullscreen);

    /* window properties */
    LOCK_FUNCTION(game_quit);
    set_close_button_callback(game_quit);
    set_window_title(window_title);

    /* window callbacks */
    window_active = TRUE;
    if(set_display_switch_mode(SWITCH_BACKGROUND) == 0) {
        if(set_display_switch_callback(SWITCH_IN, window_switch_in) != 0)
            logfile_message("can't set_display_switch_callback(SWTICH_IN, window_switch_in)");

        if(set_display_switch_callback(SWITCH_OUT, window_switch_out) != 0)
            logfile_message("can't set_display_switch_callback(SWTICH_OUT, window_switch_out)");
    }
    else
        logfile_message("can't set_display_switch_mode(SWITCH_BACKGROUND)");

    /* video message */
    videomsg_endtime = 0;
}

/*
 * video_changemode()
 * Sets up the game window
 */
void video_changemode(int resolution, int smooth, int fullscreen)
{
    int width, height;
    int mode;

    logfile_message("video_changemode(%d,%d,%d)", resolution, smooth, fullscreen);

    /* resolution */
    playarea_size = (resolution == VIDEORESOLUTION_EDT) ? video_get_window_size() : default_playarea_size;
    video_resolution = resolution;

    /* fullscreen */
    video_fullscreen = fullscreen;

    /* smooth graphics? */
    video_smooth = smooth;
    if(video_smooth) {
        if(video_get_color_depth() == 8) {
            logfile_message("can't use smooth graphics in the 256 color mode (8 bpp)");
            video_smooth = FALSE;
        }
        else if(video_resolution == VIDEORESOLUTION_1X || video_resolution == VIDEORESOLUTION_3X || video_resolution == VIDEORESOLUTION_EDT) {
            logfile_message("can't use smooth graphics using resolution %d", video_resolution);
            video_smooth = FALSE;
        }
        else {
            logfile_message("initializing 2xSaI...");
            Init_2xSaI(video_get_color_depth());
        }
    }

    /* creating the backbuffer... */
    logfile_message("creating the backbuffer...");
    if(video_buffer != NULL)
        image_destroy(video_buffer);
    video_buffer = image_create(VIDEO_SCREEN_W, VIDEO_SCREEN_H);
    image_clear(video_buffer, image_rgb(0,0,0));

    /* creating the window surface... */
    logfile_message("creating the window surface...");
    if(window_surface != NULL)
        image_destroy(window_surface);
    window_surface = image_create((int)(video_get_window_size().x), (int)(video_get_window_size().y));
    image_clear(window_surface, image_rgb(0,0,0));

    logfile_message("creating the auxiliary window surface...");
    if(window_surface_half != NULL)
        image_destroy(window_surface_half);
    window_surface_half = image_create(IMAGE2BITMAP(window_surface)->w/2, IMAGE2BITMAP(window_surface)->h/2);
    image_clear(window_surface_half, image_rgb(0,0,0));

    /* setting up the window... */
    logfile_message("setting up the window...");
    mode = video_fullscreen ? GFX_AUTODETECT : GFX_AUTODETECT_WINDOWED;
    width = (int)(video_get_window_size().x);
    height = (int)(video_get_window_size().y);
    if(set_gfx_mode(mode, width, height, 0, 0) < 0)
        fatal_error("video_changemode(): couldn't set the graphic mode (%dx%d)!\n%s", width, height, allegro_error);

    /* done! */
    logfile_message("video_changemode() ok");
}


/*
 * video_get_resolution()
 * Returns the current resolution value,
 * i.e., VIDEORESOLUTION_*
 */
int video_get_resolution()
{
    return video_resolution;
}


/*
 * video_is_smooth()
 * Smooth graphics?
 */
int video_is_smooth()
{
    return video_smooth;
}


/*
 * video_is_fullscreen()
 * Fullscreen mode?
 */
int video_is_fullscreen()
{
    return video_fullscreen;
}


/*
 * video_get_playarea_size()
 * Returns the size of the playarea.
 * Usually, it's 320x240
 */
v2d_t video_get_playarea_size()
{
    return playarea_size;
}


/*
 * video_get_window_size()
 * Returns the window size, based on
 * the current resolution
 */
v2d_t video_get_window_size()
{
    int width=VIDEO_SCREEN_W, height=VIDEO_SCREEN_H;

    switch(video_resolution) {
        case VIDEORESOLUTION_1X:
            width = VIDEO_SCREEN_W;
            height = VIDEO_SCREEN_H;
            break;

        case VIDEORESOLUTION_2X:
            width = 2*VIDEO_SCREEN_W;
            height = 2*VIDEO_SCREEN_H;
            break;

        case VIDEORESOLUTION_3X:
            width = 3*VIDEO_SCREEN_W;
            height = 3*VIDEO_SCREEN_H;
            break;

        case VIDEORESOLUTION_4X:
            width = 4*VIDEO_SCREEN_W;
            height = 4*VIDEO_SCREEN_H;
            break;

        /*case VIDEORESOLUTION_MAX: {
            int dw, dh;
            if(get_desktop_resolution(&dw, &dh) == 0) {
                int scale = min((int)(dw/VIDEO_SCREEN_W), (int)(dh/VIDEO_SCREEN_H));
                width = scale*VIDEO_SCREEN_W;
                height = scale*VIDEO_SCREEN_H;
            }
            else {
                width = VIDEO_SCREEN_W;
                height = VIDEO_SCREEN_H;
            }
            break;
        }*/

        case VIDEORESOLUTION_EDT:
            width = VIDEO_SCREEN_W;
            height = VIDEO_SCREEN_H;
            break;

        default:
            fatal_error("video_get_window_size(): unknown resolution!");
            break;
    }

    return v2d_new(width, height);
}


/*
 * video_get_backbuffer()
 * Returns a pointer to the backbuffer
 */
image_t* video_get_backbuffer()
{
    if(video_buffer == NULL)
        fatal_error("FATAL ERROR: video_get_backbuffer() returned NULL!");

    return video_buffer;
}

/*
 * video_render()
 * Updates the video manager and the screen
 */
void video_render()
{
    /* fade effect */
    fadefx_end = FALSE;
    if(fadefx_type != FADEFX_NONE) {
        fadefx_elapsed_time += timer_get_delta();
        if(fadefx_elapsed_time < fadefx_total_time) {
            if(video_get_color_depth() > 8) {
                /* true-color fade effect */
                int n;

                n = (int)( (float)255 * (fadefx_elapsed_time*1.25 / fadefx_total_time) );
                n = clip(n, 0, 255);
                n = (fadefx_type == FADEFX_IN) ? 255-n : n;

                drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
                set_trans_blender(0, 0, 0, n);
                rectfill(IMAGE2BITMAP(video_get_backbuffer()), 0, 0, VIDEO_SCREEN_W, VIDEO_SCREEN_H, fadefx_color);
                solid_mode();
            }
            else {
                /* 256-color fade effect */
                ;
            }
        }
        else {
            /* the fade effect is over */
            fadefx_end = TRUE;

            /* fade-out improvements */
            if(fadefx_type == FADEFX_OUT)
                rectfill(IMAGE2BITMAP(video_get_backbuffer()), 0, 0, VIDEO_SCREEN_W, VIDEO_SCREEN_H, fadefx_color);

            /* reset the fade effect: a quick hack */
            fadefx_type = FADEFX_NONE;
            fadefx_total_time = fadefx_elapsed_time = 0;
            fadefx_color = 0;
        }
    }



    /* video message */
    if(timer_get_ticks() < videomsg_endtime)
        textout_ex(IMAGE2BITMAP(video_get_backbuffer()), font, videomsg_data, 0, VIDEO_SCREEN_H-text_height(font), makecol(255,255,255), makecol(0,0,0));


    /* fps counter */
    if(video_is_fps_visible())
        textprintf_right_ex(IMAGE2BITMAP(video_get_backbuffer()), font, VIDEO_SCREEN_W, 0, makecol(255,255,255), makecol(0,0,0),"FPS:%3d", timer_get_fps());


    /* render */
    switch(video_get_resolution()) {
        /* tiny window */
        case VIDEORESOLUTION_1X:
        {
            draw_to_screen(video_get_backbuffer());
            break;
        }

        /* double size */
        case VIDEORESOLUTION_2X:
        {
            image_t *tmp = window_surface;

            if(video_is_smooth())
                filter_blit(video_get_backbuffer(), tmp, FILTER_2XSAI);
            else
                fast2x_blit(video_get_backbuffer(), tmp);

            draw_to_screen(tmp);
            break;
        }

        /* triple size */
        case VIDEORESOLUTION_3X:
        {
            image_t *tmp = window_surface;
            v2d_t scale = v2d_new((float)IMAGE2BITMAP(tmp)->w / (float)IMAGE2BITMAP(video_get_backbuffer())->w, (float)IMAGE2BITMAP(tmp)->h / (float)IMAGE2BITMAP(video_get_backbuffer())->h);
            image_draw_scaled(video_get_backbuffer(), tmp, 0, 0, scale, IF_NONE);
            draw_to_screen(tmp);
            break;
        }

        /* quadruple size */
        case VIDEORESOLUTION_4X:
        {
            image_t *tmp = window_surface;
            image_t *half = window_surface_half;

            if(video_is_smooth()) {
                /*filter_blit(video_get_backbuffer(), half, FILTER_2XSAI);*/
                fast2x_blit(video_get_backbuffer(), half);
                filter_blit(half, tmp, FILTER_2XSAI);
            }
            else {
                fast2x_blit(video_get_backbuffer(), half);
                fast2x_blit(half, tmp);
            }

            draw_to_screen(tmp);
            break;
        }

        /* maximum size */
        /*case VIDEORESOLUTION_MAX:
        {
            image_t *tmp = window_surface;

            if(video_is_smooth() && tmp->w >= 2*VIDEO_SCREEN_W && tmp->h >= 2*VIDEO_SCREEN_H) {
                image_t *half = window_surface_half;
                v2d_t scale = v2d_new((float)half->w / (float)video_get_backbuffer()->w, (float)half->h / (float)video_get_backbuffer()->h);
                image_draw_scaled(video_get_backbuffer(), half, 0, 0, scale, IF_NONE);
                filter_blit(half, tmp, FILTER_2XSAI);
            }
            else {
                v2d_t scale = v2d_new((float)tmp->w / (float)video_get_backbuffer()->w, (float)tmp->h / (float)video_get_backbuffer()->h);
                image_draw_scaled(video_get_backbuffer(), tmp, 0, 0, scale, IF_NONE);
            }

            draw_to_screen(tmp);
            break;
        }*/

        /* level editor */
        case VIDEORESOLUTION_EDT:
        {
            draw_to_screen(video_get_backbuffer());
            break;
        }
    }
}


/*
 * video_release()
 * Releases the video manager
 */
void video_release()
{
    logfile_message("video_release()");

    if(video_buffer != NULL)
        image_destroy(video_buffer);

    if(window_surface != NULL)
        image_destroy(window_surface);

    if(window_surface_half != NULL)
        image_destroy(window_surface_half);

    logfile_message("video_release() ok");
}


/*
 * video_showmessage()
 * Shows a text message to the user
 */
void video_showmessage(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(videomsg_data, fmt, args);
    va_end(args);

    videomsg_endtime = timer_get_ticks() + VIDEOMSG_TIMEOUT;
}


/*
 * video_get_color_depth()
 * Returns the current color depth
 */
int video_get_color_depth()
{
    return get_color_depth();
}


/*
 * video_get_desktop_color_depth()
 * Returns the default color depth of the user
 */
int video_get_desktop_color_depth()
{
    return desktop_color_depth();
}


/*
 * video_is_window_active()
 * Returns TRUE if the game window is active,
 * or FALSE otherwise
 */
int video_is_window_active()
{
    return window_active;
}



/*
 * video_get_maskcolor()
 * Returns the mask color
 */
uint32 video_get_maskcolor()
{
    switch(video_get_color_depth()) {
        case 8:  return MASK_COLOR_8;
        case 16: return MASK_COLOR_16;
        case 24: return MASK_COLOR_24;
        case 32: return MASK_COLOR_32;
    }

    return MASK_COLOR_16;
}


/*
 * video_show_fps()
 * Shows/hides the FPS counter
 */
void video_show_fps(int show)
{
    video_showfps = show;
}


/*
 * video_is_fps_visible()
 * Is the FPS counter visible?
 */
int video_is_fps_visible()
{
    return video_showfps;
}


/*
 * video_display_loading_screen()
 * Displays a loading screen
 */
void video_display_loading_screen()
{
    image_t *img = image_load(LOADINGSCREEN_FILE);
    image_blit(img, video_get_backbuffer(), 0, 0, 0, 0, IMAGE2BITMAP(img)->w, IMAGE2BITMAP(img)->h);
    image_unref(LOADINGSCREEN_FILE);

    video_render();
}


/*
 * video_get_window_surface()
 * The window surface (read-only)
 */
const image_t* video_get_window_surface()
{
    switch(video_get_resolution()) {
        case VIDEORESOLUTION_1X:
        case VIDEORESOLUTION_EDT:
            return video_get_backbuffer(); /* this "gambiarra" saves some processing... */

        default:
            return window_surface;
    }
}


/* private stuff */

/* filter_blit() applies the graphic filter
 * filter, fixing the possible defects of the
 * resulting image.
 *
 * if (filter == 2xsai) or (filter == superagle):
 * -- we assume that:
 * ---- IMAGE2BITMAP(dest)->w = 2 * src->w
 * ---- IMAGE2BITMAP(dest)->h = 2 * src->h
 */
void filter_blit(image_t *src, image_t *dest, int filter)
{
    int i, j;
    const int k=2;

    if(IMAGE2BITMAP(src) == NULL || IMAGE2BITMAP(dest) == NULL)
        return;

    switch(filter) {
        case FILTER_2XSAI:
            Super2xSaI(IMAGE2BITMAP(src), IMAGE2BITMAP(dest), 0, 0, 0, 0, IMAGE2BITMAP(src)->w, IMAGE2BITMAP(src)->h);
            for(i=0; i<IMAGE2BITMAP(dest)->h; i++) { /* image fix */
                for(j=0; j<k; j++)
                    _putpixel(IMAGE2BITMAP(dest), j, i, _getpixel(IMAGE2BITMAP(dest), k, i));
            }
            break;

        case FILTER_SUPEREAGLE:
            SuperEagle(IMAGE2BITMAP(src), IMAGE2BITMAP(dest), 0, 0, 0, 0, IMAGE2BITMAP(src)->w, IMAGE2BITMAP(src)->h);
            for(i=0; i<IMAGE2BITMAP(dest)->h; i++) { /* image fix */
                for(j=0; j<k; j++)
                    _putpixel(IMAGE2BITMAP(dest), IMAGE2BITMAP(dest)->w-1-j, i, _getpixel(IMAGE2BITMAP(dest), IMAGE2BITMAP(dest)->w-1-k, i));
            }
            break;
    }
}

/* fast2x_blit resizes the src image by a
 * factor of 2. It assumes that:
 *
 * src is a memory bitmap
 * dest is a previously created memory bitmap
 * IMAGE2BITMAP(dest)->w == 2 * src->w
 * IMAGE2BITMAP(dest)->h == 2 * src->h */
void fast2x_blit(image_t *src, image_t *dest)
{
    int i, j;

    if(IMAGE2BITMAP(src) == NULL || IMAGE2BITMAP(dest) == NULL)
        return;

    switch(video_get_color_depth())
    {
        case 8:
            for(j=0; j<IMAGE2BITMAP(dest)->h; j++) {
                for(i=0; i<IMAGE2BITMAP(dest)->w; i++)
                    ((uint8*)IMAGE2BITMAP(dest)->line[j])[i] = ((uint8*)IMAGE2BITMAP(src)->line[j/2])[i/2];
            }
            break;

        case 16:
            for(j=0; j<IMAGE2BITMAP(dest)->h; j++) {
                for(i=0; i<IMAGE2BITMAP(dest)->w; i++)
                    ((uint16*)IMAGE2BITMAP(dest)->line[j])[i] = ((uint16*)IMAGE2BITMAP(src)->line[j/2])[i/2];
            }
            break;

        case 24:
            /* TODO */
            stretch_blit(IMAGE2BITMAP(src), IMAGE2BITMAP(dest), 0, 0, IMAGE2BITMAP(src)->w, IMAGE2BITMAP(src)->h, 0, 0, IMAGE2BITMAP(dest)->w, IMAGE2BITMAP(dest)->h);
            break;

        case 32:
            for(j=0; j<IMAGE2BITMAP(dest)->h; j++) {
                for(i=0; i<IMAGE2BITMAP(dest)->w; i++)
                    ((uint32*)IMAGE2BITMAP(dest)->line[j])[i] = ((uint32*)IMAGE2BITMAP(src)->line[j/2])[i/2];
            }
            break;

        default:
            break;
    }
}


/* draws img to the screen */
void draw_to_screen(image_t *img)
{
    if(IMAGE2BITMAP(img) == NULL) {
        logfile_message("Can't use video resolution %d", video_get_resolution());
        video_showmessage("Can't use video resolution %d", video_get_resolution());
        video_changemode(VIDEORESOLUTION_2X, video_is_smooth(), video_is_fullscreen());
    }
    else
        blit(IMAGE2BITMAP(img), screen, 0, 0, 0, 0, IMAGE2BITMAP(img)->w, IMAGE2BITMAP(img)->h);
}

/* this window is active */
void window_switch_in()
{
    window_active = TRUE;
}


/* this window is not active */
void window_switch_out()
{
    window_active = FALSE;
}

/* setups the color depth */
void setup_color_depth(int bpp)
{
    set_color_depth(bpp);

    if(bpp == 8)
        set_color_conversion(COLORCONV_REDUCE_TO_256 | COLORCONV_DITHER_PAL);
    else
        set_color_conversion(COLORCONV_TOTAL);
}



/* fade effects */

/*
 * fadefx_in()
 * Fade-in effect
 */
void fadefx_in(uint32 color, float seconds)
{
    if(fadefx_type == FADEFX_NONE) {
        fadefx_type = FADEFX_IN;
        fadefx_end = FALSE;
        fadefx_color = color;
        fadefx_elapsed_time = 0;
        fadefx_total_time = seconds;
    }
}


/*
 * fadefx_out()
 * Fade-out effect
 */
void fadefx_out(uint32 color, float seconds)
{
    if(fadefx_type == FADEFX_NONE) {
        fadefx_type = FADEFX_OUT;
        fadefx_end = FALSE;
        fadefx_color = color;
        fadefx_elapsed_time = 0;
        fadefx_total_time = seconds;
    }
}



/*
 * fadefx_over()
 * Asks if the fade effect has ended
 * (only one action when this event loops)
 */
int fadefx_over()
{
    return fadefx_end;
}


/*
 * fadefx_is_fading()
 * Is the fade effect ocurring?
 */
int fadefx_is_fading()
{
    return (fadefx_type != FADEFX_NONE);
}

