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

#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <glib.h>

/**
 * pixel wise compare two png files. The png files need to have the same bpp and size.
 */
int pixel_compare_png(FILE *file0, FILE *file1);

/**
 * pixel compare two png files using pixel_compare_png
 */
int pixel_compare_png_file(const char *filename0, const char *filename1);

/**
 * recursively delete a directory
 *
 * @returns the number of files deleted, or -1 in case of error
 */ 
int recursively_delete_dir(const char *dirname, GError **error);

#endif // UTIL_H
