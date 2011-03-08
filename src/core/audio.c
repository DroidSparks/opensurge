/*
 * Open Surge Engine
 * audio.c - audio module
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
#include <logg.h>
#include <stdlib.h>
#include "audio.h"
#include "osspec.h"
#include "stringutil.h"
#include "resourcemanager.h"
#include "logfile.h"
#include "timer.h"
#include "util.h"

/* private definitions */
#define IS_OGG(path)                (str_icmp((path)+strlen(path)-4, ".ogg") == 0)
#define MUSIC_DURATION(m)           ((float)(m->stream->len) / (float)(m->stream->freq))
#define SOUND_INVALID_VOICE         -1
#define PREFERRED_NUMBER_OF_VOICES  32

/* private structures */
struct music_t {
    LOGG_Stream *stream;
    int loops_left;
    int is_paused;
    float elapsed_time;
};

struct sound_t {
    SAMPLE *data;
    int voice_id;
};

/* private stuff*/
static music_t *current_music; /* music being played at the moment (NULL if none) */
static void setup_voices();



/*
 * music_load()
 * Loads a music from a file
 */
music_t *music_load(const char *path)
{
    char abs_path[1024];
    music_t *m;

    if(NULL == (m = resourcemanager_find_music(path))) {
        resource_filepath(abs_path, path, sizeof(abs_path), RESFP_READ);
        logfile_message("music_load('%s')", abs_path);

        /* build the music object */
        m = mallocx(sizeof *m);
        m->loops_left = 0;
        m->is_paused = FALSE;
        m->elapsed_time = 0.0f;

        /* load the ogg stream */
        m->stream = logg_get_stream(abs_path, 255, 128, 0);
        if(m->stream == NULL) {
            logfile_message("music_load() error: can't get ogg stream");
            free(m);
            return NULL;
        }

        /* adding it to the resource manager */
        resourcemanager_add_music(path, m);
        resourcemanager_ref_music(path);

        /* done! */
        logfile_message("music_load() ok");
    }
    else
        resourcemanager_ref_music(path);

    return m;
}



/*
 * music_unref()
 * Will try to release the resource from
 * the memory. You will call this if, and
 * only if, you are sure you don't need the
 * resource anymore (i.e., you're not holding
 * any pointers to it)
 *
 * Used for reference counting. Normally you
 * don't need to bother with this, unless
 * you care about reducing memory usage.
 * Note that 'music_ref()' must not exist.
 * Returns the no. of references to the music
 */
int music_unref(const char *path)
{
    return resourcemanager_unref_music(path);
}



/*
 * music_destroy()
 * Destroys a music. This is called automatically
 * while unloading the resource manager.
 */
void music_destroy(music_t *music)
{
    if(music != NULL) {
        logg_destroy_stream(music->stream);
        free(music);
    }
}


/*
 * music_play()
 * Plays the given music and loops [loop] times.
 * Set loop equal to INFINITY to make it loop forever.
 */
void music_play(music_t *music, int loop)
{
    music_stop();

    if(music != NULL) {
        music->loops_left = loop;
        music->is_paused = FALSE;
        music->elapsed_time = 0.0f;
        music->stream->loop = (loop >= INFINITY); /* "gambiarra", because LOGG lacks features */
    }

    current_music = music;
}


/*
 * music_stop()
 * Stops the current music (if any)
 */
void music_stop()
{
    if(current_music != NULL) {
        char *filename = str_dup(current_music->stream->filename);
        logg_destroy_stream(current_music->stream);
        current_music->stream = logg_get_stream(filename, 255, 128, 0);
        free(filename);
    }

    current_music = NULL;
}


/*
 * music_pause()
 * Pauses the current music
 */
void music_pause()
{
    if(current_music != NULL && !(current_music->is_paused)) {
        current_music->is_paused = TRUE;
        voice_stop(current_music->stream->audio_stream->voice);
    }
}



/*
 * music_resume()
 * Resumes the current music
 */
void music_resume()
{
    if(current_music != NULL && current_music->is_paused) {
        current_music->is_paused = FALSE;
        voice_start(current_music->stream->audio_stream->voice);
    }
}


/*
 * music_set_volume()
 * Changes the volume of the current music.
 * 0.0f (quiet) <= volume <= 1.0f (loud)
 * default = 1.0f
 */
void music_set_volume(float volume)
{
    if(current_music != NULL) {
        volume = clip(volume, 0.0f, 1.0f);
        current_music->stream->volume = (int)(255.0f * volume);
        voice_set_volume(current_music->stream->audio_stream->voice, current_music->stream->volume);
    }
}


/*
 * music_get_volume()
 * Returns the volume of the current music.
 * 0.0f <= volume <= 1.0f
 */
float music_get_volume()
{
    if(current_music != NULL)
        return (float)current_music->stream->volume / 255.0f;
    else
        return 0.0f;
}



/*
 * music_is_playing()
 * Returns TRUE if a music is playing, FALSE
 * otherwise.
 */
int music_is_playing()
{
    return (current_music != NULL) && !(current_music->is_paused) && (current_music->loops_left >= 0);
}





/* sound management */


/*
 * sound_load()
 * Loads a sample from a file
 */
sound_t *sound_load(const char *path)
{
    char abs_path[1024];
    sound_t *s;

    if(NULL == (s = resourcemanager_find_sample(path))) {
        resource_filepath(abs_path, path, sizeof(abs_path), RESFP_READ);
        logfile_message("sound_load('%s')", abs_path);

        /* build the sound object */
        s = mallocx(sizeof *s);
        s->voice_id = SOUND_INVALID_VOICE;

        /* loading the sample */
        if(NULL == (s->data = IS_OGG(path) ? logg_load(abs_path) : load_sample(abs_path))) {
            logfile_message("sound_load() error: %s", allegro_error);
            free(s);
            return NULL;
        }

        /* adding it to the resource manager */
        resourcemanager_add_sample(path, s);
        resourcemanager_ref_sample(path);

        /* done! */
        logfile_message("sound_load() ok");
    }
    else
        resourcemanager_ref_sample(path);

    return s;
}

/*
 * sound_unref()
 * Will try to release the resource from
 * the memory. You will call this if, and
 * only if, you are sure you don't need the
 * resource anymore (i.e., you're not holding
 * any pointers to it)
 *
 * Used for reference counting. Normally you
 * don't need to bother with this, unless
 * you care about reducing memory usage.
 * Note that 'sound_ref()' must not exist.
 * Returns the no. of references to the sample
 */
int sound_unref(const char *path)
{
    return resourcemanager_unref_sample(path);
}


/*
 * sound_destroy()
 * Releases the given sample. This is called
 * automatically while releasing the main hash
 */
void sound_destroy(sound_t *sample)
{
    if(sample != NULL) {
        destroy_sample(sample->data);
        free(sample);
    }
}


/*
 * sound_play()
 * Plays the given sample
 */
void sound_play(sound_t *sample)
{
    sound_play_ex(sample, 1.0, 0.0, 1.0, 0);
}


/*
 * sound_play_ex()
 * Plays the given sample with extra options! :)
 *
 * 0.0 <= volume <= 1.0
 * (left speaker) -1.0 <= pan <= 1.0 (right speaker)
 * 1.0 = default frequency
 * 0 = no loops
 */
void sound_play_ex(sound_t *sample, float vol, float pan, float freq, int loop)
{
    int id;

    if(sample) {
        vol = clip(vol, 0.0, 1.0);
        pan = clip(pan, -1.0, 1.0);
        freq = max(freq, 0.0);
        id = play_sample(sample->data, (int)(255.0*vol), min(255,128+(int)(128.0*pan)), (int)(1000.0*freq), loop);
        sample->voice_id = id < 0 ? SOUND_INVALID_VOICE : id;
    }
}



/*
 * sound_stop()
 * Stops a sample
 */
void sound_stop(sound_t *sample)
{
    if(sample)
        stop_sample(sample->data);
}


/*
 * sound_is_playing()
 * Checks if a given sound is playing or not
 */
int sound_is_playing(sound_t *sample)
{
    if(sample && sample->voice_id != SOUND_INVALID_VOICE)
        return (voice_check(sample->voice_id) == sample->data);
    else
        return FALSE;
}








/* audio manager */

/*
 * audio_init()
 * Initializes the Audio Manager
 */
void audio_init(int nomusic)
{
    logfile_message("audio_init()");
    current_music = NULL;
    setup_voices();
    logfile_message("audio_init() ok");
}


/*
 * audio_release()
 * Releases the audio manager
 */
void audio_release()
{
    logfile_message("audio_release()");
    logfile_message("audio_release() ok");
}


/*
 * audio_update()
 * Updates the audio manager
 */
void audio_update()
{
    /* updating the music */
    if(current_music != NULL && !(current_music->is_paused)) {
        logg_update_stream(current_music->stream);

        /* "gambiarra" (ugly hack), because LOGG lacks features */
        if(!(current_music->stream->loop)) {
            current_music->elapsed_time += timer_get_delta();
            if(current_music->elapsed_time >= MUSIC_DURATION(current_music) + 0.25f) {
                if(--(current_music->loops_left) >= 0)
                    music_play(current_music, current_music->loops_left);
                else
                    music_stop();
            }
        }
    }
}


/*
 * setup_voices()
 * Allocates a few voices
 */
void setup_voices()
{
    int voices = PREFERRED_NUMBER_OF_VOICES;
    logfile_message("Reserving voices...");

    while(voices > 4) {
        reserve_voices(voices, 0);
        if(install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) == 0) {
            logfile_message("Reserved %d voices.", voices);
            return;
        }
        else
            voices /= 2;
    }

    logfile_message("Warning: unable to reserve voices.\n%s\n", allegro_error);
}
