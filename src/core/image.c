/*
 * Open Surge Engine
 * image.c - image implementation
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

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <png.h>
#include <allegro.h>
#include <loadpng.h>
#include <jpgalleg.h>
#include "image.h"
#include "video.h"
#include "stringutil.h"
#include "logfile.h"
#include "osspec.h"
#include "resourcemanager.h"
#include "util.h"

/* useful macros */
#define IS_PNG(path) (str_icmp((path)+strlen(path)-4, ".png") == 0)

/* private stuff */
static void maskcolor_bugfix(image_t *img);

/*
 * image_load()
 * Loads a image from a file.
 * Supported types: PNG, JPG, BMP, PCX, TGA
 */
image_t *image_load(const char *path)
{
    char abs_path[1024];
    image_t *img;

    if(NULL == (img = resourcemanager_find_image(path))) {
        resource_filepath(abs_path, path, sizeof(abs_path), RESFP_READ);
        logfile_message("image_load('%s')", abs_path);

        /* build the image object */
        img = mallocx(sizeof *img);

        /* loading the image */
        img->data = load_bitmap(abs_path, NULL);
        if(img->data == NULL) {
            logfile_message("image_load() error: %s", allegro_error);
            free(img);
            return NULL;
        }

        /* configuring the image */
        img->w = img->data->w;
        img->h = img->data->h;
        maskcolor_bugfix(img);

        /* adding it to the resource manager */
        resourcemanager_add_image(path, img);
        resourcemanager_ref_image(path);

        /* done! */
        logfile_message("image_load() ok");
    }
    else
        resourcemanager_ref_image(path);

    return img;
}



/*
 * image_unref()
 * Will try to release the resource from
 * the memory. You will call this if, and
 * only if, you are sure you don't need the
 * resource anymore (i.e., you're not holding
 * any pointers to it)
 *
 * Used for reference counting. Normally you
 * don't need to bother with this, unless
 * you care about reducing memory usage.
 * Note that 'image_ref()' must not exist.
 * Returns the no. of references to the image
 */
int image_unref(const char *path)
{
    return resourcemanager_unref_image(path);
}



/*
 * image_save()
 * Saves a image to a file
 */
void image_save(const image_t *img, const char *path)
{
    char abs_path[1024];
    int i, j, c, bpp = video_get_color_depth();
    PALETTE pal;
    BITMAP *tmp;

    resource_filepath(abs_path, path, sizeof(abs_path), RESFP_WRITE);
    logfile_message("image_save(%p,'%s')", img, abs_path);

    switch(bpp) {
        case 8:
            get_palette(pal);
            save_bitmap(abs_path, img->data, pal);
            break;

        case 15:
        case 16:
        case 24:
            save_bitmap(abs_path, img->data, NULL);
            break;

        case 32:
            if(IS_PNG(abs_path)) {
                /* we must do this to make loadpng save the 32-bit image properly */
                if(NULL != (tmp = create_bitmap(img->w, img->h))) {
                    for(j=0; j<tmp->h; j++) {
                        for(i=0; i<tmp->w; i++) {
                            c = getpixel(img->data, i, j);
                            putpixel(tmp, i, j, makeacol(getr(c), getg(c), getb(c), 255));
                        }
                    }
                    save_bitmap(abs_path, tmp, NULL);
                    destroy_bitmap(tmp);
                }
            }
            else
                save_bitmap(abs_path, img->data, NULL);
            break;
    }
}



/*
 * image_create()
 * Creates a new image of a given size
 */
image_t *image_create(int width, int height)
{
    image_t *img = mallocx(sizeof *img);

    img->data = create_bitmap(width, height);
    img->w = width;
    img->h = height;

    if(img->data != NULL)
        image_clear(img, image_rgb(0,0,0));
    else
        logfile_message("ERROR - image_create(%d,%d): couldn't create bitmap", width, height);

    return img;
}


/*
 * image_destroy()
 * Destroys an image. This is called automatically
 * while unloading the resource manager.
 */
void image_destroy(image_t *img)
{
    if(img->data != NULL) {
        destroy_bitmap(img->data);
        img->data = NULL;
    }

    free(img);
}


/*
 * image_getpixel()
 * Returns the pixel at the given position on the image
 */
uint32 image_getpixel(const image_t *img, int x, int y)
{
    return getpixel(img->data, x, y);
}


/*
 * image_putpixel()
 * Plots a pixel into the given image
 */
void image_putpixel(image_t *img, int x, int y, uint32 color)
{
    putpixel(img->data, x, y, color);
}


/*
 * image_line()
 * Draws a line from (x1,y1) to (x2,y2) using the specified color
 */
void image_line(image_t *img, int x1, int y1, int x2, int y2, uint32 color)
{
    line(img->data, x1, y1, x2, y2, color);
}


/*
 * image_ellipse()
 * Draws an ellipse with the specified centre, radius and color
 */
void image_ellipse(image_t *img, int cx, int cy, int radius_x, int radius_y, uint32 color)
{
    ellipse(img->data, cx, cy, radius_x, radius_y, color);
}


/*
 * image_rectfill()
 * Draws a filled rectangle
 */
void image_rectfill(image_t *img, int x1, int y1, int x2, int y2, uint32 color)
{
    rectfill(img->data, x1, y1, x2, y2, color);
}


/*
 * image_rgb()
 * Generates an uint32 color
 */
uint32 image_rgb(uint8 r, uint8 g, uint8 b)
{
    return makecol(r,g,b);
}


/*
 * image_color2rgb()
 * Converts an uint32 to a (r,g,b) triple
 */
void image_color2rgb(uint32 color, uint8 *r, uint8 *g, uint8 *b)
{
    if(r) *r = getr(color);
    if(g) *g = getg(color);
    if(b) *b = getb(color);
}


/*
 * image_clear()
 * Clears an given image with some color
 */
void image_clear(image_t *img, uint32 color)
{
    clear_to_color(img->data, color);
}


/*
 * image_blit()
 * Blits a surface onto another
 */
void image_blit(const image_t *src, image_t *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)
{
    blit(src->data, dest->data, source_x, source_y, dest_x, dest_y, width, height);
}


/*
 * image_draw()
 * Draws an image onto the destination surface
 * at the specified position
 *
 * flags: refer to the IF_* defines (Image Flags)
 */
void image_draw(const image_t *src, image_t *dest, int x, int y, uint32 flags)
{
    if((flags & IF_HFLIP) && !(flags & IF_VFLIP))
        draw_sprite_h_flip(dest->data, src->data, x, y);
    else if(!(flags & IF_HFLIP) && (flags & IF_VFLIP))
        draw_sprite_v_flip(dest->data, src->data, x, y);
    else if((flags & IF_HFLIP) && (flags & IF_VFLIP))
        draw_sprite_vh_flip(dest->data, src->data, x, y);
    else
        draw_sprite(dest->data, src->data, x, y);
}



/*
 * image_draw_scaled()
 * Draws a scaled image onto the destination surface
 * at the specified position
 *
 * scale: (1.0, 1.0) is the original size
 *        (2.0, 2.0) stands for a double-sized image
 *        (0.5, 0.5) stands for a smaller image
 */
void image_draw_scaled(const image_t *src, image_t *dest, int x, int y, v2d_t scale, uint32 flags)
{
    image_t *tmp;
    int w = (int)(scale.x * src->w);
    int h = (int)(scale.y * src->h);

    tmp = image_create(w, h);
    stretch_blit(src->data, tmp->data, 0, 0, src->w, src->h, 0, 0, w, h);
    image_draw(tmp, dest, x, y, flags);
    image_destroy(tmp);
}


/*
 * image_draw_rotated()
 * Draws a rotated image onto the destination bitmap at the specified position 
 *
 * ang: angle given in radians
 * cx, cy: pivot positions
 */
void image_draw_rotated(const image_t *src, image_t *dest, int x, int y, int cx, int cy, float ang, uint32 flags)
{
    float conv = (-ang * (180.0f/PI)) * (64.0f/90.0f);

    if((flags & IF_HFLIP) && !(flags & IF_VFLIP))
        pivot_sprite_v_flip(dest->data, src->data, x, y, src->w - cx, src->h - cy, ftofix(conv + 128.0f));
    else if(!(flags & IF_HFLIP) && (flags & IF_VFLIP))
        pivot_sprite_v_flip(dest->data, src->data, x, y, cx, src->h - cy, ftofix(conv));
    else if((flags & IF_HFLIP) && (flags & IF_VFLIP))
        pivot_sprite(dest->data, src->data, x, y, src->w - cx, src->h - cy, ftofix(conv + 128.0f));
    else
        pivot_sprite(dest->data, src->data, x, y, cx, cy, ftofix(conv));
}

 
/*
 * image_draw_trans()
 * Draws a translucent image
 *
 * alpha: 0.0 (invisible) <= alpha <= 1.0 (opaque)
 */
void image_draw_trans(const image_t *src, image_t *dest, int x, int y, float alpha, uint32 flags)
{
    image_t *tmp;
    int a;

    if(video_get_color_depth() > 8) {
        alpha = clip(alpha, 0.0, 1.0);
        a = (int)(255 * alpha);
        set_trans_blender(a, a, a, a);

        tmp = image_create(src->w, src->h);
        image_clear(tmp, video_get_maskcolor());
        image_draw(src, tmp, 0, 0, flags);
        draw_trans_sprite(dest->data, tmp->data, x, y);
        image_destroy(tmp);
    }
    else
        image_draw(src, dest, x, y, flags);
}


/*
 * image_draw_lit()
 * Draws an image tinted with a specific color
 *
 * 0.0 <= alpha <= 1.0
 */
void image_draw_lit(const image_t *src, image_t *dest, int x, int y, uint32 color, float alpha, uint32 flags)
{
    image_t *tmp;
    uint8 r, g, b;
    int a;

    if(video_get_color_depth() > 8) {
        alpha = clip(alpha, 0.0, 1.0);
        image_color2rgb(color, &r, &g, &b);
        a = (int)(255 * alpha);
        set_trans_blender(r, g, b, a);

        tmp = image_create(src->w, src->h);
        image_clear(tmp, video_get_maskcolor());
        image_draw(src, tmp, 0, 0, flags);
        draw_lit_sprite(dest->data, tmp->data, x, y, a);
        image_destroy(tmp);
    }
    else
        image_draw(src, dest, x, y, flags);
}


/*
 * image_pixelperfect_collision()
 * Pixel perfect collision detection
 */
int image_pixelperfect_collision(const image_t *img1, const image_t *img2, int x1, int y1, int x2, int y2)
{
    int i, j;
    uint32 mask = video_get_maskcolor();
    int (*fast_getpixel)(BITMAP*,int,int);

    /* optimizing */
    if(img1->w * img1->h > img2->w * img2->h)
        return image_pixelperfect_collision(img2, img1, x2, y2, x1, y1);

    /* fast getpixel routine */
    switch(video_get_color_depth()) {
        case 8:  fast_getpixel = _getpixel;   break;
        case 16: fast_getpixel = _getpixel16; break;
        case 24: fast_getpixel = _getpixel24; break;
        case 32: fast_getpixel = _getpixel32; break;
        default: fast_getpixel = getpixel; break;
    }

    /* loop */
    for(i=0; i<img1->h; i++) { /* i-th row */
        for(j=0; j<img1->w; j++) { /* j-th col */
            if(fast_getpixel(img1->data, j, i) != mask) {
                /* pixel position: (x1+j, y1+i) */
                if(x1+j >= x2 && x1+j < x2+img2->w && y1+i >= y2 && y1+i < y2+img2->h) {
                    if(fast_getpixel(img2->data, x1+j-x2, y1+i-y2) != mask)
                        return TRUE;
                }
            }
        }
    }

    /* fail :( */
    return FALSE;
}


/* private methods */

/*
 * maskcolor_bugfix()
 * When loading certain PNGs, magenta (color key) is
 * not considered transparent. Let's fix this.
 */
void maskcolor_bugfix(image_t *img)
{
    int i, j;
    uint32 mask = video_get_maskcolor();
    uint8 pixel_r, pixel_g, pixel_b, mask_r, mask_g, mask_b;
    image_color2rgb(mask, &mask_r, &mask_g, &mask_b);

    for(j=0; j<img->h; j++) {
        for(i=0; i<img->w; i++) {
            image_color2rgb(image_getpixel(img, i, j), &pixel_r, &pixel_g, &pixel_b);
            if(pixel_r == mask_r && pixel_g == mask_g && pixel_b == mask_b)
                image_putpixel(img, i, j, mask);
        }
    }
}

