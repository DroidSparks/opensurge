/*
 * Open Surge Engine
 * soundfactory.h - sound factory
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

#ifndef _SOUNDFACTORY_H
#define _SOUNDFACTORY_H

#include "audio.h"

/* initializes the sound factory */
void soundfactory_init();

/* releases the sound factory */
void soundfactory_release();

/* given a sound name, returns the corresponding sound effect */
sound_t *soundfactory_get(const char *sound_name);

#endif
