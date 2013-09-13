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
#include "extract-common.h"

#include "file.h"
#include "icon-dir.h"

#include <png.h>
#include <glib.h>

/**
 * really just memcpys the data
 */
static void read_png_from_mem_buf(png_structp png_ptr, png_bytep out_bytes, png_size_t byte_count_to_read);

gboolean peek_width_height(GMappedFile *file, char* data_ptr, gint* width, gint* height, gint *bpp)
{
    if (memcmp(data_ptr, "\211PNG\r\n\032\n", 8) == 0) {
        // a png file
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        g_return_val_if_fail(png_ptr != NULL, FALSE);
        png_infop png_info_ptr = png_create_info_struct(png_ptr);
        if (!png_info_ptr) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            g_assert_not_reached();
            return FALSE;
        }
        png_infop png_end_info_ptr = png_create_info_struct(png_ptr);
        if (!png_end_info_ptr) {
            png_destroy_read_struct(&png_ptr, &png_info_ptr, NULL);
            g_assert_not_reached();
            return FALSE;
        }
        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_read_struct(&png_ptr, &png_info_ptr, &png_end_info_ptr);
            return FALSE;
        }
        png_set_read_fn(png_ptr, data_ptr, read_png_from_mem_buf);

        // read info
        png_read_info(png_ptr, png_info_ptr);

        png_uint_32 png_width = png_get_image_width(png_ptr, png_info_ptr);
        png_uint_32 png_height = png_get_image_height(png_ptr, png_info_ptr);

        // uint to int conversion could create bullshit, let's just hope width and height are not too big
        g_assert(png_width <= INT_MAX); g_assert(png_height <= INT_MAX);
        *width = (gint)png_width;
        *height = (gint)png_height;

        // cleanup png
        png_destroy_read_struct(&png_ptr, &png_info_ptr, &png_end_info_ptr);

        //FIXME: we _assume_ the png is on the same quality level as a 32bit bmp,
        // so we treat it as a 32bit image without actually checking the bit depth.
        // This is obviously lazy but I've yet to see an icon file that isn't
        // compatible with this approach
        *bpp = 32;
        return TRUE;
    } else {
        // assumed to be a bitmap header
        *width = bitmap_info_header_width(data_ptr);
        *height = bitmap_info_header_height(data_ptr) / 2;
        *bpp = bitmap_info_header_bpp(data_ptr);
        return TRUE;
    }
}

void read_png_from_mem_buf(png_structp png_ptr, png_bytep out_bytes, png_size_t byte_count_to_read)
{
    memcpy(out_bytes, png_ptr->io_ptr, byte_count_to_read);
    png_ptr->io_ptr = (char *)png_ptr->io_ptr + byte_count_to_read;
}




