#define _LARGEFILE64_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
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

    // As filename is the absolute path, actual_filename is the name of the file only
    char* actual_filename = extract_filename(filename);
    strcpy(data->filename, actual_filename);
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

    // write the dir header upfront only if it's the intial pack header(nest_count
    // = 0)
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

            // dir_entry type because pack_header is just a typedef of 'struct
            // dir_entry'
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

        } else if(S_ISLNK(ent_stat.st_mode)) {
            // i don't wanna handle any symlinks rn
            free(full_path);
            continue;
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

            // check if file has actually some data and size != 0 before writing file
            // contents
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
            sync(); // to ensure data is actually residing on the file before next
                    // iteration

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

i8 unpack_dir(FILE* pack_file, u8 opts, u32 nest_count) {
    // 'opts' flag is for future use(maybe lol)

    // Flag to keep looping
    Bool read_pack_file = TRUE;

    while(read_pack_file) {
        special_marker curr_marker;
        if(fread(&curr_marker, sizeof(special_marker), 1, pack_file) < 1) return 1;

        switch(curr_marker.type) {
        case PACK_START:
            // this shouldn't be here, it should be already read before this function is invoked
            return 1;
            break;

        case PACK_END:
            read_pack_file = FALSE; // Terminate the while-loop
            break;

        case ENT_FILE:
            {
                /* On its own block to prevent pollution the case() namespace */
                file_entry curr_file_data;
                if(fread(&curr_file_data, sizeof(file_entry), 1, pack_file) < 1) return 1;

                const char* unnamed_filename = "unamed-file"; // Just in case the file had no name for some reason
                if(curr_file_data.filename_length < 1) {
                    // Copying it into curr_file_data.filename so a flag isn't needed
                    memcpy(curr_file_data.filename, unnamed_filename, strlen(unnamed_filename));
                }

                FILE* target_file = fopen(curr_file_data.filename, "w");
                if(!target_file) return 1;

                char* file_data_buff = malloc(curr_file_data.size);
                if(!file_data_buff) return 1;

                if(fread(file_data_buff, curr_file_data.size, 1, pack_file) < 1) return 1;

                if(fwrite(file_data_buff, curr_file_data.size, 1, target_file) < 1) return 1;

                fclose(target_file);
                free(file_data_buff);
            }
            break;

        case ENT_DIR_START:
            {
                /* On its own block to prevent pollution the case() namespace */
                dir_entry curr_dir_data;
                if(fread(&curr_dir_data, sizeof(dir_entry), 1, pack_file) < 1) return 1;

                // Verify entry is actually a directory
                if(!S_ISDIR(curr_dir_data.mode)) return 1;

                const char* unnamed_dirname = "unamed-directory"; // Just in case the dir had no name for some reason
                if(curr_dir_data.dirname_length < 1) {
                    // Copying it into curr_dir_data.dirname so a flag isn't needed
                    memcpy(curr_dir_data.dirname, unnamed_dirname, strlen(unnamed_dirname));
                }

                if(mkdir(curr_dir_data.dirname, curr_dir_data.mode) == -1) return 1;
                char* cwd = getcwd(NULL, 0);
                if(!cwd) return 1;
                // The path of the newely created directory
                char* target_dir_path = join_to_path(curr_dir_data.dirname, cwd);
                if(chdir(target_dir_path) == -1) return 1;

                if(unpack_dir(pack_file, opts, nest_count + 1) != 0) return 1;

                // Return to curr after unpacking the sub dir
                if(chdir(cwd) == -1) return 1;

                free(target_dir_path);
                free(cwd);
            }
            break;

        case ENT_DIR_END:
            read_pack_file = FALSE;
            break;
        default:
            return 1;
            break;
        }
    }

    return 0;
}

i8 unpack(FILE* pack_file, u8 opts, u32 nest_count) {
    // Reading PACK_START
    special_marker pack_start_marker;
    if(fread(&pack_start_marker, sizeof(special_marker), 1, pack_file) < 1) return 1;
    if(pack_start_marker.type != PACK_START) return 1;

    // Reading pack_header
    dir_entry pack_header;
    if(fread(&pack_header, sizeof(dir_entry), 1, pack_file) < 1) return 1;

    // This marks the start of the target root directory
    special_marker initial_dir_start_marker;
    if(fread(&initial_dir_start_marker, sizeof(special_marker), 1, pack_file) < 1) return 1;
    if(initial_dir_start_marker.type != ENT_DIR_START) return 1;

    // Verify whether data is corrupted or no
    if(!S_ISDIR(pack_header.mode)) return 1;

    // Making the root directory and changing into it
    if(mkdir(pack_header.dirname, pack_header.mode) == -1) {
        if(errno != EEXIST) return 1;
    }
    char* cwd = getcwd(NULL, 0);
    if(!cwd) return 1;
    char* root_dir_path = join_to_path(pack_header.dirname, cwd);
    if(chdir(root_dir_path) == -1) {
        free(cwd);
        return 1;
    }

    if(unpack_dir(pack_file, 0, DEFAULT_ROOT_DIR) != 0) {
        free(cwd);
        return 1;
    }

    free(cwd);
    free(root_dir_path);

    return 0;
}
