#pragma once
#include <dirent.h>
#include <packr/types.h>
#define DEFAULT_ROOT_DIR 0

pack_header* get_dir_data(DIR* dir, char* dir_str, u32 nest_count);
