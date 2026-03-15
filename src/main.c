#include <stdio.h>
#include <getopt.h>
#include <packr/types.h>
#include <packr/utils.h>
#include <packr/entry.h>
#include <packr/ops.h>
#include <dirent.h>
#include <unistd.h>
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


    pack_header* header = pack_header_init(); // Pack header
    if(!header) {
        printf("failed to intialize pack header\n");
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
        printf("Directory found!\n");
    }


    struct dir_data* data = get_dir_data(dir, src_path, DEFAULT_ROOT_DIR);
    header->total_size = data->total_size;
    printf("dir size is: %lu\n", data->total_size);

    printf("total_dir_count: %lu\n", data->total_dir_count);
    printf("total_file_count: %lu\n", data->total_file_count);
    printf("total_entry_count: %lu\n", data->total_entry_count);

    printf("child_dir_count: %lu\n", data->child_dir_count);
    printf("child_file_count: %lu\n", data->child_file_count);
    printf("child_entry_count: %lu\n", data->child_entry_count);


    // Cleanup
    if(!path_absolute) {
        free(src_path);
    }
    closedir(dir);
    free(header);
    free(data);


    return 0;
}

static void print_help() {
    printf("Usage:\n -p: pack a directory\n -u: unpack a .packr file\n -s: neglect metadata\n -l: path to directory\n "
           "-h: help\n");
}
