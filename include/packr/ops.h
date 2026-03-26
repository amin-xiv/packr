#pragma once
#include <dirent.h>
#include <stdio.h>
#include <packr/types.h>
#include <packr/entry.h>
#define DEFAULT_ROOT_DIR 0
#define P_NOMETADATA 0B00000001

// Returns metadata about a given directory
pack_header* get_dir_data(DIR* dir, char* dir_str, u32 nest_count);

// Returns metadata about a given file
file_entry* get_file_data(char* file_path, u32 nest_count);

// Packs a directory by writing its metadata, and children's metadata and data(for files) in a given file(the pack file)
i8 pack_dir(dir_entry* dir_header, char* dir_path, FILE* pack_file, u8 opts, u32 nest_count);

// Initiates the packing process(calls pack_dir)
i8 pack(pack_header* header, char* dir_path, DIR* dir, FILE* pack_file, u8 opts);

// unpacks a given directory, by reading data from a pack_file
i8 unpack_dir(FILE* pack_file, u8 opts, u32 nest_count);

// Unpacks a given pack file(calls unpack_dir)
i8 unpack(FILE* pack_file, u8 opts, u32 nest_count);
