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
 * hosts the only function that creates a png file from an icon data.
 */
#ifndef ICONBMP2PNG_H
#define ICONBMP2PNG_H

#include <glib.h>
#include <glib/gstdio.h>

/**
 * Writes the icon data located at bitmap_data to the already opened file.
 *
 * @returns whether the operation was successful
 */
gboolean icon_bitmap_to_png(char *bitmap_data, FILE *fp);


#endif // ICONBMP2PNG_H
