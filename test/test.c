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

#include <glib.h>

#ifndef EXTRACTOR_TEST_DATA
#   error "define EXTRACTOR_TEST_DATA as string pointing to the path of the test data"
#endif

/**
 * simple function that does some self checks for the png compare code
 */
static void png_compare_selfcheck(void)
{
    // yeah, really basic
    g_assert(0 == pixel_compare_png_file(EXTRACTOR_TEST_DATA "/one.png", EXTRACTOR_TEST_DATA "/one.png"));

    // better, compare with the carefully optimized png file
    g_assert(0 == pixel_compare_png_file(EXTRACTOR_TEST_DATA "/one.png", EXTRACTOR_TEST_DATA "/one.opti.png"));

    // compare with different picture
    g_assert(0 != pixel_compare_png_file(EXTRACTOR_TEST_DATA "/one.png", EXTRACTOR_TEST_DATA "/another.png"));
}

/**
 * compares newly extracted data from a given file with the previously extracted and considered valid data
 */
static void check_extractor_for_file(const char *file)
{
    gchar *src_file = g_build_filename(EXTRACTOR_TEST_DATA, file, NULL);
    gchar *file_png = g_strdup_printf("%s.png", file);

    //FIXME: should this really be fixed?
    gint bpps_to_test[] = { 1, 4, 8, 16, 24, 32 }; // and the special INT_MAX directory

    for (int i = 0; i < G_N_ELEMENTS(bpps_to_test); ++i) {
        // first launch the extractor
        int status;
        gchar *command_line = g_strdup_printf("\"%s\" -d \"%s\" --max-bpp=%d \"%s\"", EXTRACTOR_BINARY,
                                              EXTRACTOR_TEST_TMP, bpps_to_test[i], src_file, NULL);
        g_assert(g_spawn_command_line_sync(command_line, NULL, NULL, &status, NULL));
        g_assert(status == 0);

        g_free(command_line);

        // now check whether all the right files are there and equal
        gchar *bpp_str = g_strdup_printf("%d", bpps_to_test[i]);
        gchar *compare_directory = g_build_filename(EXTRACTOR_TEST_DATA, "extracted-data-to-compare", file, bpp_str, NULL);
        GDir *compare_dir = g_dir_open(compare_directory, 0, NULL);
        g_assert(compare_dir); // well this would be bad if it failed

        const gchar *size;
        while (size = g_dir_read_name(compare_dir)) {
            gchar *newly_extracted_file = g_build_filename(EXTRACTOR_TEST_TMP, size, "apps", file_png, NULL);
            gchar *master_file = g_build_filename(compare_directory, size, NULL);

            // magic happens here - we test the individual files on pixel equality
            g_assert(0 == pixel_compare_png_file(newly_extracted_file, master_file));

            g_free(newly_extracted_file);
            g_free(master_file);
        }
        g_dir_close(compare_dir);

        g_free(compare_directory);
        g_free(bpp_str);
    }

    g_free(file_png);
    g_free(src_file);
}


int main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);

    g_debug("Directory with test data: %s", EXTRACTOR_TEST_DATA);
    g_debug("Extractor executable to test: %s", EXTRACTOR_BINARY);
    g_debug("Directory to place temporary extraction results: %s", EXTRACTOR_TEST_TMP);

    g_test_add_func("/msicon-extractor/pngcompare-selfcheck", png_compare_selfcheck);
    g_test_add_data_func("/msicon-extractor/extract-pe", "firefox.exe", (GTestDataFunc)check_extractor_for_file);
    g_test_add_data_func("/msicon-extractor/extract-pe32+", "firefox64.exe", (GTestDataFunc)check_extractor_for_file);
    g_test_add_data_func("/msicon-extractor/extract-ico", "ff4bpp.ico", (GTestDataFunc)check_extractor_for_file);

    return g_test_run();
}