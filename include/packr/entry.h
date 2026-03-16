#pragma once
#define NAME_LEN_MAX 4096
#include <packr/types.h>
#include <packr/ops.h>

struct pack_header {
    char dirname[NAME_LEN_MAX];
    u64 total_size;        // file size
    u64 child_entry_count; // self explanatory
    u64 child_file_count;
    u64 child_dir_count;
    u64 total_entry_count;
    u64 total_dir_count;
    u64 total_file_count;
    u64 acc_time; // last access time
    u64 mod_time; // last modification time
    u64 sc_time;  // last status change time   u64 total_file_count;
    u16 dirname_length;
    u16 mode; // permissions
};
pack_header* pack_header_init();

struct file_entry {
    char filename[NAME_LEN_MAX];
    u64 size;     // file size
    u64 acc_time; // last access time
    u64 mod_time; // last modification time
    u64 sc_time;  // last status change time
    u16 filename_length;
    u16 mode; // file size
};

struct dir_entry {
    char dirname[NAME_LEN_MAX];
    u64 child_entry_count;
    u64 child_file_count;
    u64 child_dir_count;
    u64 size;     // directory size
    u64 acc_time; // last access time
    u64 mod_time; // last modification time
    u64 sc_time;  // last status change time
    u16 dirname_length;
    u16 mode; // permissions
};
