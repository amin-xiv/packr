#pragma once
#include <stddef.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>


char* join_to_path(char* filename, char* cwd) {
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
