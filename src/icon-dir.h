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

#ifndef ICON_DIR_H
#define ICON_DIR_H

#include <glib.h>
#include <string.h>

/** @file
 * Routines and macros for dealing with the icon structures.
 */

/**
 * checks whether the structure at *icon_dir is an IconDir by checking the magic bytes
 */
static inline gboolean is_icon_dir(char *icon_dir) {
    return memcmp(icon_dir, "\0\0\x01", 4) == 0;
}

// IconDir and IconDirEntry

/**
 * returns the length of the structure, doesn't include the entries
 */
#define ICON_DIR_SIZE 6

/**
 * returns the number of entries in an icon dir
 */
#define icon_dir_n_entries(icon_dir) GUINT16_FROM_LE(G_STRUCT_MEMBER(guint16, icon_dir, 4))

/**
 * returns a pointer to the entry with the index i
 */
#define icon_dir_entry(icon_dir, i) G_STRUCT_MEMBER_P(icon_dir, 6 + i * ICON_DIR_ENTRY_SIZE)

/**
 * size of an entry
 */
#define ICON_DIR_ENTRY_SIZE 16

/**
 * width as specified in an icon dir entry
 */
#define icon_dir_entry_width(icon_dir_entry_p) G_STRUCT_MEMBER(guint8, icon_dir_entry_p, 0)

/**
 * height as specified in an icon dir entry
 */
#define icon_dir_entry_height(icon_dir_entry_p) G_STRUCT_MEMBER(guint8, icon_dir_entry_p, 1)

/**
 * bpp as specified in an icon dir entry
 */
#define icon_dir_entry_bpp(icon_dir_entry_p) GUINT16_FROM_LE(G_STRUCT_MEMBER(guint16, icon_dir_entry_p, 6))

/**
 * size of the associated image data
 */
#define icon_dir_entry_bytes(icon_dir_entry_p) GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, icon_dir_entry_p, 8))

/**
 * rva of the associated image data
 */
#define icon_dir_entry_offset(icon_dir_entry_p) GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, icon_dir_entry_p, 12))


// Bitmap Info Header

/**
 * the "static" size of the header; the value returned can be used for access validation as it contains
 * all emenents accessed by the various functions, but it may not be the actual size of the structure, see
 * @link bitmap_info_header_get_size if you need that.
 */
#define BITMAP_INFO_HEADER_SIZE 40

/**
 * the actual size of the bitmap pointed to by bitmap_p
 */
#define bitmap_info_header_get_size(bitmap_p) GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, bitmap_p, 0))

/**
 * the width of the bitmap
 */
#define bitmap_info_header_width(bitmap_p) GINT32_FROM_LE(G_STRUCT_MEMBER(gint32, bitmap_p, 4))

/**
 * the height of the bitmap
 *
 * WARNING: in an icon resource, the value returned is the height of the xor and the and map
 */
#define bitmap_info_header_height(bitmap_p) GINT32_FROM_LE(G_STRUCT_MEMBER(gint32, bitmap_p, 8))

/**
 * the bits per pixel of the bitmap
 */
#define bitmap_info_header_bpp(bitmap_p) GUINT16_FROM_LE(G_STRUCT_MEMBER(guint16, bitmap_p, 14))

/**
 * the number of colors in the bitmap. If this is 0 and bpp < 16, the maximum amount of colors are used.
 */
#define bitmap_info_header_n_colors(bitmap_p) GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, bitmap_p, 36))



#endif //ICON_DIR_H