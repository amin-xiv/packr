#include <stdio.h>
#include <getopt.h>
#include <packr/types.h>

void print_help();

int main(int argc, char** argv) {
    char cur_opt;
    char* src_path;
    bool op_provided = FALSE; // Whether a -p or -u option was provided
    bool no_metadata = FALSE;
    bool path_provided = FALSE;

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

    return 0;
}

void print_help() {
    printf("Usage:\n -p: pack a directory\n -u: unpack a .packr file\n -s: neglect metadata\n -l: path to directory\n "
           "-h: help\n");
}
