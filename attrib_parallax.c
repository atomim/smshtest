// attrib_parallax.c
#include "attrib_parallax.h"
#include <string.h>
#include <neslib.h>


// ===== ATTRIBUTE TABLES (5 variants) =====
// These need to be generated from your base table with shifts

const byte ATTR_OVERRIDES[5][8] = {
    // [0] Offset -16px (shift left 2)
    { 0x05, 0x0a, 0x05, 0x00, 0x04, 0x0a, 0x01, 0x00 },

    // [1] Offset -8px (shift left 1)
    { 0x04, 0x09, 0x06, 0x01, 0x00, 0x09, 0x06, 0x00 },

    // [2] Offset 0px (NEUTRAL)
    { 0x00, 0x05, 0x0a, 0x05, 0x00, 0x04, 0x0a, 0x01 },

    // [3] Offset +8px (shift right 1)
    { 0x00, 0x04, 0x09, 0x06, 0x01, 0x00, 0x09, 0x06 },

    // [4] Offset +16px (shift right 2)
    { 0x01, 0x00, 0x05, 0x0a, 0x05, 0x00, 0x04, 0x0a },
};
// ===== ATTRIBUTE UPDATE FUNCTION =====

const byte* get_attrib_ptr(char camera_offset_x) {
    // Map camera offset to table index
    // camera_offset: -16 → 0, -8 → 1, 0 → 2, +8 → 3, +16 → 4
    // Formula: idx = 2 + (offset / 8)
    
    byte table_idx;
    
    char offset = (byte)(nesclock() & 0x03)<<0;
    
    // Convert offset to index (using fixed-point division by 8)
    table_idx = ((byte)((15+camera_offset_x)>>0+offset*0) >> 2);  // >> 3 = divide by 8
  
  
    // Double-check bounds (paranoid, but safe)
  
    if (table_idx > 128) table_idx = 0;
    if (table_idx > 4) table_idx = 4;
    // (table_idx can't be < 0 after conversion; unsigned safe)
  
  return (ATTR_OVERRIDES[4-table_idx]);
  //return ATTR_OVERRIDES[4];
}
