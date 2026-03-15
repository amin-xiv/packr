#include <packr/types.h>
#include <packr/utils.h>
#include <packr/entry.h>
#include <packr/ops.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

struct dir_data* get_dir_data(DIR* dir, char* dir_str, u32 nest_count) {
    struct dir_data* data_ptr = malloc(sizeof(struct dir_data));
    struct dir_data data_temp = {
        .child_entry_count = 0,
        .child_file_count = 0,
        .child_dir_count = 0,
        .total_entry_count = 0,
        .total_dir_count = 0,
        .total_file_count = 0,
        .acc_time = 0,
        .mod_time = 0,
        .sc_time = 0,
        .dirname_length = 0,
        .mode = 0,
    };
    data_ptr->child_entry_count = 0;
    data_ptr->child_file_count = 0;
    data_ptr->child_dir_count = 0;

    data_ptr->total_file_count = 0;
    data_ptr->total_dir_count = 0;
    data_ptr->total_entry_count = 0;

    data_ptr->total_size = 0;

    data_ptr->dirname_length = strlen(dir_str);

    struct dirent* entry;
    struct stat ent_stat;

    while((entry = readdir(dir)) != NULL) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            data_ptr->total_dir_count++;
            data_ptr->total_entry_count++;
            continue;
        }


        char* full_path = join_to_path(entry->d_name, dir_str);

        printf("current entry: %s\n", full_path);

        if(lstat(full_path, &ent_stat) == -1) {
            free(full_path);
            return NULL;
        }

        if(S_ISDIR(ent_stat.st_mode)) {
            DIR* dir_inner = opendir(full_path);

            if(!dir_inner) {
                return NULL;
            }

            struct dir_data* data_inner = get_dir_data(dir_inner, full_path, nest_count + 1);
            data_ptr->total_size += data_inner->total_size;
            data_ptr->total_entry_count++;
            data_ptr->total_dir_count++;

            // Add data_inner's total counts just in case there was nested directories
            data_ptr->total_dir_count += data_inner->total_dir_count;
            data_ptr->total_entry_count += data_inner->total_entry_count;
            data_ptr->total_file_count += data_inner->total_file_count;

            // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so we can increment child counts
            if(!nest_count) {
                data_ptr->child_entry_count++;
                data_ptr->child_dir_count++;
            }

            free(dir_inner);
            free(data_inner);
        } else {
            data_ptr->total_size += ent_stat.st_size;
            data_ptr->total_entry_count++;
            data_ptr->total_file_count++;

            // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so we can increment child counts
            if(!nest_count) {
                data_ptr->child_entry_count++;
                data_ptr->child_file_count++;
            }
        }

        free(full_path);
    }

    return data_ptr;
}
