#define _LARGEFILE64_SOURCE
#include <malloc.h>
#include <packr/entry.h>
#include <packr/ops.h>
#include <packr/types.h>
#include <packr/utils.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

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
    data_ptr->entry_class = (nest_count - 1) == 0 ? CHILD_ENT : NESTED_ENT;
    data_ptr->size = 0;
    add_dirname(data_ptr, NULL, dir_str);

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
                free(full_path);
                return NULL;
            }

            pack_header* data_inner = get_dir_data(dir_inner, full_path, nest_count + 1);

            if(!data_inner) {
                free(full_path);
                closedir(dir_inner);
                return NULL;
            }

            data_ptr->size += data_inner->size;
            data_ptr->total_entry_count++;
            data_ptr->total_dir_count++;

            // Add data_inner's total counts just in case there was nested directories
            data_ptr->total_dir_count += data_inner->total_dir_count;
            data_ptr->total_entry_count += data_inner->total_entry_count;
            data_ptr->total_file_count += data_inner->total_file_count;

            // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so
            // we can increment child counts
            if(!nest_count) {
                data_ptr->child_entry_count++;
                data_ptr->child_dir_count++;
            }

            closedir(dir_inner);
            free(data_inner);
        } else {
            data_ptr->size += ent_stat.st_size;
            data_ptr->total_entry_count++;
            data_ptr->total_file_count++;

            // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so
            // we can increment child counts
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

file_entry* get_file_data(char* filename, u32 nest_count) {
    file_entry* data = malloc(sizeof(file_entry));
    if(!data) return NULL;

    memset(data, '\0', sizeof(file_entry));
    data->size = 0;
    data->acc_time = 0;
    data->mod_time = 0;
    data->sc_time = 0;
    data->filename_length = 0;
    data->mode = 0;
    data->entry_class = (nest_count - 1) == 0 ? CHILD_ENT : NESTED_ENT;

    struct stat file_stat;
    if(lstat(filename, &file_stat) == -1) {
        return NULL;
    }

    strcpy(data->filename, filename);
    data->size = file_stat.st_size;
    data->filename_length = strlen(filename); // +1 to count the \0
    data->acc_time = file_stat.st_atim.tv_sec + NSEC_TO_SEC(file_stat.st_atim.tv_nsec);
    data->mod_time = file_stat.st_mtim.tv_sec + NSEC_TO_SEC(file_stat.st_mtim.tv_nsec);
    data->sc_time = file_stat.st_ctim.tv_sec + NSEC_TO_SEC(file_stat.st_ctim.tv_nsec);
    data->mode = file_stat.st_mode;

    return data;
}

i8 pack_dir(dir_entry* dir_header, char* dir_path, FILE* pack_file, u8 opts, u32 nest_count) {
    Bool no_metadata = (opts & P_NOMETADATA) != 0;
    dir_entry dir_header_copy = *dir_header;

    DIR* dir = opendir(dir_path);
    if(!dir) {
        return 1;
    }

    // write the dir header upfront only if it's the intial pack header(nest_count = 0)
    if(!nest_count) {
        if(fwrite(dir_header, sizeof(dir_entry), 1, pack_file) < 1) {
            closedir(dir);
            return 1;
        }
    }

    special_marker dir_marker_start = {.type = ENT_DIR_START};
    if(fwrite(&dir_marker_start, sizeof(special_marker), 1, pack_file) < 1) {
        closedir(dir);
        return 1;
    }

    // i.e. if nest_count > 0
    if(nest_count) {
        if(fwrite(dir_header, sizeof(dir_entry), 1, pack_file) < 1) {
            closedir(dir);
            return 1;
        }
    }

    struct stat ent_stat;
    struct dirent* curr_ent;
    while((curr_ent = readdir(dir)) != NULL) {
        if(!strcmp(curr_ent->d_name, ".") || !strcmp(curr_ent->d_name, "..")) {
            continue;
        }

        char* full_path = join_to_path(curr_ent->d_name, dir_path);
        printf("current entry to pack: %s\n", full_path);

        if(lstat(full_path, &ent_stat) == -1) {
            free(full_path);
            closedir(dir);
            return 1;
        }

        if(S_ISDIR(ent_stat.st_mode)) {
            DIR* dir_inner = opendir(full_path);
            if(!dir_inner) {
                free(full_path);
                closedir(dir);
                return 1;
            }

            // dir_entry type because pack_header is just a typedef of 'struct dir_entry'
            dir_entry* dir_data_inner = get_dir_data(dir_inner, full_path, DEFAULT_ROOT_DIR);
            if(!dir_data_inner) {
                free(full_path);
                closedir(dir_inner);
                closedir(dir);
                return 1;
            }

            if(pack_dir(dir_data_inner, full_path, pack_file, opts, nest_count + 1) != 0) {
                free(full_path);
                closedir(dir_inner);
                closedir(dir);
                return 1;
            }

            dir_header_copy.total_dir_count--;
            dir_header_copy.total_entry_count--;

            if(!nest_count) {
                dir_header_copy.child_entry_count--;
                dir_header_copy.child_dir_count--;
            }

            free(full_path);
            closedir(dir_inner);
            free(dir_data_inner);

        } else {
            file_entry* file_data = get_file_data(full_path, nest_count + 1);
            if(!file_data) {
                free(full_path);
                closedir(dir);
                return 1;
            }

            special_marker file_marker = {.type = ENT_FILE};
            if(fwrite(&file_marker, sizeof(special_marker), 1, pack_file) < 1) {
                free(full_path);
                closedir(dir);
                free(file_data);
                return 1;
            }

            if(fwrite(file_data, sizeof(file_entry), 1, pack_file) < 1) {
                free(full_path);
                closedir(dir);
                free(file_data);
                return 1;
            }

            // check if file has actually some data and size != 0 before writing file contents
            if(file_data->size) {
                FILE* file_stream = fopen(full_path, "r");
                if(!file_stream) {
                    free(full_path);
                    closedir(dir);
                    free(file_data);
                    return 1;
                }

                // if the file has actual contents and not empty
                char* read_buff = malloc(file_data->size);
                memset(read_buff, '\0', file_data->size);
                if(read_buff)
                    if(fread(read_buff, file_data->size, 1, file_stream) < 1) {
                        free(full_path);
                        closedir(dir);
                        fclose(file_stream);
                        free(file_data);
                        return 1;
                    }

                if(fwrite(read_buff, file_data->size, 1, pack_file) < 1) {
                    free(full_path);
                    closedir(dir);
                    fclose(file_stream);
                    free(file_data);
                    free(read_buff);
                    return 1;
                }

                free(read_buff);
                fclose(file_stream);
            }

            free(file_data);
            free(full_path);
            sync(); // to ensure data is actually residing on the file before next iteration

            dir_header_copy.total_file_count--;
            dir_header_copy.total_entry_count--;

            if(!nest_count) {
                dir_header_copy.child_entry_count--;
                dir_header_copy.child_file_count--;
            }
        }
    }

    special_marker dir_marker_end = {.type = ENT_DIR_END};
    if(fwrite(&dir_marker_end, sizeof(special_marker), 1, pack_file) < 1) {
        closedir(dir);
        return 1;
    }

    sync(); // to ensure data is actually residing on the file
    closedir(dir);

    return 0;
}

i8 pack(pack_header* header, char* dir_path, DIR* dir, FILE* pack_file, u8 opts) {
    special_marker pack_start_marker = {.type = PACK_START};
    if(fwrite(&pack_start_marker, sizeof(special_marker), 1, pack_file) < 1) {
        return 1;
    }

    if(pack_dir(header, dir_path, pack_file, opts, DEFAULT_ROOT_DIR) != 0) {
        return 1;
    }

    special_marker pacK_end_marker = {.type = PACK_END};
    if(fwrite(&pacK_end_marker, sizeof(special_marker), 1, pack_file) < 1) {
        return 1;
    }

    return 0;
}
