#pragma once
#include <dirent.h>
#include <packr/types.h>
#include <packr/utils.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

u64 get_dir_size(DIR* dir, char* dir_str) {
    u64 accumulator = 0;

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
            return -1;
        }

        if(S_ISDIR(ent_stat.st_mode)) {
            DIR* dir_inner = opendir(full_path);

            if(!dir_inner) {
                return 1;
            }

            u64 inner_accumulator = get_dir_size(dir_inner, full_path);
            accumulator += inner_accumulator;

            free(dir_inner);
        } else {
            accumulator += ent_stat.st_size;
        }

        free(full_path);
    }

    return accumulator;
}
