#pragma once
#define NSEC_TO_SEC(x) (x / 1e9)
#include <packr/entry.h>

char* join_to_path(char* filename, char* cwd);
void add_dirname(dir_entry* dir_ent, char* named_as, char* src_path);
char* extract_filename(char* path);
