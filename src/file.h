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

/** @file
 * functions for dealing with a mapped icon or pe file.
 */

#ifndef FILE_H
#define FILE_H

#include <glib.h>

/**
 * checks whether an offset is safe to be casted to a pointer
 */
gboolean is_offset_valid(GMappedFile *file, guint32 offset, guint32 size);

/**
 * gets a pointer to a structure represented by the given offset.
 * Will return a NULL pointer if the offset was invalid
 */
gchar* offset_to_ptr(GMappedFile *file, guint32 offset, guint32 size);

/**
 * returns an offset generated from a pointer
 */
guint32 ptr_to_offset(GMappedFile *file, gpointer data);

/**
 * checks whether a pointer is safe to be accessed
 */
gboolean is_ptr_valid(GMappedFile *file, gpointer data, guint32 size);

#endif // FILE_H
