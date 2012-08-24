/*
 * Open Surge Engine
 * osspec.h - OS Specific Routines
 * Copyright (C) 2009, 2012  Alexandre Martins <alemartf(at)gmail(dot)com>
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


/* resource_filepath() modes */
#define RESFP_READ                            0
#define RESFP_WRITE                           1


/* public functions */
void osspec_init(); /* call this before everything else */
void osspec_release(); /* call this after everything else */
int filepath_exists(const char *filepath);
int directory_exists(const char *dirpath);
void absolute_filepath(char *dest, const char *relativefp, size_t dest_size);
void home_filepath(char *dest, const char *relativefp, size_t dest_size);
void resource_filepath(char *dest, const char *relativefp, size_t dest_size, int resfp_mode);
void create_process(const char *path, int argc, char *argv[]);
char* basename(const char *path);
int foreach_file(const char *wildcard, int (*callback)(const char *filename, void *param), void *param, int recursive);
int launch_url(const char *url); /* launches an URL: returns TRUE on success */


#endif
