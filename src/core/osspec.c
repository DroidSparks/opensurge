/*
 * Open Surge Engine
 * osspec.c - OS Specific Routines
 * Copyright (C) 2009-2010, 2012  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include <stdio.h>
#include <ctype.h>
#include <allegro.h>
#include "global.h"
#include "osspec.h"
#include "util.h"
#include "stringutil.h"



/* uncomment to disable case insensitive filename support
 * on platforms that do not support it (like *nix)
 * (see fix_case_path() for more information).
 *
 * Disabling this feature can speed up a little bit
 * the file searching routines on platforms like *nix
 * (it has no effect under Windows, though).
 *
 * Keeping this enabled provides case insensitive filename
 * support on platforms like *nix. */
/*#define DISABLE_FIX_CASE_PATH*/



/* uncomment to disable filepath optimizations - useful
 * on distributed file systems (example: nfs).
 *
 * Disabling this feature improves memory usage just
 * a little bit. You probably want to keep this
 * option enabled.
 *
 * Keeping this enabled improves drastically the
 * speed of the game when the files are stored
 * in a distributed file system like NFS. */
/*#define DISABLE_FILEPATH_OPTIMIZATIONS*/




#ifndef __WIN32__

#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#else

#include <winalleg.h>

#endif




/* private stuff */
#ifndef __WIN32__
static struct passwd *userinfo;
#endif

static char executable_name[1024];
static char* fix_case_path(char *filepath);
static int fix_case_path_backtrack(const char *pwd, const char *remaining_path, const char *delim, char *dest);
static void search_the_file(char *dest, const char *relativefp, size_t dest_size);

typedef struct {
    int (*callback)(const char *filename, void *param);
    void *param;
} foreach_file_helper;

static int foreach_file_callback(const char *filename, int attrib, void *param);

#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
/* cache stuff (also private): it's a basic dictionary */
static void cache_init(); /* hi! :-) */
static void cache_release(); /* bye! */
static char *cache_search(const char *key); /* searches for key */
static void cache_insert(const char *key, const char *value); /* new key */
static void cache_update(const char *key, const char *value); /* will update key, if it exists */

/* the cache is implemented as a simple binary tree */
typedef struct cache_t {
    char *key, *value;
    struct cache_t *left, *right;
} cache_t;
static cache_t *cache_root;
static cache_t *cachetree_release(cache_t *node);
static cache_t *cachetree_search(cache_t *node, const char *key);
static cache_t *cachetree_insert(cache_t *node, const char *key, const char *value);
#endif

/* url encoding routines */
/*static char hex2ch(char ch);*/
static char ch2hex(char code);
static char *url_encode(const char *str);
/*static char *url_decode(const char *str);*/


/* public functions */

/* 
 * osspec_init()
 * Operating System Specifics - initialization
 */
void osspec_init()
{
#ifndef __WIN32__
    int i;
    char tmp[1024];
    char subdirs[][32] = {   /* subfolders at $HOME/.$GAME_UNIXNAME/ */
        { "" },
        { "characters" },
        { "config" },
        { "fonts" },
        { "images" },
        { "languages" },
        { "levels" },
        { "musics" },
        { "objects" },
        { "quests" },
        { "samples" },
        { "screenshots" },
        { "sprites" },
        { "themes" },
        { "ttf" }
    };



    /* retrieving user data */
    if(NULL == (userinfo = getpwuid(getuid())))
        fprintf(stderr, "WARNING: couldn't obtain information about your user. User-specific data may not work.\n");



    /* creating sub-directories */
    for(i=0; i<sizeof(subdirs)/32; i++) {
        home_filepath(tmp, subdirs[i], sizeof(tmp));
        mkdir(tmp, 0755);
    }
#endif

    /* executable name */
    get_executable_name(executable_name, sizeof(executable_name));

#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
    /* initializing the cache */
    cache_init();
#endif
}


/* 
 * osspec_release()
 * Operating System Specifics - release
 */
void osspec_release()
{
#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
    /* releasing the cache */
    cache_release();
#endif
}


/*
 * filepath_exists()
 * Returns TRUE if the given file exists
 * or FALSE otherwise
 */
int filepath_exists(const char *filepath)
{
    return exists(filepath);
}



/*
 * directory_exists()
 * Returns TRUE if the given directory exists
 * or FALSE otherwise
 */
int directory_exists(const char *dirpath)
{
    return file_exists(dirpath, FA_DIREC | FA_HIDDEN | FA_RDONLY, NULL);
}




/*
 * absolute_filepath()
 * Converts a relative filepath into an
 * absolute filepath.
 */
void absolute_filepath(char *dest, const char *relativefp, size_t dest_size)
{
    if(is_relative_filename(relativefp)) {
#ifndef __WIN32__
        char tmp[1024];
        str_cpy(tmp, executable_name, sizeof(tmp));
        tmp[ strlen(GAME_UNIX_COPYDIR) ] = '\0';
        if(strcmp(tmp, GAME_UNIX_COPYDIR) == 0)
            sprintf(dest, "%s/%s", GAME_UNIX_INSTALLDIR, relativefp);
        else {
            str_cpy(dest, executable_name, dest_size);
            replace_filename(dest, dest, relativefp, dest_size);
        }
#else
        str_cpy(dest, executable_name, dest_size);
        replace_filename(dest, dest, relativefp, dest_size);
#endif
    }
    else
        str_cpy(dest, relativefp, dest_size); /* relativefp is already an absolute filepath */

    fix_filename_slashes(dest);
    canonicalize_filename(dest, dest, dest_size);
    fix_case_path(dest);
}



/*
 * home_filepath()
 * Similar to absolute_filepath(), but this routine considers
 * the $HOME/.$GAME_UNIXNAME/ directory instead
 */
void home_filepath(char *dest, const char *relativefp, size_t dest_size)
{
#ifndef __WIN32__

    if(userinfo) {
        sprintf(dest, "%s/.%s/%s", userinfo->pw_dir, GAME_UNIXNAME, relativefp);
        fix_filename_slashes(dest);
        canonicalize_filename(dest, dest, dest_size);
        fix_case_path(dest);
    }
    else
        absolute_filepath(dest, relativefp, dest_size);

#else

    absolute_filepath(dest, relativefp, dest_size);

#endif
}




/*
 * resource_filepath()
 * Similar to absolute_filepath() and home_filepath(), but this routine
 * searches the specified file both in the home directory and in the
 * game directory
 */
void resource_filepath(char *dest, const char *relativefp, size_t dest_size, int resfp_mode)
{
    switch(resfp_mode) {
        /* I'll read the file */
        case RESFP_READ:
        {

#ifndef DISABLE_FILEPATH_OPTIMIZATIONS

            /* optimizations: without this, the game could become terribly slow
             * when the files are distributed over a network (example: nfs) */
            char *path;
            if(is_relative_filename(relativefp)) {
                if(NULL == (path=cache_search(relativefp))) {
                    /* I'll have to search the file... */
                    search_the_file(dest, relativefp, dest_size);

                    /* store the resulting filepath in the memory */
                    cache_insert(relativefp, dest);
                }
                else
                    str_cpy(dest, path, dest_size);
            }
            else
                search_the_file(dest, relativefp, dest_size);

#else

            search_the_file(dest, relativefp, dest_size);

#endif
            break;

        }

        /* I'll write to the file */
        case RESFP_WRITE:
        {
            FILE *fp;
            struct al_ffblk info;
            absolute_filepath(dest, relativefp, dest_size);

            if(al_findfirst(dest, &info, FA_HIDDEN) == 0) {
                /* the file exists AND it's NOT read-only */
                al_findclose(&info);
            }
            else {

                /* the file does not exist OR it is read-only */
                if(!filepath_exists(dest)) {

                    /* it doesn't exist */
                    if(NULL != (fp = fopen(dest, "w"))) {
                        /* is it writable? this shouldn't happen */
                        fclose(fp);
                        delete_file(dest);
                    }
                    else {
                        /* it's not writable */
                        home_filepath(dest, relativefp, dest_size);
                    }

                }
                else {
                    /* the file exists, but it's read-only */
                    home_filepath(dest, relativefp, dest_size);
                }

            }

#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
            /* this is sooo important... */
            if(cache_search(relativefp) != NULL)
                cache_update(relativefp, dest);
            else
                cache_insert(relativefp, dest);
#endif
            break;
        }

        /* Unknown mode */
        default:
        {
            fprintf(stderr, "resource_filepath(): invalid resfp_mode (%d)", resfp_mode);
            break;
        }
    }
}



/*
 * create_process()
 * Creates a new process;
 *     path is the absolute path to the executable file
 *     argc is argument count
 *     argv[] contains the command line arguments
 *
 * NOTE: argv[0] also contains the absolute path of the executable
 */
void create_process(const char *path, int argc, char *argv[])
{
#ifndef __WIN32__
    pid_t pid;

    argv[argc] = NULL;
    if(0 == (pid=fork())) /* if(child process) */
        execv(path, argv);
#else
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char cmd[10240]="";
    int i, is_file;

    for(i=0; i<argc; i++) {
        is_file = filepath_exists(argv[i]); /* TODO: test folders with spaces (must test program.exe AND level.lev) */
        if(is_file) strcat(cmd, "\"");
        strcat(cmd, argv[i]);
        strcat(cmd, is_file ? "\" " : " ");
    }
    cmd[strlen(cmd)-1] = '\0'; /* erase the last blank space */

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if(!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        MessageBox(NULL, "Couldn't CreateProcess()", "Error", MB_OK);
        MessageBox(NULL, cmd, "Command Line", MB_OK);
        exit(1);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif
}


/*
 * basename()
 * Finds out the filename portion of a completely specified file path
 */
char *basename(const char *path)
{
    return get_filename(path);
}


/*
 * foreach_file()
 * Traverses a directory, calling callback on each file
 * wildcard may be a relative path, e.g.: "images / *.png"
 *
 * Note: this function doesn't recurse
 * Note 2: callback must return 0 to let the enumeration proceed, or non-zero to stop it
 *
 * Returns the number of successfull calls to callback.
 */
int foreach_file(const char *wildcard, int (*callback)(const char *filename, void *param), void *param)
{
    int deny_flags = FA_DIREC | FA_LABEL;
    char abs_path[1024];
    foreach_file_helper h = { callback, param };

    absolute_filepath(abs_path, wildcard, sizeof(abs_path));
    return for_each_file_ex(abs_path, 0, deny_flags, foreach_file_callback, (void*)(&h));
}



/*
 * launch_url()
 * Launches an URL using the default browser.
 * Returns TRUE on success.
 * Useful stuff: http://www.dwheeler.com/essays/open-files-urls.html
 */
int launch_url(const char *url)
{
    int ret = TRUE;
    char *safe_url = url_encode(url); /* it's VERY important to sanitize the URL... */

    if(strncmp(safe_url, "http://", 7) == 0 || strncmp(safe_url, "https://", 8) == 0 || strncmp(safe_url, "ftp://", 6) == 0 || strncmp(safe_url, "mailto:", 7) == 0) {
#ifdef __WIN32__
        ShellExecute(NULL, "open", safe_url, NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
        char *safe_cmd = mallocx(sizeof(char) * (strlen(safe_url) + 32));
        *safe_cmd = 0;

        if(filepath_exists("/usr/bin/open"))
            sprintf(safe_cmd, "open \"%s\"", safe_url);
        else 
            ret = FALSE; /* failure */

        if(*safe_cmd)
            ret = system(safe_cmd) >= 0;

        free(safe_cmd);
#else
        char *safe_cmd = mallocx(sizeof(char) * (strlen(safe_url) + 32));
        *safe_cmd = 0;

        if(filepath_exists("/usr/bin/xdg-open"))
            sprintf(safe_cmd, "xdg-open \"%s\"", safe_url);
        else if(filepath_exists("/usr/bin/firefox"))
            sprintf(safe_cmd, "firefox \"%s\"", safe_url);
        else 
            ret = FALSE; /* failure */

        if(*safe_cmd)
            ret = system(safe_cmd) >= 0;

        free(safe_cmd);
#endif
    }
    else {
        ret = FALSE;
        fatal_error("Can't launch url: invalid protocol (valid ones are: http, https, ftp, mailto).\n%s", safe_url);
    }

    free(safe_url);
    return ret;
}






/* private methods */

/* helps foreach_file() */
int foreach_file_callback(const char *filename, int attrib, void *param)
{
    foreach_file_helper *h = (foreach_file_helper*)param;
    return h->callback(filename, h->param);
}

/* backtracking routine used in fix_case_path()
 * returns TRUE iff a solution is found */
int fix_case_path_backtrack(const char *pwd, const char *remaining_path, const char *delim, char *dest)
{
    char *query, *pos, *p, *tmp, *new_pwd;
    const char *q;
    struct al_ffblk info;
    int ret = FALSE;

    if(NULL != (pos = strchr(remaining_path, *delim))) {
        /* if remaning_path is "my/example/query", then
           query is equal to "my" */
        query = mallocx(sizeof(char) * (pos-remaining_path+1));
        for(p=query,q=remaining_path; q!=pos;)
            *p++ = *q++;
        *p = 0;

        /* next folder... */
        tmp = mallocx(sizeof(char) * (strlen(pwd)+2));
        sprintf(tmp, "%s*", pwd);
        if(al_findfirst(tmp, &info, FA_ALL) == 0) {
            do {
                if(str_icmp(query, info.name) == 0) {
                    new_pwd = mallocx(sizeof(char) * (strlen(pwd)+strlen(query)+strlen(delim)+1));
                    sprintf(new_pwd, "%s%s%s", pwd, info.name, delim);
                    ret = fix_case_path_backtrack(new_pwd, pos+1, delim, dest);
                    free(new_pwd);
                    if(ret) break;
                }
            }
            while(al_findnext(&info) == 0);
            al_findclose(&info);
        }
        free(tmp);

        /* releasing resources */
        free(query);
    }
    else {
        /* no more subdirectories */
        tmp = mallocx(sizeof(char) * (strlen(pwd)+2));
        sprintf(tmp, "%s*", pwd);
        if(al_findfirst(tmp, &info, FA_ALL) == 0) {
            do {
                if(str_icmp(remaining_path, info.name) == 0) {
                    ret = TRUE;
                    sprintf(dest, "%s%s", pwd, info.name);
                    break;
                }
            }
            while(al_findnext(&info) == 0);
            al_findclose(&info);
        }
        free(tmp);
    }

    /* done */
    return ret;
}


/* Case-insensitive filename support for all platforms.
 *
 * If the user requests for the file "LEVELS/MyLevel.lev", but
 * only "levels/mylevel.lev" exists, the valid filepath will
 * be used. This routine does nothing on Windows.
 *
 * filepath may be modified during the process. A copy of it
 * is returned. */
char* fix_case_path(char *filepath)
{
#if !defined(DISABLE_FIX_CASE_PATH) && !defined(__WIN32__)
    char *tmp;
    const char delimiter[] = "/";
    int solved = FALSE;

    if(!filepath_exists(filepath)) {
        fix_filename_slashes(filepath);
        tmp = mallocx(sizeof(char) * (strlen(filepath)+1));

        if(*filepath == *delimiter)
            solved = fix_case_path_backtrack(delimiter, filepath+1, delimiter, tmp);
        else
            solved = fix_case_path_backtrack("", filepath, delimiter, tmp);

        if(solved)
            strcpy(filepath, tmp);

        free(tmp);
    }
#endif

    return filepath;
}


/* auxiliary routine for resource_filepath(): given any filepath (relative or absolute),
 * finds the absolute path (either in the home directory or in the game directory) */
void search_the_file(char *dest, const char *relativefp, size_t dest_size)
{
    home_filepath(dest, relativefp, dest_size);
    if(!filepath_exists(dest) && !directory_exists(dest))
        absolute_filepath(dest, relativefp, dest_size);
}








/* url encoding-decoding routines */

/* converts ch from hex */
/*char hex2ch(char ch) {
    return isdigit(ch) ? (ch - '0') : ((char)toupper(ch) - 'A' + 10);
}*/

/* converts code to hex */
char ch2hex(char code) {
    static char hex[] = "0123456789ABCDEF";
    return hex[code & 0xF];
}

/* returns an url-encoded version of str */
char *url_encode(const char *str) {
    const char *p;
    char *buf = mallocx(sizeof(char) * (strlen(str) * 3 + 1)), *q = buf;

    for(p = str; *p; p++) {
        if(isalnum(*p) || *p == '-' || *p == '#' || *p == '_' || *p == '.' || *p == '~' || *p == ':' || *p == '?' || *p == '&' || *p == '/' || *p == '=' || *p == '+' || *p == '@')
            *q++ = *p; /* safety: we ensure that *p != '\\', *p != '\"' */
        else if(*p == ' ') 
            *q++ = '+';
        else 
            *q++ = '%', *q++ = ch2hex(*p >> 4), *q++ = ch2hex(*p & 0xF);
    }

    *q = 0;
    return buf;
}

/* returns an url-decoded version of str */
/*char *url_decode(const char *str) {
    const char *p;
    char *buf = mallocx(sizeof(char) * (strlen(str) + 1)), *q = buf;

    for(p = str; *p; p++) {
        if(*p == '%') {
            if(*(p+1) && *(p+2)) {
                *q++ = (hex2ch(*(p+1)) << 4) | hex2ch(*(p+2));
                p += 2;
            }
        }
        else if(*p == '+')
            *q++ = ' ';
        else
            *q++ = *p;
    }

    *q = 0;
    return buf;
}*/



#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
/* ------- cache interface -------- */

/* initializes the cache */
void cache_init()
{
    cache_root = NULL;
}

/* releases the cache */
void cache_release()
{
    cache_root = cachetree_release(cache_root);
}

/* finds a string in the dictionary */
char *cache_search(const char *key)
{
    cache_t *node = cachetree_search(cache_root, key);
    return node ? node->value : NULL;
}

/* inserts a string into the dictionary */
void cache_insert(const char *key, const char *value)
{
    cache_root = cachetree_insert(cache_root, key, value);
}

/* will update an entry of the dictionary, if that entry exists */
void cache_update(const char *key, const char *value)
{
    cache_t *node = cachetree_search(cache_root, key);
    if(node != NULL) {
        free(node->value);
        node->value = str_dup(value);
    }
}

/* ------ cache implementation --------- */
cache_t *cachetree_release(cache_t *node)
{
    if(node) {
        node->left = cachetree_release(node->left);
        node->right = cachetree_release(node->right);
        free(node->key);
        free(node->value);
        free(node);
    }

    return NULL;
}

cache_t *cachetree_search(cache_t *node, const char *key)
{
    int cmp;

    if(node) {
        cmp = strcmp(key, node->key);

        if(cmp < 0)
            return cachetree_search(node->left, key);
        else if(cmp > 0)
            return cachetree_search(node->right, key);
        else
            return node;
    }
    else
        return NULL;
}

cache_t *cachetree_insert(cache_t *node, const char *key, const char *value)
{
    int cmp;
    cache_t *t;

    if(node) {
        cmp = strcmp(key, node->key);

        if(cmp < 0)
            node->left = cachetree_insert(node->left, key, value);
        else if(cmp > 0)
            node->right = cachetree_insert(node->right, key, value);

        return node;
    }
    else {
        t = mallocx(sizeof *t);
        t->key = str_dup(key);
        t->value = str_dup(value);
        t->left = t->right = NULL;
        return t;
    }
}
#endif

