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

#include "util.h"

#include <png.h>
#include <glib.h>
#include <glib/gstdio.h>

int pixel_compare_png(FILE* file0, FILE* file1)
{
    // initialize png ptrs
    png_structp png0 = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_structp png1 = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png0 || !png1) {
        g_error("initializing libpng failed!");
        g_assert_not_reached();
    }

    png_infop png_info0 = png_create_info_struct(png0);
    png_infop png_info1 = png_create_info_struct(png1);

    if (!png_info0 || !png_info1) {
        png_destroy_read_struct(&png0, NULL, NULL);
        png_destroy_read_struct(&png1, NULL, NULL);
        g_error("creating png info struct failed!");
        g_assert_not_reached();
    }

    if (setjmp(png_jmpbuf(png0)) || setjmp(png_jmpbuf(png1))) {
        png_destroy_read_struct(&png0, &png_info0, NULL);
        png_destroy_read_struct(&png1, &png_info1, NULL);

        g_error("error occured while reading png file");
        g_assert_not_reached();
    }

    // init io
    png_init_io(png0, file0);
    png_init_io(png1, file1);

    // now read the png
    png_read_png(png0, png_info0, 0, NULL);
    png_read_png(png1, png_info1, 0, NULL);

    guint32 height0 = png_get_image_height(png0, png_info0);
    guint32 height1 = png_get_image_height(png1, png_info1);
    g_assert(height0 == height1);

    guint32 rowbytes0 = png_get_rowbytes(png0, png_info0);
    guint32 rowbytes1 = png_get_rowbytes(png1, png_info1);
    // we cant reliability compare images with different encodings
    g_assert(rowbytes0 == rowbytes1);

    png_bytepp rows0 = png_get_rows(png0, png_info0);
    png_bytepp rows1 = png_get_rows(png1, png_info1);

    // now memcmp the rows
    for (int i = 0; i < height0; ++i) {
        int result = memcmp(rows0[i], rows1[i], rowbytes0);
        if (result != 0) {
            return result;
        }
    }

    png_destroy_read_struct(&png0, &png_info0, NULL);
    png_destroy_read_struct(&png1, &png_info1, NULL);

    return 0;
}

int pixel_compare_png_file(const char* filename0, const char* filename1)
{
    FILE *fp0 = fopen(filename0, "rb");
    if (!fp0) {
        g_error("`%s' could not be opened!", filename0);
        g_assert_not_reached();
    }
    FILE *fp1 = fopen(filename1, "rb");
    if (!fp1) {
        g_error("`%s' could not be opened!", filename1);
        g_assert_not_reached();
    }

    int retval = pixel_compare_png(fp0, fp1);

    fclose(fp0);
    fclose(fp1);

    return retval;
}


