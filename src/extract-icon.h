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

/**
 * Functions for extracting icons from an IconDir
 */

#ifndef EXTRACT_ICON_H
#define EXTRACT_ICON_H

#include <glib.h>

/**
 * walks through an IconDir structure and returns a hash table mapping a string
 * like "WIDTHxHEIGHT" to a matching @link IconInfo. If there are multiple icons
 * with the same size, only the one with the highest bpp is returned.
 *
 * @returns a newly allocated GHashTable of newly allocated strings and newly allocated IconInfos
 */
GHashTable *parse_icon_file(GMappedFile *file, gint max_bpp);

/**
 * returns whether the file is an icon file
 */
gboolean is_icon_file(GMappedFile *file);

#endif // EXTRACT_ICON_H
