#include <packr/types.h>
#include <packr/utils.h>
#include <packr/entry.h>
#include <packr/ops.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <unistd.h>
#include <malloc.h>
#include <stddef.h>

static void print_help();

int main(int argc, char** argv) {
    char cur_opt;
    char* src_path = NULL;
    char* named_as = NULL;
    bool op_provided = FALSE; // Whether a -p or -u option was provided
    bool no_metadata = FALSE;
    bool path_provided = FALSE;
    bool path_absolute = FALSE;

    while((cur_opt = getopt(argc, argv, "pushl:a:")) != -1) {
        switch(cur_opt) {
        case 'p':
            op_provided = TRUE;
            break;

        case 'u':
            if(op_provided == TRUE) {
                fprintf(stderr, "Can't pack and unpack at the same time!\n");
                return 1;
            }
            op_provided = TRUE;
            break;

        case 's':
            no_metadata = TRUE;
            break;

        case 'l':
            path_provided = TRUE;
            src_path = optarg;
            break;


        case 'h':
            print_help();
            return 0;
            break;

        case 'a':
            named_as = optarg;
            break;

        case '?':
            print_help();
            return 1;
        }
    }


    if(!op_provided || !path_provided) {
        print_help();
        return 1;
    }




    DIR* dir = NULL; // pointer to target directory stream

    if(*src_path == '/') {
        path_absolute = TRUE;
    } else {
        char* cwd = getcwd(NULL, 0);
        src_path = join_to_path(src_path, cwd);
        free(cwd);
        printf("target: %s\n", src_path);
    }

    dir = opendir(src_path);
    if(dir == NULL) {
        perror("Can't access directory");
        if(!path_absolute) free(src_path);
        return 1;
    } else {
        printf("Source directory found!\n");
    }


    pack_header* dir_data = get_dir_data(dir, src_path, DEFAULT_ROOT_DIR);
    if(named_as) {
        memcpy(dir_data->dirname, named_as, strlen(named_as) + 1); // +1 for the \0
        dir_data->dirname_length = strlen(named_as);
    } else {
        i16 slash_last_instance = 0;
        for(size_t i = 0; i < strlen(src_path); i++) {
            if(src_path[i] == '/') {
                slash_last_instance = i;
            }
        }
        char* target_name = src_path + slash_last_instance + 1;          // +1 to skip the last '/'
        memcpy(dir_data->dirname, target_name, strlen(target_name) + 1); // +1 to include the \0
        dir_data->dirname_length = strlen(dir_data->dirname);
    }
    printf("dir size is: %lu\n", dir_data->size);
    printf("dir name: %s\n", dir_data->dirname);
    printf("dir name length: %u\n", dir_data->dirname_length);

    printf("total_dir_count: %lu\n", dir_data->total_dir_count);
    printf("total_file_count: %lu\n", dir_data->total_file_count);
    printf("total_entry_count: %lu\n", dir_data->total_entry_count);

    printf("child_dir_count: %lu\n", dir_data->child_dir_count);
    printf("child_file_count: %lu\n", dir_data->child_file_count);
    printf("child_entry_count: %lu\n", dir_data->child_entry_count);

    printf("last access time: %lu\n", dir_data->acc_time);
    printf("last modification time: %lu\n", dir_data->mod_time);
    printf("last status change time: %lu\n", dir_data->sc_time);
    printf("mode: %d\n", dir_data->mode);

    const char* extension = ".packr";
    char* pack_file_str = malloc(strlen(extension) + strlen(dir_data->dirname) + 1); // +1 for the \0
    if(!pack_file_str) {
        free(pack_file_str);
        perror("malloc()");
        return 1;
    }
    strcpy(pack_file_str, dir_data->dirname);
    strcat(pack_file_str, extension);

    FILE* pack_file_stream = fopen(pack_file_str, "w+");
    if(!pack_file_stream) {
        if(!path_absolute) {
            free(src_path);
        }
        closedir(dir);
        free(dir_data);
        perror("fdopen()");
        free(pack_file_str);
        return 1;
    }


    if(pack(dir_data, src_path, dir, pack_file_stream, DEFAULT_ROOT_DIR) != 0) {
        perror("pack()");
        if(!path_absolute) {
            free(src_path);
        }
        free(pack_file_str);
        closedir(dir);
        free(dir_data);
        fclose(pack_file_stream);
        return 1;
    }

    // Cleanup
    if(!path_absolute) {
        free(src_path);
    }
    closedir(dir);
    free(dir_data);
    free(pack_file_str);
    fclose(pack_file_stream);

    return 0;
}

static void print_help() {
    printf("Usage:\n -p: pack a directory\n -u: unpack a .packr file\n -s: neglect metadata\n -l: path to directory\n "
           "-h: help\n");
}
