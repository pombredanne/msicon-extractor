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
#ifndef EXTRACT_PE_H
#define EXTRACT_PE_H

#include <glib.h>

/**
 * Return a pointer to a the GrpIconDir with numer i
 */
char *get_grp_icon_dir_from_pe(GMappedFile *pe_file, int i);

/**
 * Whether the file is a valid pe file
 */
gboolean is_pe_file(GMappedFile *file);

/**
 * parses a group icon directory and returns the well known hashtable of IconInfos
 *
 * @see extract-icon.h
 */
GHashTable *parse_group_icon_dir(GMappedFile *file, char *group_icon_dir, gint max_bpp);

#endif // EXTRACT_PE_H
