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
#ifndef PE_H
#define PE_H

#include <glib.h>
#include <string.h>
#include "file.h"

/**
 * functions and macros for dealing with PE files
 */

/**
 * returns a pointer to the pe optional header
 */
static inline char *get_pe_optional_header(GMappedFile *file)
{
    guint32 pe_offset = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, offset_to_ptr(file, 0x3c, 4), 0));
    g_return_val_if_fail(memcmp(offset_to_ptr(file, pe_offset, 4), "PE\0", 4) == 0, NULL);

    // get size of optional header
    guint16 opt_header_size = GUINT16_FROM_LE(*(guint16 *)offset_to_ptr(file, pe_offset + 20, 2));

    return offset_to_ptr(file, pe_offset + 24, opt_header_size);
}

/**
 * returns whether the pe file the optional header belongs to is a pe32+ file
 */
static inline gboolean is_pe32_plus(char *optional_header)
{
    if (GUINT16_FROM_LE(*(guint16 *)optional_header) == 0x20b) {
        return TRUE;
    } else {
        g_assert(GUINT16_FROM_LE(*(guint16 *)optional_header) == 0x10b);
        return FALSE;
    }
}

/**
 * size of a section table entry, in bytes
 */
#define PE_SECTION_TABLE_ENTRY_SIZE 40


/**
 * returns a pointer to the section table
 */
static inline char *get_pe_section_table(GMappedFile *file, int *number_of_sections)
{
    // the section table begins after the optional header
    // the pe header contains the size of the optional header
    guint32 pe_offset = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, offset_to_ptr(file, 0x3c, 4), 0));
    g_return_val_if_fail(memcmp(offset_to_ptr(file, pe_offset, 4), "PE\0", 4) == 0, NULL);

    guint16 opt_header_size = GUINT16_FROM_LE(*(guint16 *)offset_to_ptr(file, pe_offset + 20, 2));
    guint32 opt_header_offset = pe_offset + 24;
    guint16 num_sections = GUINT16_FROM_LE(*(guint16 *)offset_to_ptr(file, pe_offset + 6, 2));
    if (number_of_sections != NULL) {
        *number_of_sections = num_sections;
    }

    return offset_to_ptr(file, opt_header_offset + opt_header_size, num_sections * PE_SECTION_TABLE_ENTRY_SIZE);
}

/**
 * returns a pointer to the section header of the given section, or NULL if the section wasn't found
 */
static inline char *get_pe_section_header(GMappedFile *file, char *section_name)
{
    int num_sections;
    char *section_table = get_pe_section_table(file, &num_sections);
    char *entry = NULL;
    for (int i = 0; i < num_sections; ++i) {
        char *section = &section_table[i * PE_SECTION_TABLE_ENTRY_SIZE];
        if (strcmp(section_name, section) == 0) {
            entry = section;
        }
    }
    return entry;
}

/**
 * returns a pointer to the resource directory
 */
static inline char *get_pe_resource_directory(GMappedFile *file)
{
    // the rva of the resource directory is saved in the optional header, but
    // unfortunately, the file offset and size may differ.
    // therefore, we examine the section table to find the .rsrc section
    // which points to the same data directory (FIXME: is this specified?)
    char *rsrc_entry = get_pe_section_header(file, ".rsrc");
    g_return_val_if_fail(rsrc_entry != NULL, NULL);

    guint32 rsrc_offset = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, rsrc_entry, 20));
    guint32 rsrc_size = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, rsrc_entry, 16));

    return offset_to_ptr(file, rsrc_offset, rsrc_size);
}

/**
 * converts a rva to a file pointer by examining the resource table
 */
static inline guint32 pe_rva_to_offset(GMappedFile *file, guint32 rva)
{
    // we find the section the rva points to, then we subtract it to get the section offset
    // next, we add the section offset to the file offset of the section.
    int n_sections;
    char *section_table = get_pe_section_table(file, &n_sections);

    for (int i = 0; i < n_sections; ++i) {
        char *entry = &section_table[n_sections * PE_SECTION_TABLE_ENTRY_SIZE];

        guint32 entry_rva = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, entry, 12));
        guint32 entry_vsize = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, entry, 8));
        if (rva >= entry_rva && rva < (entry_rva + entry_vsize)) {
            guint32 offset_in_section = rva - entry_rva;
            guint32 section_file_offset = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, entry, 20));

            return section_file_offset + offset_in_section;
        }
    }

    g_error("Invalid rva 0x%x supplied", rva);
    g_assert_not_reached();
}

/**
 * resource type for icon groups
 */
#define RT_GROUP_ICON 14

/**
 * resource type for single icons
 */
#define RT_ICON 3

/**
 * length of the resource directory structure
 */
#define PE_RESOURCE_DIRECTORY_SIZE 16

/**
 * number of name entries following the directory structure
 */
static inline guint16 pe_resource_directory_num_name_entries(char *resource_dir)
{
    guint16 val = G_STRUCT_MEMBER(guint16, resource_dir, 12);
    return GUINT16_FROM_LE(val);
}

/**
 * number of id entries following the name entries
 */
static inline guint16 pe_resource_directory_num_id_entries(char *resource_dir)
{
    guint16 val = G_STRUCT_MEMBER(guint16, resource_dir, 14);
    return GUINT16_FROM_LE(val);
}
/**
 * the size of an resource dir entry
 */
#define PE_RESOURCE_ENTRY_SIZE 8

/**
 * the id of the entry (or a rva to the name, depending on the entry)
 */
static inline gint32 pe_resource_entry_id(char *resource_entry_p)
{
    gint32 val = G_STRUCT_MEMBER(gint32, resource_entry_p, 0);
    return GINT32_FROM_LE(val);
}

/**
 * checks whether the data in the resource entry is a pointer to the next resource table or a pointer to the data
 */
static inline gboolean pe_resource_entry_has_data(char *resource_entry_p)
{
    guint32 val = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, resource_entry_p, 4));
    if ((val & 0x80000000) == 0x80000000) {
        // highest bit is 1 -> pointer to the next resource table
        return FALSE;
    } else {
        // highest bit is 0 -> rva of the data
        return TRUE;
    }
}

/**
 * returns a pointer relative to the resource section leading to the resource entry data
 */
static inline guint32 pe_resource_entry_get_data_rva(char *resource_entry_p)
{
    return GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, resource_entry_p, 4));
}

/**
 * returns the pointer to the next resource table, relative to the start of the resource section
 */
static inline guint32 pe_resource_entry_get_subtable_rva(char *resource_entry_p)
{
    return GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, resource_entry_p, 4)) & 0x7FFFFFFF; //clear highest bit
}

/**
 * returns a rva to the actual resource data
 *
 * WARNING: this is actually not a rva like the other ones, but an absolute virtual address.
 * you can get the rva (and with that the file pointer) by subtracting the rva of the .rsrc section
 */
static inline guint32 pe_resource_data_rva(char *resource_entry_data_p)
{
    guint32 val = G_STRUCT_MEMBER(guint32, resource_entry_data_p, 0);
    return GUINT32_FROM_LE(val);
}

/**
 * returns the length of the actual data associated with the resource entry
 */
static inline guint32 pe_resource_data_size(char *resource_entry_data_p)
{
    guint32 val = G_STRUCT_MEMBER(guint32, resource_entry_data_p, 4);
    return GUINT32_FROM_LE(val);
}

/**
 * size of a group icon dir structure, not including the following entries
 */
#define GROUP_ICON_DIR_SIZE 6

/**
 * checks the magic first 4 bytes for the group icon directory
 */
static inline gboolean group_icon_dir_check_magic(char *group_icon_dir_p)
{
    return memcmp(group_icon_dir_p, "\0\0\x01", 4) == 0;
}

/**
 * returns the number of entries following the group icon directory
 */
static inline guint16 group_icon_dir_count(char *group_icon_dir_p)
{
    guint16 val = G_STRUCT_MEMBER(guint16, group_icon_dir_p, 4);
    return GUINT16_FROM_LE(val);
}

/*
 * the size of a group icon directory entry structure
 */
#define GROUP_ICON_DIR_ENTRY_SIZE 14

/**
 * returns the width as specified in the icon dir entry
 */
static inline guint8 group_icon_dir_entry_width(char *entry_p)
{
    return G_STRUCT_MEMBER(guint8, entry_p, 0);
}

/**
 * returns the height as specified in the entry
 */
static inline guint8 group_icon_dir_entry_height(char *entry_p)
{
    return G_STRUCT_MEMBER(guint8, entry_p, 1);
}

/**
 * returns the bpp as specified in the entry
 */
static inline guint16 group_icon_dir_entry_bpp(char *entry_p)
{
    guint16 val = G_STRUCT_MEMBER(guint16, entry_p, 6);
    return GUINT16_FROM_LE(val);
}

/**
 * returns the resource id of the RT_ICON resource that carries the icon
 */
static inline guint16 group_icon_dir_entry_resid(char *entry_p)
{
    guint16 val = G_STRUCT_MEMBER(guint16, entry_p, 12);
    return GUINT16_FROM_LE(val);
}

/**
 * the size of the actual image data in bytes
 */
static inline guint32 group_icon_dir_entry_data_size(char *entry_p)
{
    guint32 val = G_STRUCT_MEMBER(guint32, entry_p, 8);
    return GUINT32_FROM_LE(val);
}

/**
 * returns an entry fromt the group icon directory
 */
static inline char *group_icon_dir_get_entry(char *dir_p, int entry_no)
{
    return G_STRUCT_MEMBER_P(dir_p, GROUP_ICON_DIR_SIZE + entry_no * GROUP_ICON_DIR_ENTRY_SIZE);
}

#endif //PE_H