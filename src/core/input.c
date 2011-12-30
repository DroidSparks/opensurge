/*
 * Open Surge Engine
 * input.c - input management
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

#include <allegro.h>
#include "input.h"
#include "util.h"
#include "video.h"
#include "logfile.h"
#include "timer.h"

/* available devices */
typedef enum input_device_t input_device_t;
enum input_device_t {
    IT_KEYBOARD,
    IT_MOUSE,
    IT_COMPUTER,
    IT_JOYSTICK,
    IT_USER
};

/* input strcuture (private) */
struct input_t {
    input_device_t type; /* which input device? keyboard, mouse...? other? */
    int state[IB_MAX], oldstate[IB_MAX]; /* states */
    int x, y, z; /* mouse-related, cursor position */
    int dx, dy, dz; /* delta-x, delta-y, delta-z (mouse mickeys) */
    int keybmap[IB_MAX]; /* keyboard-related, key mappings */
    int enabled; /* enable input? */
};

typedef struct input_list_t input_list_t;
struct input_list_t {
    input_t *data;
    input_list_t *next;
};

/* private data */
static input_list_t *inlist;
static int got_joystick;
static int ignore_joystick;

/* private methods */
static void input_register(input_t *in);
static void input_unregister(input_t *in);
static void get_mouse_mickeys_ex(int *mickey_x, int *mickey_y, int *mickey_z);



/*
 * input_init()
 * Initializes the input module
 */
void input_init()
{
    logfile_message("input_init()");

    /* installing Allegro stuff */
    logfile_message("Installing Allegro input devices...");
    if(install_keyboard() != 0)
        logfile_message("install_keyboard() failed: %s", allegro_error);
    if(install_mouse() == -1)
        logfile_message("install_mouse() failed: %s", allegro_error);

    /* initializing */
    inlist = NULL;

    /* joystick */
    got_joystick = FALSE;
    ignore_joystick = TRUE;
    if(install_joystick(JOY_TYPE_AUTODETECT) == 0) {
        if(num_joysticks > 0 && joy[0].num_sticks > 0 && joy[0].stick[0].num_axis >= 2 && joy[0].num_buttons >= 4) {
            logfile_message("Joystick installed successfully!");
            got_joystick = TRUE;
        }
        else if(num_joysticks <= 0) {
            logfile_message("No joystick has been detected.");
        }
        else {
            logfile_message(
                "Invalid joystick! Please make sure your (digital) "
                "joystick has at least 4 buttons and 2 axis. "
                "num_joysticks: %d ; "
                "joy[0].num_sticks: %d ; "
                "joy[0].stick[0].num_axis: %d ; "
                "joy[0].num_buttons: %d",
                num_joysticks,
                joy[0].num_sticks,
                joy[0].num_sticks > 0 ? joy[0].stick[0].num_axis : 0,
                joy[0].num_buttons
            );
        }
    }
    else
        logfile_message("install_joystick() failed: %s", allegro_error);
}

/*
 * input_update()
 * Updates all the registered input objects
 */
void input_update()
{
    int i, lock_mouse = FALSE;
    static int old_f6 = 0;
    input_list_t *it;

    /* polling devices */
    if(keyboard_needs_poll())
        poll_keyboard();

    if(mouse_needs_poll())
        poll_mouse();

    if(input_joystick_available())
        poll_joystick();

    /* updating input objects */
    for(it = inlist; it; it=it->next) {

        /* updating the old states */
        for(i=0; i<IB_MAX; i++)
            it->data->oldstate[i] = it->data->state[i];

        /* checking the appropriate input device */
        switch(it->data->type) {
            case IT_KEYBOARD: {
                for(i=0; i<IB_MAX; i++)
                    it->data->state[i] = key[ it->data->keybmap[i] ];
                break;
            }

            case IT_MOUSE: {
                get_mouse_mickeys_ex(&it->data->dx, &it->data->dy, &it->data->dz);
                it->data->x = mouse_x;
                it->data->y = mouse_y;
                it->data->z = mouse_z;
                it->data->state[IB_UP] = (it->data->dz < 0);
                it->data->state[IB_DOWN] = (it->data->dz > 0);
                it->data->state[IB_LEFT] = FALSE;
                it->data->state[IB_RIGHT] = FALSE;
                it->data->state[IB_FIRE1] = (mouse_b & 1);
                it->data->state[IB_FIRE2] = (mouse_b & 2);
                it->data->state[IB_FIRE3] = (mouse_b & 4);
                it->data->state[IB_FIRE4] = FALSE;
                it->data->state[IB_FIRE5] = FALSE;
                it->data->state[IB_FIRE6] = FALSE;
                it->data->state[IB_FIRE7] = FALSE;
                it->data->state[IB_FIRE8] = FALSE;
                break;
            }

            case IT_JOYSTICK: {
                if(input_joystick_available()) {
                    it->data->state[IB_UP] = joy[0].stick[0].axis[1].d1;
                    it->data->state[IB_DOWN] = joy[0].stick[0].axis[1].d2;
                    it->data->state[IB_LEFT] = joy[0].stick[0].axis[0].d1;
                    it->data->state[IB_RIGHT] = joy[0].stick[0].axis[0].d2;
                    it->data->state[IB_FIRE1] = joy[0].button[0].b;
                    it->data->state[IB_FIRE2] = joy[0].button[1].b;
                    it->data->state[IB_FIRE3] = joy[0].button[2].b;
                    it->data->state[IB_FIRE4] = joy[0].button[3].b;
                    it->data->state[IB_FIRE5] = (joy[0].num_buttons >= 5) ? joy[0].button[4].b : FALSE;
                    it->data->state[IB_FIRE6] = (joy[0].num_buttons >= 6) ? joy[0].button[5].b : FALSE;
                    it->data->state[IB_FIRE7] = (joy[0].num_buttons >= 7) ? joy[0].button[6].b : FALSE;
                    it->data->state[IB_FIRE8] = (joy[0].num_buttons >= 8) ? joy[0].button[7].b : FALSE;
                }
                break;
            }

            case IT_USER: {
                for(i=0; i<IB_MAX; i++)
                    it->data->state[i] = key[ it->data->keybmap[i] ];
                if(input_joystick_available()) {
                    it->data->state[IB_UP] |= joy[0].stick[0].axis[1].d1;
                    it->data->state[IB_DOWN] |= joy[0].stick[0].axis[1].d2;
                    it->data->state[IB_LEFT] |= joy[0].stick[0].axis[0].d1;
                    it->data->state[IB_RIGHT] |= joy[0].stick[0].axis[0].d2;
                    it->data->state[IB_FIRE1] |= joy[0].button[0].b;
                    it->data->state[IB_FIRE2] |= joy[0].button[1].b;
                    it->data->state[IB_FIRE3] |= joy[0].button[2].b;
                    it->data->state[IB_FIRE4] |= joy[0].button[3].b;
                    it->data->state[IB_FIRE5] |= (joy[0].num_buttons >= 5) ? joy[0].button[4].b : FALSE;
                    it->data->state[IB_FIRE6] |= (joy[0].num_buttons >= 6) ? joy[0].button[5].b : FALSE;
                    it->data->state[IB_FIRE7] |= (joy[0].num_buttons >= 7) ? joy[0].button[6].b : FALSE;
                    it->data->state[IB_FIRE8] |= (joy[0].num_buttons >= 8) ? joy[0].button[7].b : FALSE;
                }
                break;
            }

            case IT_COMPUTER: {
                break;
            }
        }
    }

    /* lock mouse? */
    if(lock_mouse && video_is_window_active())
        position_mouse(SCREEN_W/2, SCREEN_H/2);

    /* ignore/restore joystick */
    if(!old_f6 && key[KEY_F6]) {
        input_ignore_joystick(!input_is_joystick_ignored());
        video_showmessage("%s joystick input", input_is_joystick_ignored() ? "Ignored" : "Restored");
    }
    old_f6 = key[KEY_F6];

    /* quit game */
    if(key[KEY_ALT] && key[KEY_F4])
        game_quit();
}


/*
 * input_release()
 * Releases the input module
 */
void input_release()
{
    input_list_t *it, *next;

    logfile_message("input_release()");
    for(it = inlist; it; it=next) {
        next = it->next;
        free(it->data);
        free(it);
    }
}


/*
 * input_button_down()
 * Checks if a given button is down
 */
int input_button_down(input_t *in, inputbutton_t button)
{
    return in->enabled ? in->state[(int)button] : FALSE;
}


/*
 * input_button_pressed()
 * Checks if a given button is pressed, not holded
 */
int input_button_pressed(input_t *in, inputbutton_t button)
{
    return in->enabled ? (in->state[(int)button] && !in->oldstate[(int)button]) : FALSE;
}


/*
 * input_button_up()
 * Checks if a given button is up
 */
int input_button_up(input_t *in, inputbutton_t button)
{
    return in->enabled ? (!in->state[(int)button] && in->oldstate[(int)button]) : FALSE;
}



/*
 * input_create_keyboard()
 * Creates an input object based on the keyboard
 *
 * keybmap: array of IB_MAX integers. Use NULL
 *          to use the default settings.
 *
 * keybmap_len: numer of elements of keybmap
 */
input_t *input_create_keyboard(int keybmap[], int keybmap_len)
{
    input_t *in = mallocx(sizeof *in);
    int i;

    in->type = IT_KEYBOARD;
    in->enabled = TRUE;
    in->dx = in->dy = in->x = in->y = 0;
    for(i=0; i<IB_MAX; i++)
        in->state[i] = in->oldstate[i] = FALSE;

    if(keybmap) {
        /* custom keyboard map */
        for(i=0; i<IB_MAX; i++)
            in->keybmap[i] = i < keybmap_len ? keybmap[i] : 0;
    }
    else {
        /* default settings */
        in->keybmap[IB_UP] = KEY_UP;
        in->keybmap[IB_DOWN] = KEY_DOWN;
        in->keybmap[IB_RIGHT] = KEY_RIGHT;
        in->keybmap[IB_LEFT] = KEY_LEFT;
        in->keybmap[IB_FIRE1] = KEY_SPACE;
        in->keybmap[IB_FIRE2] = KEY_LCONTROL;
        in->keybmap[IB_FIRE3] = KEY_ENTER;
        in->keybmap[IB_FIRE4] = KEY_ESC;
        in->keybmap[IB_FIRE5] = KEY_W;
        in->keybmap[IB_FIRE6] = KEY_A;
        in->keybmap[IB_FIRE7] = KEY_S;
        in->keybmap[IB_FIRE8] = KEY_D;
    }

    input_register(in);
    return in;
}




/* 
 * input_create_mouse()
 * Creates an input object based on the mouse
 */
input_t *input_create_mouse()
{
    input_t *in = mallocx(sizeof *in);
    int i;

    in->type = IT_MOUSE;
    in->enabled = TRUE;
    in->dx = in->dy = in->x = in->y = 0;
    for(i=0; i<IB_MAX; i++)
        in->state[i] = in->oldstate[i] = FALSE;

    input_register(in);
    return in;
}




/*
 * input_create_computer()
 * Creates an object that receives "input" from
 * the computer
 */
input_t *input_create_computer()
{
    input_t *in = mallocx(sizeof *in);
    int i;

    in->type = IT_COMPUTER;
    in->enabled = TRUE;
    in->dx = in->dy = in->x = in->y = 0;
    for(i=0; i<IB_MAX; i++)
        in->state[i] = in->oldstate[i] = FALSE;

    input_register(in);
    return in;
}


/*
 * input_create_joystick()
 * Creates an object that receives input from
 * a joystick
 */
input_t *input_create_joystick()
{
    input_t *in;
    int i;

    if(!input_joystick_available()) {
        logfile_message("WARNING: called input_create_joystick(), but no joystick is available!");
        return NULL;
    }

    in = mallocx(sizeof *in);
    in->type = IT_JOYSTICK;
    in->enabled = TRUE;
    in->dx = in->dy = in->x = in->y = 0;
    for(i=0; i<IB_MAX; i++)
        in->state[i] = in->oldstate[i] = FALSE;

    input_register(in);
    return in;
}


/*
 * input_create_user()
 * Creates an user's custom input device
 */
input_t *input_create_user()
{
    input_t *in;
    int i;

    /* initializing */
    in = mallocx(sizeof *in);
    in->type = IT_USER;
    in->enabled = TRUE;
    in->dx = in->dy = in->x = in->y = 0;
    for(i=0; i<IB_MAX; i++)
        in->state[i] = in->oldstate[i] = FALSE;

    /* default settings (keyboard) */
    in->keybmap[IB_UP] = KEY_UP;
    in->keybmap[IB_DOWN] = KEY_DOWN;
    in->keybmap[IB_RIGHT] = KEY_RIGHT;
    in->keybmap[IB_LEFT] = KEY_LEFT;
    in->keybmap[IB_FIRE1] = KEY_SPACE;
    in->keybmap[IB_FIRE2] = KEY_LCONTROL;
    in->keybmap[IB_FIRE3] = KEY_ENTER;
    in->keybmap[IB_FIRE4] = KEY_ESC;
    in->keybmap[IB_FIRE5] = KEY_W;
    in->keybmap[IB_FIRE6] = KEY_A;
    in->keybmap[IB_FIRE7] = KEY_S;
    in->keybmap[IB_FIRE8] = KEY_D;

    /* done! */
    input_register(in);
    return in;
}


/*
 * input_destroy()
 * Destroys an input object
 */
void input_destroy(input_t *in)
{
    input_unregister(in);
    free(in);
}


/*
 * input_ignore()
 * Ignore Control
 */
void input_ignore(input_t *in)
{
    in->enabled = FALSE;
}


/*
 * input_restore()
 * Restore Control
 */
void input_restore(input_t *in)
{
    in->enabled = TRUE;
}



/*
 * input_is_ignored()
 * Returns TRUE if the input is ignored,
 * or FALSE otherwise
 */
int input_is_ignored(input_t *in)
{
    return !in->enabled;
}




/*
 * input_clear()
 * Clears all the input buttons
 */
void input_clear(input_t *in)
{
    int i;
    for(i=0; i<IB_MAX; i++)
        in->state[i] = in->oldstate[i] = FALSE;
}



/*
 * input_simulate_button_down()
 * Useful for computer-controlled input objects
 */
void input_simulate_button_down(input_t *in, inputbutton_t button)
{
    in->state[(int)button] = TRUE;
}



/*
 * input_simulate_button_up()
 * Useful for computer-controlled input objects
 */
void input_simulate_button_up(input_t *in, inputbutton_t button)
{
    in->state[(int)button] = FALSE;
}



/*
 * input_joystick_available()
 * Is a joystick available?
 */
int input_joystick_available()
{
    return got_joystick && !ignore_joystick;
}


/*
 * input_ignore_joystick()
 * Ignores the input received from a joystick (if available)
 */
void input_ignore_joystick(int ignore)
{
    ignore_joystick = ignore;
}


/*
 * input_is_joystick_ignored()
 * Is the joystick input ignored?
 */
int input_is_joystick_ignored()
{
    return ignore_joystick;
}


/*
 * input_get_xy()
 * Gets the xy coordinates (mouse-related routine)
 */
v2d_t input_get_xy(input_t *in)
{
    return v2d_new(in->x, in->y);
}



/* private methods */


/* registers an input device */
void input_register(input_t *in)
{
    input_list_t *node = mallocx(sizeof *node);

    node->data = in;
    node->next = inlist;
    inlist = node;
}

/* unregisters the given input device */
void input_unregister(input_t *in)
{
    input_list_t *node, *next;

    if(inlist->data == in) {
        next = inlist->next;
        free(inlist);
        inlist = next;
    }
    else {
        node = inlist;
        while(node->next && node->next->data != in)
            node = node->next;
        if(node->next) {
            next = node->next->next; 
            free(node->next);
            node->next = next;
        }
    }
}


/* get mouse mickeys (mouse wheel included) */
void get_mouse_mickeys_ex(int *mickey_x, int *mickey_y, int *mickey_z)
{
    get_mouse_mickeys(mickey_x, mickey_y);
    if(mickey_z != NULL) {
        static int mz, first = TRUE;
        if(first) { mz = mouse_z; first = FALSE; }
        *mickey_z = mouse_z - mz;
        mz = mouse_z;
    }
}
