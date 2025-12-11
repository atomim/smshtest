
/*
 * fourscore.h - Unified DPCM-safe controller reading
 *
 * Automatically detects Four Score vs Standard controllers and handles both:
 *
 * FOUR SCORE:
 *   - Reads all 4 players
 *   - Uses signature-based glitch detection
 *   - fourscore_detected() returns 1
 *
 * STANDARD CONTROLLERS:
 *   - Reads P1 and P2 only
 *   - P3/P4 are zeroed
 *   - Uses keep-previous on glitch (safe fallback)
 *   - fourscore_detected() returns 0
 *
 * Usage:
 *   #include "fourscore.h"
 *   
 *   // At init:
 *   fourscore_set_dpcm(0);  // 1 if using DPCM samples
 *   
 *   // Each frame:
 *   fourscore_read();
 *   
 *   pad1 = fourscore_pad1;
 *   pad2 = fourscore_pad2;
 *   if (fourscore_detected()) {
 *       pad3 = fourscore_pad3;
 *       pad4 = fourscore_pad4;
 *   }
 */

#ifndef FOURSCORE_H
#define FOURSCORE_H

#include <stdint.h>

#pragma bss-name (push, "ZEROPAGE")
/* Current frame button states (active-high after inversion: 1 = pressed) */
extern uint8_t fourscore_pad1;
extern uint8_t fourscore_pad2;
extern uint8_t fourscore_pad3;  /* Zero if no Four Score */
extern uint8_t fourscore_pad4;  /* Zero if no Four Score */

/* Previous frame (for edge detection) */
extern uint8_t fourscore_pad1_old;
extern uint8_t fourscore_pad2_old;
extern uint8_t fourscore_pad3_old;
extern uint8_t fourscore_pad4_old;

/* Configuration: set to 1 if DPCM audio is playing */
extern uint8_t fourscore_dpcm_active;

/* Status: 1 if Four Score detected, 0 if standard controllers */
extern uint8_t fourscore_is_fourscore;

/* Glitch counter: increments each time a glitch is detected (for debugging) */
extern uint8_t fourscore_glitch_cnt;

/* Both-parity glitch counter: increments if BOTH reads are glitched (should stay 0!) */
extern uint8_t fourscore_both_glitch;

#pragma bss-name (pop)

/*
 * Read all controllers (call once per frame)
 * 
 * Cycles: ~1150 (no DPCM) / ~2300 (with DPCM)
 */
void __fastcall__ fourscore_read(void);

/*
 * Returns 1 if Four Score hardware detected, 0 otherwise
 * Valid after calling fourscore_read()
 */
uint8_t __fastcall__ fourscore_detected(void);

/* Helper macros */
#define fourscore_set_dpcm(active) (fourscore_dpcm_active = (active))

/* Edge detection helpers */
#define PAD_PRESSED(pad, old, btn)  (((pad) & (btn)) && !((old) & (btn)))
#define PAD_RELEASED(pad, old, btn) (!((pad) & (btn)) && ((old) & (btn)))
#define PAD_HELD(pad, btn)          ((pad) & (btn))

#endif /* FOURSCORE_H */