#include <packr/utils.h>
#include <packr/entry.h>
#include <stddef.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>


char* join_to_path(char* filename, char* cwd) {
    if(!filename || !cwd) {
        return NULL;
    }


    char* temp = malloc(strlen(filename) + strlen(cwd) + 2); // +1 to for the extra '/', +1 for the null terminator
    strncpy(temp, cwd, strlen(cwd) + 1);                     // +1 to copy the null terminator
    size_t curr_temp_len = strlen(temp); // Needed due to overwriting and rewriting the null terminator
    temp[curr_temp_len] = '/';
    temp[curr_temp_len + 1] = '\0';

    strcat(temp, filename);

    // now it's the cwd + '/' + filename
    cwd = temp;

    return cwd;
}

void add_dirname(dir_entry* dir_ent, char* named_as, char* src_path) {
    if(named_as) {
        memcpy(dir_ent->dirname, named_as, strlen(named_as) + 1); // +1 for the \0
        dir_ent->dirname_length = strlen(named_as);
    } else {
        i16 slash_last_instance = 0;
        for(size_t i = 0; i < strlen(src_path); i++) {
            if(src_path[i] == '/') {
                slash_last_instance = i;
            }
        }

        char* target_name = src_path + slash_last_instance + (slash_last_instance ? 1 : 0); // +1 to skip the last '/'
        memcpy(dir_ent->dirname, target_name, strlen(target_name) + 1);                     // +1 to include the \0
        dir_ent->dirname_length = strlen(dir_ent->dirname);
    }
}
