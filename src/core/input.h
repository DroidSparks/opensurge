/*
 * Open Surge Engine
 * input.h - input management
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

#ifndef _INPUT_H
#define _INPUT_H

#include "v2d.h"

/* forward declarations */
typedef enum inputbutton_t inputbutton_t;
typedef struct input_t input_t;

/* available buttons */
#define IB_MAX              12   /* number of buttons */
enum inputbutton_t {
    IB_UP,      /* up */
    IB_DOWN,    /* down */
    IB_RIGHT,   /* right */
    IB_LEFT,    /* left */
    IB_FIRE1,   /* jump */
    IB_FIRE2,   /* switch character */
    IB_FIRE3,   /* pause */
    IB_FIRE4,   /* quit */
    IB_FIRE5,
    IB_FIRE6,
    IB_FIRE7,
    IB_FIRE8
};

/* public methods */
void input_init();
void input_update();
void input_release();
int input_joystick_available(); /* a joystick is available AND the user wants to use it */
void input_ignore_joystick(int ignore); /* ignores the input received from a joystick (if available) */
int input_is_joystick_ignored();

input_t *input_create_computer(); /* computer-controlled "input" */
input_t *input_create_keyboard(int keybmap[], int keybmap_len); /* keyboard */
input_t *input_create_mouse(); /* mouse */
input_t *input_create_joystick(); /* joystick */
input_t *input_create_user(); /* user's custom input device */
void input_destroy(input_t *in);

int input_button_down(input_t *in, inputbutton_t button);
int input_button_pressed(input_t *in, inputbutton_t button);
int input_button_up(input_t *in, inputbutton_t button);
void input_simulate_button_down(input_t *in, inputbutton_t button);
void input_simulate_button_up(input_t *in, inputbutton_t button);
void input_ignore(input_t *in);
void input_restore(input_t *in);
int input_is_ignored(input_t *in);
void input_clear(input_t *in);
v2d_t input_get_xy(input_t *in);

#endif
