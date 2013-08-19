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
#include "file.h"
#include "extract-icon.h"
#include "extract-pe.h"
#include "write-icons.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib/gprintf.h>
#include <stdio.h>

// option values
static gchar *output_dir = NULL; // icon theme directory
static gchar *output_name = NULL; // the name of the application
static gchar *input_file = NULL; // .ico, .dll or .exe file
static gint   icon_no = 0; // icon number
static gint   max_bpp = G_MAXINT; // ignore all images with higher bpp
// operation flags
static gboolean list_icons = FALSE; // just list all available icons

// option entries
static GOptionEntry entries[] = {
    { "directory", 'd', 0, G_OPTION_ARG_FILENAME, &output_dir,  N_("The directory to place the resulting files. Defaults to the current directory."), "path" },
    { "name",      'n', 0, G_OPTION_ARG_STRING,   &output_name, N_("The name to give to the icon. Defaults to the base name of the input file."),     "myapp"},
    { "index",     'i', 0, G_OPTION_ARG_INT,      &icon_no,     N_("The index of the icon to extract. Defaults to 0."),                               "NUM"  },
    { "list",      'l', 0, G_OPTION_ARG_NONE,     &list_icons,  N_("Do not extract anything but output a list of all existing icons."),               NULL   },
    { "max-bpp",   0,   0, G_OPTION_ARG_INT,      &max_bpp,     N_("Do not extract images with more bits per pixel"),                                 "BPP"  },
    { NULL }
};

/**
 * parses the command line and gives control over to the extract function
 */
int main(int argc, char **argv)
{
    GError *error = NULL;
    GOptionContext *context;

    context = g_option_context_new(N_("INPUTFILE - extract icons from windows .dll, .exe and .ico files"));
    g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_printerr(_("Error while parsing comamnd line: %s\n"), error->message);
        g_clear_error(&error);
        return 1;
    } else {
        // we _assume_ that argv[1] is in the correct file name encoding, otherwise we're doomed
        if (argc != 2) {
            g_printerr(_("Incorrect number of arguments. See %s --help\n"), g_get_prgname());
            return 1;
        }
        input_file = argv[1];

        // set output dir
        if (output_dir == NULL) {
            output_dir = g_get_current_dir();
        }
        // set basename
        if (output_name == NULL) {
            output_name = g_path_get_basename(input_file);
        }

        // open the file into memory
        GMappedFile *file;
        if (!(file = g_mapped_file_new(input_file, FALSE, &error))) {
            g_printerr(_("Error while reading `%s': %s\n"), input_file, error->message);
            return 1;
        }

        GHashTable *table;

        // check magic of icon directory
        if (is_icon_file(file)) {
            // we have an icon file -> extract icons
            g_debug("icon file");

            GHashTable *table = parse_icon_file(file, max_bpp);

            if (list_icons) {
                GList *keys = g_hash_table_get_keys(table);
                if (keys) {
                    do {
                        if (keys->data) {
                            g_printf("%s\n", keys->data);
                        }
                    } while (keys = g_list_next(keys));
                    g_list_free(g_list_first(keys));
                }
            } else {
                //extract
                write_icon_table_to_disk(output_dir, output_name, table);
            }

            g_hash_table_unref(table);
        } else if (is_pe_file(file)) {
            g_debug("pe file");

            char *grp_icon_dir;
            if (list_icons) {
                // enumerate all possible icons
                for (int i = 0; grp_icon_dir = get_grp_icon_dir_from_pe(file, i); ++i) {
                    g_printf("%d: ", i);

                    GHashTable *table = parse_group_icon_dir(file, grp_icon_dir, max_bpp);
                    GList *keys = g_hash_table_get_keys(table);
                    if (keys) {
                        do {
                            if (keys->data) {
                                g_printf("%s ", keys->data);
                            }
                        } while (keys = g_list_next(keys));
                        g_list_free(g_list_first(keys));
                    }
                    g_printf("\n");
                    g_hash_table_unref(table);
                }
            } else {
                // extract icon specified by icon_no
                char *grp_icon_dir = get_grp_icon_dir_from_pe(file, icon_no);
                if (grp_icon_dir == NULL) {
                    g_printerr(_("ERROR: Icon number %d does not exist in `%s'\n"), icon_no, input_file);
                    return 1;
                }

                GHashTable *table = parse_group_icon_dir(file, grp_icon_dir, max_bpp);
                g_assert(table != NULL); //really, there's no way that could happen

                write_icon_table_to_disk(output_dir, output_name, table);
                g_hash_table_unref(table);
            }
        } else {
            g_printerr(_("Error while reading `%s': File format not recognized. Only ICO and PE files are supported.\n"), input_file);
            return 1;
        }

        g_mapped_file_unref(file);
    }

    g_option_context_free(context);
}