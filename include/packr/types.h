#include <stdint.h>
#include <packr/entry.h>


typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


typedef struct file_entry file_entry;
typedef struct dir_entry dir_entry;


typedef enum {
    FALSE = 0,
    TRUE = 1,
} bool;
