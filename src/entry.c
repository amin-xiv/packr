#include <packr/types.h>
#include <packr/entry.h>
#include <packr/ops.h>
#include <malloc.h>
#include <string.h>

pack_header* pack_header_init() {
    pack_header* header = malloc(sizeof(pack_header));
    memset(header, '\0', sizeof(pack_header));
    return header;
};
