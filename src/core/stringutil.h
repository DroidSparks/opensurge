/*
 * Open Surge Engine
 * stringutil.h - string utilities
 * Copyright (C) 2010, 2013  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

#include <stdlib.h>

int str_to_hash(const char *str); /* generates a hash key */
const char* str_to_upper(const char *str); /* returns a pointer to a static variable */
const char* str_to_lower(const char *str); /* returns a pointer to a static variable */
int str_icmp(const char *s1, const char *s2); /* case-insensitive compare function */
char* str_cpy(char *dest, const char *src, size_t dest_size); /* safe version of strcpy() */
void str_trim(char *dest, const char *src); /* trim */
char *str_dup(const char *str); /* duplicates a string - strdup() isn't ANSI C */
const char* str_addslashes(const char *str); /* replaces " by \\", returning a static char* */
char* str_rstr(char *haystack, const char *needle); /* reverse strstr() */
const char* str_from_int(int integer); /* converts integer to string, returning a static char* */

#endif

