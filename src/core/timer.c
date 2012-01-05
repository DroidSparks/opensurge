/*
 * Open Surge Engine
 * timer.c - time handler
 * Copyright (C) 2010, 2012  Alexandre Martins <alemartf(at)gmail(dot)com>
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



/*
 * Uncomment the line below to use Allegro routines to
 * handle the timers (so you don't have to rely on
 * platform-specific code).
 *
 * Please keep in mind that Allegro timers may be
 * slower, though.
 */

/*#define USE_ALLEGRO_TIMERS*/



#include <allegro.h>
#include "global.h"
#include "timer.h"
#include "util.h"
#include "logfile.h"

#if !defined(USE_ALLEGRO_TIMERS) && !defined(__WIN32__)
#include <sys/time.h>
#elif !defined(USE_ALLEGRO_TIMERS) && defined(__WIN32__)
#include <winalleg.h>
#endif




/* constants */
#define MIN_FRAME_INTERVAL 15 /* (1/15) * 1000 ~ 67 fps max */
#define MAX_FRAME_INTERVAL 17 /* (1/17) * 1000 ~ 58 fps min */


/* internal data */
static int partial_fps, fps_accum, fps;
static uint32 last_time;
static float delta;
static int yield_cpu;

#ifdef USE_ALLEGRO_TIMERS
static volatile uint32 elapsed_time;
static void update_timer();
#else
static uint32 start_time;
static uint32 get_tick_count(); /* platform-specific code */
#endif


/*
 * timer_init()
 * Initializes the Time Handler
 */
void timer_init(int optimize_cpu_usage)
{
    logfile_message("timer_init()");

    /* installing Allegro stuff */
    logfile_message("Installing Allegro timers...");
    if(install_timer() != 0)
        logfile_message("install_timer() failed: %s", allegro_error);

    /* should we optimize the cpu usage? */
    yield_cpu = optimize_cpu_usage;

    /* initializing... */
    partial_fps = 0;
    fps_accum = 0;
    fps = 0;
    delta = 0.0;

#ifdef USE_ALLEGRO_TIMERS
    /* tracking the time manually */
    elapsed_time = 0;
    LOCK_VARIABLE(elapsed_time);
    LOCK_FUNCTION(update_timer);
    install_int(update_timer, 10);
#else
    start_time = get_tick_count();
#endif

    /* done! */
    last_time = timer_get_ticks();
}


/*
 * timer_update()
 * Updates the Time Handler. This routine
 * must be called at every cycle of
 * the main loop
 */
void timer_update()
{
    uint32 current_time, delta_time; /* both in milliseconds */

    /* time control */
    for(delta_time = 0 ;;) {
        current_time = timer_get_ticks();
        delta_time = (current_time > last_time) ? (current_time - last_time) : 0;
        last_time = (current_time >= last_time) ? last_time : current_time;

        if(delta_time < MIN_FRAME_INTERVAL) {
            if(yield_cpu) {
                /* we don't like having the cpu usage at 100%. will the OS make our process active again on time? */
#ifndef __WIN32__
                /*rest(0);*/ /* if we use rest(0), probably not... */
                rest(1);
#else
                Sleep(1);
#endif
            }
        }
        else
            break;
    }
    delta_time = min(delta_time, MAX_FRAME_INTERVAL);
    delta = (float)delta_time * 0.001;

    /* FPS (frames per second) */
    partial_fps++; /* 1 render per cycle */
    fps_accum += (int)delta_time;
    if(fps_accum >= 1000) {
        fps = partial_fps;
        partial_fps = 0;
        fps_accum = 0;
    }

    /* done! */
    last_time = timer_get_ticks();
}


/*
 * timer_release()
 * Releases the Time Handler
 */
void timer_release()
{
    logfile_message("timer_release()");
}


/*
 * timer_get_delta()
 * Returns the time interval, in seconds,
 * between the last two cycles of the
 * main loop
 */
float timer_get_delta()
{
    return delta;
}


/*
 * timer_get_ticks()
 * Elapsed milliseconds since
 * the application has started
 */
uint32 timer_get_ticks()
{
#ifdef USE_ALLEGRO_TIMERS
    return elapsed_time * 10;
#else
    uint32 ticks = get_tick_count();
    if(ticks < start_time)
        start_time = ticks;
    return ticks - start_time;
#endif
}


/*
 * timer_get_fps()
 * Returns the FPS rate
 */
int timer_get_fps()
{
    return fps;
}



/* internal methods */

#ifdef USE_ALLEGRO_TIMERS

void update_timer()
{
    elapsed_time++;
}
END_OF_FUNCTION(update_timer)

#elif defined(__WIN32__)

uint32 get_tick_count()
{
    return GetTickCount();
}

#else

uint32 get_tick_count()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec*1000) + (now.tv_usec/1000);
}

#endif
