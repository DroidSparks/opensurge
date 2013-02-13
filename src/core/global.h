/*
 * Open Surge Engine
 * global.h - global definitions
 * Copyright (C) 2008-2013  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _GLOBAL_H
#define _GLOBAL_H

/* if this is defined, set it to the bleeding edge release */
/* otherwise, then we assume we're compiling the stable release */
/*#define GAME_BUILD_VERSION      1337*/

/* Game data */
#define GAME_UNIXNAME           "opensurge"
#define GAME_TITLE              "Open Surge Engine"
#define GAME_VERSION            0
#define GAME_SUB_VERSION        2
#define GAME_WIP_VERSION        0
#define GAME_WEBSITE            "opensnc.sourceforge.net"
#define GAME_YEAR               "2008-2013"

/* Cute stuff */
#ifndef GAME_UNIX_INSTALLDIR
#define GAME_UNIX_INSTALLDIR    "/usr/share/opensurge"
#endif

#ifndef GAME_UNIX_EXECDIR
#define GAME_UNIX_EXECDIR       "/usr/bin"
#endif

#ifndef GAME_BUILD_VERSION
#define GAME_VERSION_STRING     STRVALUE(GAME_VERSION) "." STRVALUE(GAME_SUB_VERSION) "." STRVALUE(GAME_WIP_VERSION)
#else
#define GAME_VERSION_STRING     STRVALUE(GAME_VERSION) "." STRVALUE(GAME_SUB_VERSION) "." STRVALUE(GAME_WIP_VERSION) " build " STRVALUE(GAME_BUILD_VERSION)
#endif

/* Global definitions and constants */
#ifdef INFINITY
#undef INFINITY
#endif

#ifdef INFINITY_FLT
#undef INFINITY_FLT
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifdef PI
#undef PI
#endif

#ifdef EPSILON
#undef EPSILON
#endif

#define TRUE                    -1
#define FALSE                   0
#define EPSILON                 1e-5
#define PI                      3.14159265
#define INFINITY                (1<<30)

#ifdef __MSVC__
static const float ZERO_FLT = 0.0;
#define INFINITY_FLT            (1.0/ZERO_FLT)
#else
#define INFINITY_FLT            (1.0/0.0)
#endif

/* Macro hacks ;) */
#define QUOTEME(x)              #x
#define STRVALUE(x)             QUOTEME(x)

/* Exact-width integer types */
#ifndef __MSVC__

#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#else

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

#endif

#endif
