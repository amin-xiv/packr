#pragma once
#include <dirent.h>
#include <stdio.h>
#include <packr/types.h>
#include <packr/entry.h>
#define DEFAULT_ROOT_DIR 0
#define P_NOMETADATA 0B00000001

pack_header* get_dir_data(DIR* dir, char* dir_str, u32 nest_count);
file_entry* get_file_data(char* file_path);
i8 pack(pack_header* header, char* dir_path, DIR* dir, FILE* pack_file, u8 opts);
i8 pack_dir(dir_entry* dir_header, char* dir_path, FILE* pack_file, u8 opts, u32 nest_count);
