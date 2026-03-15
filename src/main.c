#include <stdio.h>
#include <getopt.h>
#include <packr/types.h>
#include <packr/ops.h>
#include <packr/utils.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

static void print_help();

int main(int argc, char** argv) {
    char cur_opt;
    char* src_path;
    bool op_provided = FALSE; // Whether a -p or -u option was provided
    bool no_metadata = FALSE;
    bool path_provided = FALSE;
    bool path_absolute = FALSE;

    while((cur_opt = getopt(argc, argv, "pushl:")) != -1) {
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
        }
    }


    if(!op_provided || !path_provided) {
        print_help();
        return 1;
    }


    pack_header* header; // Pack header
    DIR* dir = NULL;     // pointer to target directory stream

    if(*src_path == '/') {
        path_absolute = TRUE;
        dir = opendir(src_path);
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
        printf("Directory found!\n");
    }


    u64 dir_size = get_dir_size(dir, src_path);
    printf("dir size is: %lu\n", dir_size);


    // Cleanup
    if(!path_absolute) {
        free(src_path);
    }
    closedir(dir);

    return 0;
}

static void print_help() {
    printf("Usage:\n -p: pack a directory\n -u: unpack a .packr file\n -s: neglect metadata\n -l: path to directory\n "
           "-h: help\n");
}
