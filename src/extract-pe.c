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
#include "extract-pe.h"
#include "config.h"

#include "pe.h"
#include "write-icons.h"
#include "extract-common.h"

#include <glib/gi18n-lib.h>

gboolean is_pe_file(GMappedFile *file)
{
    char *dos_header = offset_to_ptr(file, 0, 2);
    if (memcmp(dos_header, "MZ", 2) != 0) {
        g_debug("no msdos header found");
        return FALSE;
    }

    guint32 pe_offset = GUINT32_FROM_LE(*(guint32 *)offset_to_ptr(file, 0x3c, 4));
    char *pe_signature = offset_to_ptr(file, pe_offset, 4);
    if (memcmp(pe_signature, "PE\0", 4) != 0) {
        g_debug("no pe signature found at offset %d", pe_offset);
        return FALSE;
    }

    return TRUE;
}

/**
 * find the 2nd level resource dir using the type id
 */
static char *get_l2_resource_dir_by_type(char *resource_section, gint32 type)
{
    // search the toplevel resource directory for the type
    int tl_num_name_entries = pe_resource_directory_num_name_entries(resource_section);
    int tl_num_id_entries = pe_resource_directory_num_id_entries(resource_section);
    g_return_val_if_fail(tl_num_id_entries > 0, NULL); //really, that shouldn't be happening

    // walk through all entries to find the one with the right type
    char *tl_entry = NULL;
    for (int i = 0; i < tl_num_id_entries; ++i) {
        char *entry = resource_section + PE_RESOURCE_DIRECTORY_SIZE +
        tl_num_name_entries * PE_RESOURCE_ENTRY_SIZE +
        i * PE_RESOURCE_ENTRY_SIZE;
        if (pe_resource_entry_id(entry) == type) {
            tl_entry = entry;
            break;
        }
    }

    if (tl_entry == NULL) return NULL;
    g_assert(!pe_resource_entry_has_data(tl_entry));

    return resource_section + pe_resource_entry_get_subtable_rva(tl_entry);
}

/**
 * return the data for the first entry found in the resource diretory
 */
static char *get_data_of_first_resource(GMappedFile *file, char *resource_section, guint32 resource_dir_rva)
{
    char *l3_directory = resource_section + resource_dir_rva;
    int l3_num_entries = pe_resource_directory_num_name_entries(l3_directory) + pe_resource_directory_num_id_entries(l3_directory);
    if (l3_num_entries < 1) return NULL; //FIXME: should we assert? it definitely shouldn't happen

    char *l3_entry = l3_directory + PE_RESOURCE_DIRECTORY_SIZE;
    g_assert(pe_resource_entry_has_data(l3_entry));

    // get the resource data
    char *resource_data = resource_section + pe_resource_entry_get_data_rva(l3_entry);
    g_debug("resource data struct at offst 0x%x", ptr_to_offset(file, resource_data));
    guint32 actual_data_rva = pe_resource_data_rva(resource_data);
    guint32 actual_data_length = pe_resource_data_size(resource_data);

    // HACK: the rva of the actual data is located on the disk at the same offset as the .rsrc data,
    // but it might not appear in the section table, so we need to do our own pointer magic
    char *rsrc_section_header = get_pe_section_header(file, ".rsrc");
    guint32 rsrc_rva = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, rsrc_section_header, 12));
    guint32 rsrc_offset = GUINT32_FROM_LE(G_STRUCT_MEMBER(guint32, rsrc_section_header, 20));

    guint32 actual_data_offset = actual_data_rva - rsrc_rva + rsrc_offset;

    g_debug("actual data at offset 0x%x", actual_data_offset);
    return offset_to_ptr(file, actual_data_offset, actual_data_length);
}

/**
 * find a resource in the pe file @a pe_file described by the resource type @a type
 * and return the @a no th one. Will return NULL if the resource could not be found.
 *
 * FIXME: doesn't allow type strings
 */
static char *find_resource_by_type(GMappedFile *pe_file, gint32 type, int no)
{
    char *resource_section = get_pe_resource_directory(pe_file);
    g_debug("resource directory at offset 0x%x", ptr_to_offset(pe_file, resource_section));

    // get the 2nd level resource directory
    char *l2_directory = get_l2_resource_dir_by_type(resource_section, type);
    if (l2_directory == NULL) {
        g_debug("no resource of type %d could be found", type);
        return NULL;
    }
    int l2_num_entries = pe_resource_directory_num_name_entries(l2_directory) + pe_resource_directory_num_id_entries(l2_directory);

    // check whether no is in bounds
    if (no < 0 || no >= l2_num_entries) return NULL;

    // get right entry
    char *l2_entry = l2_directory + PE_RESOURCE_DIRECTORY_SIZE + no * PE_RESOURCE_ENTRY_SIZE;
    g_assert(!pe_resource_entry_has_data(l2_entry));

    // third level is language, as I've yet to see a multi language icon we just pick the first one
    return get_data_of_first_resource(pe_file, resource_section, pe_resource_entry_get_subtable_rva(l2_entry));
}

/**
 * find a resource in the pe file @a pe_file described by the resource type @a type
 * and return the one with the id @a id. Will return NULL if the resource could not be found.
 *
 * FIXME: doesn't allow type or name strings
 * FIXME: code duplication with above function
 */
static char *find_resource_by_type_and_id(GMappedFile *pe_file, guint32 type, guint32 id)
{
    char *resource_section = get_pe_resource_directory(pe_file);

    // now get the next directory and take right one
    char *l2_directory = get_l2_resource_dir_by_type(resource_section, type);
    if (l2_directory == NULL) {
        g_debug("no resource of type %d could be found", type);
        return NULL;
    }
    int l2_num_name_entries = pe_resource_directory_num_name_entries(l2_directory);
    int l2_num_id_entries = pe_resource_directory_num_id_entries(l2_directory);

    char *l2_entry = NULL;
    for (int i = 0; i < l2_num_id_entries; ++i) {
        char *entry = l2_directory + PE_RESOURCE_DIRECTORY_SIZE +
                      l2_num_name_entries * PE_RESOURCE_ENTRY_SIZE +
                      i * PE_RESOURCE_ENTRY_SIZE;
        if (pe_resource_entry_id(entry) == id) {
            l2_entry = entry;
            break;
        }
    }
    if (l2_entry == NULL) {
        g_debug("data with id %d not found in resource directory at rva 0x%x", id, ptr_to_offset(pe_file, l2_directory));
        return NULL;
    }
    g_assert(!pe_resource_entry_has_data(l2_entry));

    // third level is language, as I've yet to see a multi language icon so we just pick the first one
    return get_data_of_first_resource(pe_file, resource_section, pe_resource_entry_get_subtable_rva(l2_entry));
}

char* get_grp_icon_dir_from_pe(GMappedFile* pe_file, int i)
{
    return find_resource_by_type(pe_file, RT_GROUP_ICON, i);
}

GHashTable* parse_group_icon_dir(GMappedFile* file, char* group_icon_dir, gint max_bpp)
{
    GHashTable *table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)icon_info_free);
    for (int i = 0; i < group_icon_dir_count(group_icon_dir); ++i) {
        g_assert(is_ptr_valid(file, group_icon_dir_get_entry(group_icon_dir, i), GROUP_ICON_DIR_ENTRY_SIZE));
        char *entry = group_icon_dir_get_entry(group_icon_dir, i);

        // width and height might be defined in the bitmap header instead
        gint width = group_icon_dir_entry_width(entry);
        gint height = group_icon_dir_entry_height(entry);
        gint bpp = group_icon_dir_entry_bpp(entry);

        guint16 resid = group_icon_dir_entry_resid(entry);

        g_debug("finding data with resource id %d", resid);
        char *data = find_resource_by_type_and_id(file, RT_ICON, resid);
        g_assert(data != NULL);

        if (width == 0 || height == 0 || bpp == 0) {
            // width and height are always missing as soon as they are >= 256, bpp is sometimes missing too
            //FIXME: why the bpp?
            if (!peek_width_height(file, data, &width, &height, &bpp)) {
                g_warning("No valid width and height found for IconDirEntry no. %d", i);
                continue;
            }
        }

        if (bpp > max_bpp) continue;

        gchar *key = g_strdup_printf("%dx%d", width, height);
        if (!g_hash_table_contains(table, key) ||
            ((IconInfo*)g_hash_table_lookup(table, key))->bpp < bpp) {
            IconInfo *info = g_malloc(sizeof(IconInfo));
                info->width = width;
                info->height = height;
                info->bpp = bpp;
                info->image_data = data;
                info->image_size = group_icon_dir_entry_data_size(entry);
                info->image_data_free = NULL;
                g_hash_table_replace(table, key, info);
                g_debug("placed into hashtable: variant #%d at %dx%dx%d", i, width, height, bpp);
            } else {
                g_free(key);
                g_debug("not placing into hashtable: variant #%d at %dx%dx%d", i, width, height, bpp);
            }
    }
    return table;
}


