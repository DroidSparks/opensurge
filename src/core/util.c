/*
 * Open Surge Engine
 * util.c - utilities
 * Copyright (C) 2008-2010  Alexandre Martins <alemartf(at)gmail(dot)com>
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "v2d.h"
#include "util.h"
#include "timer.h"
#include "logfile.h"



/* private stuff */
static volatile int game_over = FALSE;
static void merge_sort_recursive(void *base, size_t size, int (*comparator)(const void*,const void*), int p, int q);
static void merge_sort_mix(void *base, size_t size, int (*comparator)(const void*,const void*), int p, int q, int m);




/* Memory management */


/*
 * mallocx()
 * Similar to malloc(), but aborts the
 * program if it does not succeed.
 */
void *mallocx(size_t bytes)
{
    void *p = malloc(bytes);

    if(!p)
        fatal_error("FATAL ERROR: mallocx(%u) failed.\n", bytes);

    return p;
}


/*
 * relloacx()
 * Similar to realloc(), but abots the
 * program if it does not succeed.
 */
void* reallocx(void *ptr, size_t bytes)
{
    void *p = realloc(ptr, bytes);

    if(!p)
        fatal_error("FATAL ERROR: reallocx() failed.\n");

    return p;
}



/* Game routines */

/*
 * game_quit()
 * Quit game?
 */
void game_quit(void)
{
    game_over = TRUE;
}
END_OF_FUNCTION(game_quit)


/*
 * game_is_over()
 * Game over?
 */
int game_is_over()
{
    return game_over;
}


/*
 * game_version_compare()
 * Compares the given parameters to the version
 * of the game. Returns:
 * < 0 (game version is inferior),
 * = 0 (same version),
 * > 0 (game version is superior)
 */
int game_version_compare(int version, int sub_version, int wip_version)
{
    int game_version = (GAME_VERSION*10000 + GAME_SUB_VERSION*100 + GAME_WIP_VERSION);
    int other_version = (version*10000 + sub_version*100 + wip_version);

    return game_version - other_version;
}


/* Other */


/*
 * bounding_box()
 * bounding box collision method
 * r[4] = x1, y1, x2(=x1+w), y2(=y1+h)
 */
int bounding_box(float a[4], float b[4])
{
    return (a[0]<b[2] && a[2]>b[0] && a[1]<b[3] && a[3]>b[1]);
}



/*
 * circular_collision()
 * Circular collision method
 * a, b = points to test
 * r_a = radius of a
 * r_b = radius of b
 */
int circular_collision(v2d_t a, float r_a, v2d_t b, float r_b)
{
    return ( v2d_magnitude(v2d_subtract(a,b)) <= r_a + r_b );
}



/*
 * swap_ex()
 * Swaps two variables. Use the
 * swap() macro instead of this.
 */
void swap_ex(void *a, void *b, size_t size)
{
    uint8 *sa=a, *sb=b, c;
    size_t i;

    for(i=0; i<size; i++) {
        c = sa[i];
        sa[i] = sb[i];
        sb[i] = c;
    }
}





/*
 * fatal_error()
 * Displays a fatal error and aborts the application
 * (printf() format)
 */
void fatal_error(const char *fmt, ...)
{
    char buf[2048];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    logfile_message(buf);
    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
    allegro_message("%s", buf);
    exit(1);
}


/*
 * old_school_angle()
 * Old school angle
 */
float old_school_angle(float angle)
{
    if(angle >= 0 && angle < PI/4-EPSILON)
        return 0;
    else if(angle >= PI/4-EPSILON && angle < PI/2-EPSILON)
        return PI/4;
    else if(angle >= PI/2-EPSILON && angle < PI/2+EPSILON)
        return PI/2;
    else if(angle >= PI/2+EPSILON && angle < PI-EPSILON)
        return 3*PI/4;
    else if(angle >= PI-EPSILON && angle < PI+EPSILON)
        return PI;
    else if(angle >= PI+EPSILON && angle < 3*PI/2-EPSILON)
        return 5*PI/4;
    else if(angle >= 3*PI/2-EPSILON && angle < 3*PI/2+EPSILON)
        return 3*PI/2;
    else if(angle > 3*PI/2+EPSILON && angle <= 7*PI/4+EPSILON)
        return 7*PI/4;
    else
        return 0;
}

/*
 * merge_sort()
 * Similar to stdlib's qsort, but merge_sort is
 * a stable sorting algorithm
 *
 * base       - Pointer to the first element of the array to be sorted
 * num        - Number of elements in the array pointed by base
 * size       - Size in bytes of each element in the array
 * comparator - The return value of this function should represent
 *              whether elem1 is considered less than, equal to,
 *              or greater than elem2 by returning, respectively,
 *              a negative value, zero or a positive value
 *              int comparator(const void *elem1, const void *elem2)
 */
void merge_sort(void *base, size_t num, size_t size, int (*comparator)(const void*,const void*))
{
    merge_sort_recursive(base, size, comparator, 0, num-1);
}



/* private stuff */
void merge_sort_recursive(void *base, size_t size, int (*comparator)(const void*,const void*), int p, int q)
{
    if(q > p) {
        int m = (p+q)/2;
        merge_sort_recursive(base, size, comparator, p, m);
        merge_sort_recursive(base, size, comparator, m+1, q);
        merge_sort_mix(base, size, comparator, p, q, m);
    }
}

void merge_sort_mix(void *base, size_t size, int (*comparator)(const void*,const void*), int p, int q, int m)
{
    uint8 *arr = mallocx((q-p+1) * size);
    uint8 *i = arr;
    uint8 *j = arr + (m+1-p) * size;
    int k = p;

    memcpy(arr, (uint8*)base + p * size, (q-p+1) * size);

    while(i < arr + (m+1-p) * size && j <= arr + (q-p) * size) {
        if(comparator((const void*)i, (const void*)j) <= 0) {
            memcpy((uint8*)base + (k++) * size, i, size);
            i += size;
        }
        else {
            memcpy((uint8*)base + (k++) * size, j, size);
            j += size;
        }
    }

    while(i < arr + (m+1-p) * size) {
        memcpy((uint8*)base + (k++) * size, i, size);
        i += size;
    }

    while(j <= arr + (q-p) * size) {
        memcpy((uint8*)base + (k++) * size, j, size);
        j += size;
    }

    free(arr);
}

