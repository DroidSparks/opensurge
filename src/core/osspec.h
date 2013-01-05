/*
 * Open Surge Engine
 * osspec.h - OS Specific Routines
 * Copyright (C) 2009, 2012-2013  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _OSSPEC_H
#define _OSSPEC_H

#include <stdlib.h>

/* engine functions */
void osspec_init(const char *basedir); /* call this before everything else. You may pass NULL to basedir. */
void osspec_release(); /* call this after everything else */

/* resource access. Resources are stored either in the game folder, or in the home folder (*nix). */
typedef enum { RESFP_READ, RESFP_WRITE } resfp_t; /* do you want to access the resource for writing or for reading? */
void resource_filepath(char *dest, const char *relativefp, size_t dest_size, resfp_t mode); /* returns the absolute path of relativefp */
int foreach_resource(const char *wildcard, int (*callback)(const char *filename, void *param), void *param, int recursive); /* filename is an absolute filepath */

/* simple file access */
int filepath_exists(const char *filepath); /* does the given (absolute) filepath exist? */
char* basename(const char *path); /* basename of path */
int launch_url(const char *url); /* launches an URL: returns TRUE on success */

#endif
