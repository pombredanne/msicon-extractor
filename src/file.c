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

gboolean is_offset_valid(GMappedFile* file, guint32 rva, guint32 size)
{
    return rva + size <= g_mapped_file_get_length(file);
}

gchar* offset_to_ptr(GMappedFile* file, guint32 rva, guint32 size)
{
    g_return_val_if_fail(is_offset_valid(file, rva, size), NULL);
    return g_mapped_file_get_contents(file) + rva;
}

guint32 ptr_to_offset(GMappedFile* file, gpointer data)
{
    return (char *)data - g_mapped_file_get_contents(file);
}

gboolean is_ptr_valid(GMappedFile* file, gpointer data, guint32 size)
{
    return (char *)data >= g_mapped_file_get_contents(file) &&
        ((char *)data + size) <= (g_mapped_file_get_contents(file) + g_mapped_file_get_length(file));
}



