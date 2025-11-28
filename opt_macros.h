
/*
 * NES Platform Fighter - Simple C Optimization Macros
 * 
 * These macros avoid expensive cc65 library calls using only C.
 * No inline assembly required!
 * 
 * REQUIREMENTS:
 * 1. Add these to your zeropage:
 *    #pragma bss-name("ZEROPAGE")
 *    byte abs_a, abs_b;
 *    #pragma bss-name("BSS")
 * 
 * 2. Include this header where needed
 */

#ifndef SIMPLE_OPT_MACROS_H
#define SIMPLE_OPT_MACROS_H

/* ========================================================================
 * ZEROPAGE TEMPORARIES
 * 
 * Define these in your zeropage segment:
 *   byte abs_a, abs_b;
 * ======================================================================== */

extern unsigned char abs_a, abs_b;

/* ========================================================================
 * ABS_DIFF(arr, idx1, idx2, result)
 * 
 * Computes: result = |arr[idx1] - arr[idx2]|
 * 
 * ~47 cycles vs ~145 for library abs() chain (3× faster!)
 * 
 * Usage:
 *   ABS_DIFF(actor_x, j, num, distx);
 *   ABS_DIFF(actor_y, j, num, disty);
 * ======================================================================== */

#define ABS_DIFF(arr, idx1, idx2, result) \
    abs_a = (arr)[(idx1)]; \
    abs_b = (arr)[(idx2)]; \
    if (abs_a > abs_b) { \
        (result) = abs_a - abs_b; \
    } else { \
        (result) = abs_b - abs_a; \
    }

/* ========================================================================
 * ABS_DIFF_VARS(var1, var2, result)
 * 
 * Computes: result = |var1 - var2| for direct variables
 * 
 * Usage:
 *   ABS_DIFF_VARS(player_x, enemy_x, distance);
 * ======================================================================== */

#define ABS_DIFF_VARS(var1, var2, result) \
    abs_a = (var1); \
    abs_b = (var2); \
    if (abs_a > abs_b) { \
        (result) = abs_a - abs_b; \
    } else { \
        (result) = abs_b - abs_a; \
    }

/* ========================================================================
 * ABS_VAL(val, result)
 * 
 * Computes: result = |val| for signed byte
 * 
 * Usage:
 *   signed char diff = (signed char)(a - b);
 *   ABS_VAL(diff, distance);
 * ======================================================================== */

#define ABS_VAL(val, result) \
    if ((signed char)(val) < 0) { \
        (result) = -(signed char)(val); \
    } else { \
        (result) = (val); \
    }

/* ========================================================================
 * NESLIB MIN/MAX vs SAFE versions
 * 
 * NESLIB macros (#define MIN(x1,x2) ((x1)<(x2)?(x1):(x2)))
 * are FASTER for simple variables:
 *   MIN(var1, var2)  - neslib: ~15 cycles, safe: ~27 cycles
 * 
 * But SAFE macros are 4× FASTER for array access or expressions:
 *   MIN(actor_x[i], actor_x[j])  - neslib: ~138 cycles, safe: ~35 cycles
 *   MIN(a+10, b+20)              - neslib: ~150 cycles, safe: ~35 cycles
 * 
 * WHY: neslib macros evaluate arguments twice, triggering:
 *   1. Library calls (pusha0, tosicmp) for comparison
 *   2. Re-evaluation of the winning expression for result
 * 
 * RECOMMENDATION:
 *   - Simple vars: use neslib MIN/MAX
 *   - Arrays/expressions: use MIN_SAFE/MAX_SAFE
 * ======================================================================== */

/* SAFE versions - evaluate args once, use for arrays/expressions */
#define MAX_SAFE(x1, x2, result) \
    abs_a = (x1); \
    abs_b = (x2); \
    (result) = (abs_a > abs_b) ? abs_a : abs_b

#define MIN_SAFE(x1, x2, result) \
    abs_a = (x1); \
    abs_b = (x2); \
    (result) = (abs_a < abs_b) ? abs_a : abs_b

/* ========================================================================
 * CLAMP(val, min_val, max_val, result)
 * 
 * Computes: result = clamp(val, min, max)
 * ======================================================================== */

#define CLAMP(val, min_val, max_val, result) \
    abs_a = (val); \
    if (abs_a < (min_val)) { \
        (result) = (min_val); \
    } else if (abs_a > (max_val)) { \
        (result) = (max_val); \
    } else { \
        (result) = abs_a; \
    }

/* ========================================================================
 * COMPARISON REMINDER
 * 
 * cc65 generates EXPENSIVE code for 16-bit comparisons!
 * 
 * BAD:  if (speed_x > target_x)     // Uses pushax + tosicmp (~113 cycles)
 * 
 * For 8-bit values, always use byte type:
 * GOOD: byte a = speed_x;
 *       byte b = target_x;
 *       if (a > b)                  // Simple cmp (~10 cycles)
 * ======================================================================== */

/* ========================================================================
 * 16-BIT PROMOTION REMINDER
 * 
 * cc65 promotes to 16-bit on ANY arithmetic with constants!
 * 
 * BAD:  actor_x[i] + 8 > platform_x    // Promotes to 16-bit!
 * 
 * GOOD: byte center = (byte)(actor_x[i] + 8);  // Force 8-bit
 *       center > platform_x                      // Now 8-bit compare
 * ======================================================================== */

#endif /* SIMPLE_OPT_MACROS_H */