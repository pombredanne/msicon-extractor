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
#ifndef WRITE_ICONS_H
#define WRITE_ICONS_H

#include <glib.h>

/**
 * this structure carries information about an icon.
 */
typedef struct {
    gint width;
    gint height;
    gint bpp;
    // image data
    gchar *image_data; ///< pointer to the image data, may be a bitmap or a png file
    void (*image_data_free)(char*); ///< pointer to a free function for the image data, may be null if the data is static
    gsize image_size; ///< number of bytes in the image
} IconInfo;

/**
 * frees an @link IconInfo structure
 */
void icon_info_free(IconInfo *info);

/**
 * writes the hastable of icon infos to a directory tree on disk, converting
 * every icon to png
 */
gboolean write_icon_table_to_disk(const gchar *base_dir, const gchar *appname, GHashTable *table);

#endif // WRITE_ICONS_H
