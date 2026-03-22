#pragma once

#include <stdint.h>
#include <packr/types.h>
#include <packr/ops.h>

#define NAME_LEN_MAX 4096

#define ENT_DIR_START ((uint8_t)0x01)
#define ENT_DIR_END ((uint8_t)0x02)
#define ENT_FILE ((uint8_t)0x04)
#define PACK_START ((uint8_t)0x08)
#define PACK_END ((uint8_t)0x10) // 16

struct file_entry {
    char filename[NAME_LEN_MAX];
    u64 size;     // file size
    u64 acc_time; // last access time
    u64 mod_time; // last modification time
    u64 sc_time;  // last status change time
    u16 filename_length;
    u16 mode;       // permissions
    u8 entry_class; // For future features(maybe)
} __attribute__((packed));

struct dir_entry {
    char dirname[NAME_LEN_MAX];
    u64 child_entry_count;
    u64 child_file_count;
    u64 child_dir_count;
    u64 total_entry_count;
    u64 total_dir_count;
    u64 total_file_count;
    u64 size;     // directory size(obviously)
    u64 acc_time; // last access time
    u64 mod_time; // last modification time
    u64 sc_time;  // last status change time
    u16 dirname_length;
    u16 mode;       // permissions
    u8 entry_class; // For future features(maybe)

} __attribute__((packed));

pack_header* pack_header_init();

// Needed by special markers
typedef enum {
    CHILD_ENT = 0x40,  // 64
    NESTED_ENT = 0x80, // 128
} entry_class;

// This is included BEFORE entry headers(to know how much memory to read)
typedef struct {
    u8 type; // Should only be set by ENT_* and PACK_* macros and binary ORed with enum entry_class
} __attribute__((packed)) special_marker;
