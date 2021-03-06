/*
 * Open Surge Engine
 * simulate_button.h - Simulates that an user is pressing (or not) a button of the input device of the observed player
 * Copyright (C) 2010, 2011  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _OD_SIMULATEBUTTON_H
#define _OD_SIMULATEBUTTON_H

#include "base/objectdecorator.h"
#include "../../core/nanocalc/nanocalc.h"

objectmachine_t* objectdecorator_simulatebuttondown_new(objectmachine_t *decorated_machine, const char *button_name);
objectmachine_t* objectdecorator_simulatebuttonup_new(objectmachine_t *decorated_machine, const char *button_name);

#endif

