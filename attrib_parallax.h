// attrib_parallax.h
#ifndef ATTRIB_PARALLAX_H
#define ATTRIB_PARALLAX_H


#include <neslib.h>
#include <stdlib.h>


// 5 pre-computed attribute tables for offsets: -16, -8, 0, +8, +16px
// Index mapping: 0=(-16px), 1=(-8px), 2=(0px neutral), 3=(+8px), 4=(+16px)
extern const byte ATTR_TABLES[5][64];

// Update attribute table based on camera offset
const byte* get_attrib_ptr(char camera_offset_x);

#endif

