#pragma once
#include <dirent.h>
#include <packr/types.h>
#define DEFAULT_ROOT_DIR 0

struct dir_data {
    u64 child_entry_count; // self explanatory
    u64 child_file_count;
    u64 child_dir_count;
    u64 total_size; // file size
    u64 total_entry_count;
    u64 total_dir_count;
    u64 total_file_count;
    u64 acc_time; // last access time
    u64 mod_time; // last modification time
    u64 sc_time;  // last status change time   u64 total_file_count;
    u16 dirname_length;
    u8 mode;
};
struct dir_data* get_dir_data(DIR* dir, char* dir_str, bool at_parent);
