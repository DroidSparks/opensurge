/*
 * 2xSaI engine. This file was obtained from:
 * http://bob.allegronetwork.com/projects.html
 * Authors: Derek Liauw Kie Fa/Kreed, Robert J Ohannessian, Peter Wang
 */

/*
2xSaI
~~~~~

Copyright (c) Derek Liauw Kie Fa, 1999  
Modifications for Allegro 3.9+ comptibility by Robert J Ohannessian.

Comments, Suggestions, Questions, Bugs etc.:
    derek-liauw@usa.net  
    void_star_@excite.com

Original web site: http://members.xoom.com/derek_liauw/                              

This program is free for non-commercial use.                      


Disclaimer
----------
#include <std_disclaimer.h>
Use this program at your own risk. This program was not intended to do
anything malicious, but we are not responsible for any damage or loss
that may arise by the use of this program.
*/

#ifndef _2XSAI_H
#define _2XSAI_H

#include <allegro.h>

#if !defined(_MSC_VER)
#include <stdint.h>
typedef uint8_t __UInt8;
typedef uint16_t __UInt16;
typedef uint32_t __UInt32;
#else
typedef unsigned __int8 __UInt8;
typedef unsigned __int16 __UInt16;
typedef unsigned __int32 __UInt32;
#endif

int Init_2xSaI(int depth);
void Super2xSaI(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h);
void Super2xSaI_ex(__UInt8 *src, __UInt32 src_pitch, __UInt8 *unused, BITMAP *dest, __UInt32 width, __UInt32 height);

void SuperEagle(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h);
void SuperEagle_ex(__UInt8 *src, __UInt32 src_pitch, __UInt8 *unused, BITMAP *dest, __UInt32 width, __UInt32 height);

#endif
