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
#include "write-icons.h"
#include "iconbmp2png.h"

#include <png.h>
#include <errno.h>
#include <glib/gi18n-lib.h>

void icon_info_free(IconInfo* info)
{
    if (info->image_data_free) {
        info->image_data_free(info->image_data);
    }
    g_free(info);
}

gboolean write_icon_table_to_disk(const gchar *base_dir, const gchar *appname, GHashTable* table)
{
    gboolean retval = TRUE;

    GList *keys = g_hash_table_get_keys(table);
    if (!keys) return retval;
    
    do {
        if (keys->data) {
            gchar *dirname = g_build_filename(base_dir, keys->data, "apps", NULL);
            gchar *filename = g_strdup_printf("%s.png", appname);
            gchar *filename_full = g_build_filename(dirname, filename, NULL);

            // create directory
            if (g_mkdir_with_parents(dirname, 0755) == -1) {
                g_printerr(_("ERROR: Could not create directory `%s': %s\n"), dirname, g_strerror(errno));
                retval = FALSE;
                goto freestuff;
            }

            // open file
            FILE *fp = fopen(filename_full, "wb");
            if (!fp) {
                g_printerr(_("ERROR: Could not open file `%s' for reading: %s\n"), filename_full, g_strerror(errno));
                retval = FALSE;
                goto freestuff;
            }

            // check if we have a png file first
            IconInfo *info = (IconInfo*)g_hash_table_lookup(table, keys->data);
            g_return_val_if_fail(info != NULL, FALSE);
            char *icon_data = info->image_data;
            if (png_sig_cmp(icon_data, 0, 8) == 0) {
                g_debug("png file, passing through: %s", keys->data);
                // png files will be just passed through
                size_t bytes_written = fwrite(icon_data, 1, info->image_size, fp);
                if (bytes_written != info->image_size) {
                    g_printerr(_("ERROR: Only %d of %d bytes could be written to `%s': %s\n"),
                               bytes_written, info->image_size, filename_full, g_strerror(errno));
                    retval = FALSE;
                }
            } else {
                // delegate to conversion routine
                retval = retval && icon_bitmap_to_png(icon_data, fp);
            }

            // close file
            fclose(fp); // it could fail, but what the hell should we do in that case? ignoring it is easier and has the same effect

            // goto is used to avoid nesting too many else clauses, we need to free the allocated filenames again, that's all.
            // code me a prettier solution and I'll merge it
            freestuff:
            g_free(dirname);
            g_free(filename);
            g_free(filename_full);
        }
    } while (keys = g_list_next(keys));

    return retval;
}

void read_png_from_mem_buf(png_structp png_ptr, png_bytep out_bytes, png_size_t byte_count_to_read)
{
    memcpy(out_bytes, png_ptr->io_ptr, byte_count_to_read);
    png_ptr->io_ptr += byte_count_to_read;
}



