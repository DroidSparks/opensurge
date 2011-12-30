/*
 * Open Surge Engine
 * screenshot.c - screenshots module
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

#include <allegro.h>
#include <stdio.h>
#include "screenshot.h"
#include "osspec.h"
#include "logfile.h"
#include "video.h"
#include "input.h"

/* private data */
static input_t *in;
static char *next_available_filename();

/*
 * screenshot_init()
 * Initializes the screenshot module
 */
void screenshot_init()
{
    int m[IB_MAX];

    m[IB_UP] = m[IB_DOWN] = m[IB_LEFT] = m[IB_RIGHT] = KEY_A; /* whatever */
    m[IB_FIRE3] = m[IB_FIRE4] = KEY_A;
    m[IB_FIRE1] = KEY_EQUALS;
    m[IB_FIRE2] = KEY_PRTSCR;

    in = input_create_keyboard(m);
}


/*
 * screenshot_update()
 * Checks if the user wants to take a snapshot, and if
 * he/she does, we must do it.
 */
void screenshot_update()
{
    /* take the snapshot! (press the '=' key or the 'printscreen' key) */
    if(input_button_pressed(in, IB_FIRE1) || input_button_pressed(in, IB_FIRE2)) {
        char *file = next_available_filename();
        image_save(video_get_window_surface(), file);
        video_showmessage("'screenshots/%s' saved", basename(file));
        logfile_message("New screenshot: %s", file);
    }
}


/*
 * screenshot_release()
 * Releases this module
 */
void screenshot_release()
{
    input_destroy(in);
}






/* misc */
char *next_available_filename()
{
    static char f[64], abs_path[1024];
    int i;

    for(i=0;;i++) {
        sprintf(f, "screenshots/s%03d.png", i);
        resource_filepath(abs_path, f, sizeof(abs_path), RESFP_WRITE);
        if(!filepath_exists(abs_path))
            return abs_path;
    }
}
