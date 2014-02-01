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
#include "extract-icon.h"
#include "config.h"

#include "icon-dir.h"
#include "file.h"
#include "extract-common.h"
#include "write-icons.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <png.h>

gboolean is_icon_file(GMappedFile* file)
{
    return is_icon_dir(offset_to_ptr(file, 0, 4));
}

GHashTable* parse_icon_file(GMappedFile* file, gint max_bpp)
{
    char *dir = offset_to_ptr(file, 0, ICON_DIR_SIZE);
    GHashTable *table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)icon_info_free);
    for (int i = 0; i < icon_dir_n_entries(dir); ++i) {
        g_assert(is_ptr_valid(file, icon_dir_entry(dir, i), ICON_DIR_ENTRY_SIZE));
        char *entry = icon_dir_entry(dir, i);

        // width and height might be defined in the bitmap header instead
        gint width = icon_dir_entry_width(entry);
        gint height = icon_dir_entry_height(entry);
        gint bpp = icon_dir_entry_bpp(entry);

        char *data = offset_to_ptr(file, icon_dir_entry_offset(entry), icon_dir_entry_bytes(entry));

        if (width == 0 || height == 0 || bpp == 0) {
            // width and height are always missing as soon as they are >= 256, bpp is sometimes missing too
            //FIXME: why the bpp?
            if (!peek_width_height(file, data, &width, &height, &bpp)) {
                g_warning("No valid width and height found for IconDirEntry no. %d", i);
                continue;
            }
        }

        if (bpp > max_bpp) continue;

        gchar *key = g_strdup_printf("%dx%d", width, height);
        if (!g_hash_table_contains(table, key) ||
                ((IconInfo*)g_hash_table_lookup(table, key))->bpp < bpp) {
            IconInfo *info = g_malloc(sizeof(IconInfo));
            info->width = width;
            info->height = height;
            info->bpp = bpp;
            info->image_data = data;
            info->image_size = icon_dir_entry_bytes(entry);
            info->image_data_free = NULL;
            g_hash_table_replace(table, key, info);
            g_debug("placed into hashtable: variant #%d at %dx%dx%d", i, width, height, bpp);
        } else {
            g_free(key);
            g_debug("not placing into hashtable: variant #%d at %dx%dx%d", i, width, height, bpp);
        }
    }
    return table;
}


