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
#ifndef EXTRACT_COMMON_H
#define EXTRACT_COMMON_H

#include <glib.h>

/**
 * extraction functions common for both pe and ico files
 */


/**
 * looks for with and height in the png file or bitmapinfoheader
 */
gboolean peek_width_height(GMappedFile *file, char *icon_data, gint *width, gint *height, gint *bpp);

#endif // EXTRACT_COMMON_H
