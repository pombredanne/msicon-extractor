//  Copyright (C) 2013 Jonas Kuemmerlin <rgcjonas@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "iconbmp2png.h"
#include "icon-dir.h"

#include <glib.h>
#include <png.h>

/**
 * function to return the pixel value in an indexed pixel format with a bpp <= 8
 */
static guint8 get_pixel_from_row(guint8 *row, int bpp, int x) {
    g_return_val_if_fail(bpp <= 8, 0);

    return (row[x * bpp / 8] >> (8 - bpp - (x % (8 / bpp)) * bpp)) & ((1 << bpp) - 1);
}

/**
 * returns true if the pixel specified by x in the given row of the and map
 * is opaque
 */
static gboolean pixel_opaque(guint8 *and_row, int x)
{
    return get_pixel_from_row(and_row, 1, x) == 0;
}

/**
 * return the next multiple of @param n being >= @param arg
 */
static int next_multiple(int n, int arg)
{
    if (arg % n == 0) return arg;
    else return (arg / n) * n + n;
}

// this is a mega function which does a lot of dirty work
gboolean icon_bitmap_to_png(char* bitmap_data_ptr, FILE* fp)
{
    // first get the info from the header
    int bpp = bitmap_info_header_bpp(bitmap_data_ptr);
    int width = bitmap_info_header_width(bitmap_data_ptr);
    int height = bitmap_info_header_height(bitmap_data_ptr) / 2;
    int colors = bitmap_info_header_n_colors(bitmap_data_ptr);
    if (colors == 0 && bpp < 16) {
        // use maximum colors. which drugs does one need to take in order to think of this as a good idea?
        colors = 1 << bpp;
    }

    // calculate widths and offsets
    // both xor and and rows are padded to 4 byte boundaries
    //FIXME: theoretically we could generate overflows here, we just hope noone feeds us with exorbitant icon sizes
    int xor_row_width = width * bpp / 8.0f; // ensure rounding to full bytes, necessary for bpp < 8
    xor_row_width = next_multiple(4, xor_row_width); // the spec mandates 4 byte padding
    int and_row_width = width / 8.0f; // make sure we round up to full bytes
    and_row_width = next_multiple(4, and_row_width);

    guint8 *color_tbl = G_STRUCT_MEMBER_P(bitmap_data_ptr, bitmap_info_header_get_size(bitmap_data_ptr));
    guint8 *xor_map = G_STRUCT_MEMBER_P(color_tbl, colors * 4);
    guint8 *and_map = G_STRUCT_MEMBER_P(xor_map, height * xor_row_width);

    //debug
    g_debug("writing bitmap at %p: %dx%dx%d with %d colors", bitmap_data_ptr, width, height, bpp, colors);

    // init png
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    g_return_val_if_fail(png_ptr, FALSE);

    png_infop png_info = png_create_info_struct(png_ptr);
    if (!png_info) {
        png_destroy_write_struct(&png_ptr, NULL);
        g_assert_not_reached();
        return FALSE;
    }

    // error handling
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &png_info);
        g_assert_not_reached();
        return FALSE;
    }

    png_init_io(png_ptr, fp);

    // set the info header
    png_set_IHDR(png_ptr, png_info, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // begin writing, write info header
    png_write_info(png_ptr, png_info);

    // allocate single row buffer
    guint8 *row = g_new0(guint8, 4 * width);

    // write image row by row
    for (int y = 1; y <= height; ++y) {
        guint8 *xor_row = &xor_map[(height - y) * xor_row_width];
        guint8 *and_row = &and_map[(height - y) * and_row_width];

        // fill row buffer based on bpp
        switch (bpp) {
            case 24:
                // stored as 3 byte triplet in bgr format
                for (int x = 0; x < width; ++x) {
                    // find pixel in bitmap file
                    guint8 *xor_pixel = &xor_row[x * 3];
                    guint8 *buffer_pixel = &row[x * 4];
                    buffer_pixel[0] = xor_pixel[2]; //r
                    buffer_pixel[1] = xor_pixel[1]; //g
                    buffer_pixel[2] = xor_pixel[0]; //b
                    buffer_pixel[3] = pixel_opaque(and_row, x) ? 0xFF : 0;
                }
                break;
            case 32:
                // stored as 4 byte bgra quad
                for (int x = 0; x < width; ++x) {
                    guint8 *xor_pixel = &xor_row[x * 4];
                    guint8 *buffer_pixel = &row[x * 4];
                    buffer_pixel[0] = xor_pixel[2]; //r
                    buffer_pixel[1] = xor_pixel[1]; //g
                    buffer_pixel[2] = xor_pixel[0]; //b
                    buffer_pixel[3] = xor_pixel[3]; //a
                }
                break;
            case 16:
                // stored as uint16, each color is stored as 5 bits
                // the least significant 5 bits is blue, followed by green and red
                // most significant bit is not used
                for (int x = 0; x < width; ++x) {
                    guint16 xor_pixel = GUINT16_FROM_LE(G_STRUCT_MEMBER(guint16, xor_row, x * 2));
                    guint8 *buffer_pixel = &row[x * 4];

                    //FIXME: I haven't found any 16bpp icons to test the conversion
                    int r = (xor_pixel >> 10) & 31;
                    int g = (xor_pixel >> 5) & 31;
                    int b = xor_pixel & 31;

                    buffer_pixel[0] = (r << 3) | (r >> 2);
                    buffer_pixel[1] = (g << 3) | (g >> 2);
                    buffer_pixel[2] = (b << 3) | (b >> 2);
                    buffer_pixel[3] = pixel_opaque(and_row, x) ? 0xFF : 0;
                }
                break;
            case 8:
            case 4:
            case 1:
                // stored in one byte per pixel, indexed colors
                for (int x = 0; x < width; ++x) {
                    guint8 xor_pixel = get_pixel_from_row(xor_row, bpp, x);
                    guint8 *buffer_pixel = &row[x * 4];
                    guint8 *color = &color_tbl[xor_pixel * 4];

                    // copy color values
                    buffer_pixel[0] = color[2];
                    buffer_pixel[1] = color[1];
                    buffer_pixel[2] = color[0];
                    buffer_pixel[3] = pixel_opaque(and_row, x) ? 0xFF : 0;
                }
                break;
            default:
                g_warning("unrecognized bpp value: %d", bpp);
        }

        // write row buffer to file
        png_write_row(png_ptr, row);
    }

    // finalize writing
    g_free(row);
    png_write_end(png_ptr, png_info);
    png_destroy_write_struct(&png_ptr, &png_info);

    return TRUE;
}

