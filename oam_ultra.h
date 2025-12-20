/*=====================================================
 * Ultra-Compact Metasprite System v2
 * 
 * Compatible with neslib oam_id pattern.
 * No external dependencies (no SPRID).
 * 
 * Usage:
 *   //#link "oam_ultra.s"
 *   #include "oam_ultra.h"
 *   
 *   #define CHAR1_STAND  0xd8
 *   
 *   // Same pattern as oam_meta_spr:
 *   oam_id = oam_spr_2x2(x, y, oam_id, variant, CHAR1_STAND);
 *=====================================================*/

#ifndef OAM_ULTRA_H
#define OAM_ULTRA_H

/* Variant encoding:
 * bits 0-1: palette (0-3)
 * bit 2:    flip H
 */
#define SPR_FLIP  0x04

/* Helper to compute variant from palette and facing */
#define SPR_VARIANT(pal, flip) (((pal) & 0x03) | ((flip) ? SPR_FLIP : 0))

/* Function prototypes - match neslib pattern */
unsigned char __fastcall__ oam_spr_1x1(unsigned char x, unsigned char y,
                                       unsigned char oam_id, unsigned char variant,
                                       unsigned char tile);

unsigned char __fastcall__ oam_spr_2x2(unsigned char x, unsigned char y,
                                       unsigned char oam_id, unsigned char variant,
                                       unsigned char tile);

#endif /* OAM_ULTRA_H */