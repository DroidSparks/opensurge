/*
 * Open Surge Engine
 * bullet_trajectory.h - This decorator makes the object follow a bullet trajectory
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

#ifndef _OD_BULLETTRAJECTORY_H
#define _OD_BULLETTRAJECTORY_H

#include "base/objectdecorator.h"
#include "../../core/nanocalc/nanocalc.h"

/* speed_x and speed_y are given in pixels per second */
objectmachine_t* objectdecorator_bullettrajectory_new(objectmachine_t *decorated_machine, expression_t *speed_x, expression_t *speed_y);

#endif

