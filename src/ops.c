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

pack_header* get_dir_data(DIR* dir, char* dir_str, u32 nest_count) {
    pack_header* data_ptr = pack_header_init();
    if(!data_ptr) {
        return NULL;
    }

    data_ptr->child_entry_count = 0;
    data_ptr->child_file_count = 0;
    data_ptr->child_dir_count = 0;

    data_ptr->total_file_count = 0;
    data_ptr->total_dir_count = 0;
    data_ptr->total_entry_count = 0;

    data_ptr->total_size = 0;

    struct dirent* entry;
    struct stat ent_stat;

    while((entry = readdir(dir)) != NULL) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
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

            pack_header* data_inner = get_dir_data(dir_inner, full_path, nest_count + 1);

            if(!data_inner) {
                return NULL;
            }

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

            closedir(dir_inner);
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

    // get timestamps and mode
    if(!nest_count) {
        struct stat root_stat;
        if(lstat(dir_str, &root_stat) == -1) {
            return NULL;
        }

        data_ptr->acc_time = root_stat.st_atim.tv_sec + NSEC_TO_SEC(root_stat.st_atim.tv_nsec);
        data_ptr->mod_time = root_stat.st_mtim.tv_sec + NSEC_TO_SEC(root_stat.st_mtim.tv_nsec);
        data_ptr->sc_time = root_stat.st_ctim.tv_sec + NSEC_TO_SEC(root_stat.st_ctim.tv_nsec);
        data_ptr->mode = root_stat.st_mode;
    }

    return data_ptr;
}
