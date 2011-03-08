/*
 * Open Surge Engine
 * confirmbox.h - confirm box
 * Copyright (C) 2008-2009  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _CONFIRMBOX_H
#define _CONFIRMBOX_H

/* public functions */
void confirmbox_init();
void confirmbox_release();
void confirmbox_update();
void confirmbox_render();

/* interface */
void confirmbox_alert(const char *ptext, const char *option1, const char *option2);
int confirmbox_selected_option();

#endif
