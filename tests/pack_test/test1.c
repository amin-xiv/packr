#include <dirent.h>
#include <packr/types.h>
#include <packr/entry.h>
#include <stdio.h>
#include <fcntl.h>


int main(void) {
    const char* pack_path = "/mnt/data/Data/Systems_Programming/practice_projects/packr/build/directorname.packr";
    FILE* file_stream = fopen(pack_path, "rb");
    if(!file_stream) {
        perror("fopen()");
        return 1;
    }

    special_marker pack_start_marker;
    if(fread(&pack_start_marker, sizeof(special_marker), 1, file_stream) < 1) {
        fprintf(stderr, "error while pack start marker!\n");
        return 1;
    }

    dir_entry curr_dir_data;
    if(fread(&curr_dir_data, sizeof(dir_entry), 1, file_stream) < 1) {
        fprintf(stderr, "error while reading pack header\n");
    }
    printf("==========================\n");
    printf("==========================\n");
    printf("dirname: %s\n", curr_dir_data.dirname);
    printf("dirname length: %d\n", curr_dir_data.dirname_length);
    printf("size: %lu\n", curr_dir_data.size);
    printf("child entry count: %lu\n", curr_dir_data.child_entry_count);
    printf("child file count: %lu\n", curr_dir_data.child_file_count);
    printf("child dir count : %lu\n", curr_dir_data.child_dir_count);
    printf("total entry count: %lu\n", curr_dir_data.total_entry_count);
    printf("total file count: %lu\n", curr_dir_data.total_file_count);
    printf("total dir count: %lu\n", curr_dir_data.total_dir_count);
    printf("last access time: %lu\n", curr_dir_data.acc_time);
    printf("last modification time: %lu\n", curr_dir_data.mod_time);
    printf("last status change time: %lu\n", curr_dir_data.sc_time);
    printf("mode: %d\n", curr_dir_data.mode);
    printf("==========================\n");
    printf("==========================\n");

    u32 run_count = 0;
    while(TRUE) {
        special_marker curr_marker;
        if(fread(&curr_marker, sizeof(special_marker), 1, file_stream) < 1) {
            fprintf(stderr, "error while reading marker!\n");
            return 1;
        }
        printf("current marker: %d\n", curr_marker.type);

        switch(curr_marker.type) {
        case PACK_END:
            printf("PACK_END..\n");
            return 1;
            break;
        case ENT_DIR_START:
            if(!run_count) {
                continue;
            }
            printf("==========================\n");
            printf("ENT_DIR_START..\n");
            {
                dir_entry curr_dir_data;
                if(fread(&curr_dir_data, sizeof(dir_entry), 1, file_stream) < 1) {
                    fprintf(stderr, "error while reading a directory entry from file stream\n");
                }
                printf("dirname: %s\n", curr_dir_data.dirname);
                printf("dirname length: %d\n", curr_dir_data.dirname_length);
                printf("size: %lu\n", curr_dir_data.size);
                printf("child entry count: %lu\n", curr_dir_data.child_entry_count);
                printf("child file count: %lu\n", curr_dir_data.child_file_count);
                printf("child dir count : %lu\n", curr_dir_data.child_dir_count);
                printf("total entry count: %lu\n", curr_dir_data.total_entry_count);
                printf("total file count: %lu\n", curr_dir_data.total_file_count);
                printf("total dir count: %lu\n", curr_dir_data.total_dir_count);
                printf("last access time: %lu\n", curr_dir_data.acc_time);
                printf("last modification time: %lu\n", curr_dir_data.mod_time);
                printf("last status change time: %lu\n", curr_dir_data.sc_time);
                printf("mode: %d\n", curr_dir_data.mode);
            }
            break;
        case ENT_DIR_END:
            printf("ENT_DIR_END..\n");
            printf("==========================\n");
            break;
        case ENT_FILE:
            printf("==========================\n");
            printf("ENT_FILE..\n");
            {
                file_entry curr_file_data;
                if(fread(&curr_file_data, sizeof(file_entry), 1, file_stream) < 1) {
                    fprintf(stderr, "Error while reading file\n");
                    return 1;
                }
                printf("file name: %s\n", curr_file_data.filename);
                printf("file name length: %d\n", curr_file_data.filename_length);
                printf("size: %lu\n", curr_file_data.size);
                printf("last access time: %lu\n", curr_file_data.acc_time);
                printf("last modification time: %lu\n", curr_file_data.mod_time);
                printf("last status change time: %lu\n", curr_file_data.sc_time);
                printf("contents: \n");
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
                char read_buff[curr_file_data.size];
                if(fread(&read_buff, 1, curr_file_data.size, file_stream) != curr_file_data.size) {
                    fprintf(stderr, "Error while reading file contents..\n");
                    return 1;
                }
                printf("%.*s\n", (int)curr_file_data.size, read_buff);
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            }
            break;
        default:
            fprintf(stderr, "Fatal: Unknown marker!\n");
            return 1;
        }
        run_count++;
    }
    return 0;
}
