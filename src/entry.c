#include <packr/types.h>
#include <packr/entry.h>
#include <malloc.h>

pack_header* pack_header_init() {
    pack_header* header = malloc(sizeof(struct pack_header));
    return header ? header : NULL;
};
