;===============================================================================
; FOURSCORE_UNIFIED.S - DPCM-safe controller reading for all configurations
;===============================================================================
;
; Handles BOTH Four Score and Standard controllers automatically:
;
; FOUR SCORE DETECTED (signature $10 or $04 found):
;   - Uses parity-flip technique (two reads with opposite CPU cycle parity)
;   - Selects clean read via "lower signature wins" rule
;   - All 4 players valid
;
; STANDARD CONTROLLERS (both signatures are $FF):
;   - Uses parity-flip technique (same two reads)
;   - If reads match: use result (no glitch)
;   - If reads differ: keep previous frame (safe fallback)
;   - P3/P4 zeroed (no controller connected)
;
; ZP Usage: 21 bytes
; Cycles: ~1150 (no DPCM) / ~2300 (with DPCM)
;
;===============================================================================

.export _fourscore_read
.export _fourscore_detected
.export _fourscore_pad1
.export _fourscore_pad2
.export _fourscore_pad3
.export _fourscore_pad4
.export _fourscore_pad1_old
.export _fourscore_pad2_old
.export _fourscore_pad3_old
.export _fourscore_pad4_old
.export _fourscore_dpcm_active
.export _fourscore_is_fourscore
.export _fourscore_glitch_cnt
.export _fourscore_both_glitch

;-------------------------------------------------------------------------------
; Constants
;-------------------------------------------------------------------------------
JOYPAD1         = $4016
JOYPAD2         = $4017
FOURSCORE_SIG1  = $10       ; Port 1 signature (bit 19 = 1)
FOURSCORE_SIG2  = $04       ; Port 2 signature (bit 18 = 1)
STANDARD_SIG    = $FF       ; Standard controller tail (all 1s)

;-------------------------------------------------------------------------------
; Zero Page Variables (19 bytes)
;-------------------------------------------------------------------------------
.segment "ZEROPAGE"

; Outputs (persist across frames)
_fourscore_pad1:        .res 1
_fourscore_pad2:        .res 1
_fourscore_pad3:        .res 1
_fourscore_pad4:        .res 1
_fourscore_pad1_old:    .res 1
_fourscore_pad2_old:    .res 1
_fourscore_pad3_old:    .res 1
_fourscore_pad4_old:    .res 1

; Config / status
_fourscore_dpcm_active: .res 1      ; Set to 1 if using DPCM audio
_fourscore_is_fourscore:.res 1      ; Result: 1 = Four Score, 0 = Standard
_fourscore_glitch_cnt:  .res 1      ; Glitch counter (for debugging)
_fourscore_both_glitch: .res 1      ; Both-parity glitch counter (should stay 0!)

; Saved A buffer (for DPCM comparison)
_fs_saved_a_p1:         .res 1
_fs_saved_a_p2:         .res 1
_fs_saved_a_p3:         .res 1
_fs_saved_a_p4:         .res 1
_fs_saved_a_sig1:       .res 1
_fs_saved_a_sig2:       .res 1

; Internals
_fs_sig1:               .res 1      ; Current read signature port 1
_fs_sig2:               .res 1      ; Current read signature port 2
_fs_dummy:              .res 1

; Total: 21 bytes

;===============================================================================
; CODE
;===============================================================================
.segment "CODE"

;===============================================================================
; _fourscore_read - Main entry point
;===============================================================================
.proc _fourscore_read
    ; Save old values for edge detection
    lda _fourscore_pad1
    sta _fourscore_pad1_old
    lda _fourscore_pad2
    sta _fourscore_pad2_old
    lda _fourscore_pad3
    sta _fourscore_pad3_old
    lda _fourscore_pad4
    sta _fourscore_pad4_old

    ; First read (writes to fourscore_pad1-4, _fs_sig1/2)
    jsr read_24bits
    
    ; Check if DPCM protection needed
    lda _fourscore_dpcm_active
    beq detect_mode             ; No DPCM = skip second read, go detect

    ;=== DPCM ACTIVE: Save first read, do second ===
    lda _fourscore_pad1
    sta _fs_saved_a_p1
    lda _fourscore_pad2
    sta _fs_saved_a_p2
    lda _fourscore_pad3
    sta _fs_saved_a_p3
    lda _fourscore_pad4
    sta _fs_saved_a_p4
    lda _fs_sig1
    sta _fs_saved_a_sig1
    lda _fs_sig2
    sta _fs_saved_a_sig2

    ; Parity offset (3 cycles) + second read
    bit _fs_dummy
    jsr read_24bits

    ;=== Now decide: Four Score or Standard? ===
detect_mode:
    ; Check signatures - if EITHER is valid, it's Four Score
    lda _fs_sig1
    cmp #FOURSCORE_SIG1
    beq is_fourscore
    lda _fs_sig2
    cmp #FOURSCORE_SIG2
    beq is_fourscore
    
    ; Also check saved A buffer (in case B was glitched)
    lda _fourscore_dpcm_active
    beq no_signature_found      ; No A buffer exists, skip to standard mode
    lda _fs_saved_a_sig1
    cmp #FOURSCORE_SIG1
    beq is_fourscore
    lda _fs_saved_a_sig2
    cmp #FOURSCORE_SIG2
    beq is_fourscore
    
no_signature_found:
    ; Neither signature valid → Standard controllers
    jmp standard_mode

;-----------------------------------------------------------------------
; FOUR SCORE MODE - use signature-based selection
;-----------------------------------------------------------------------
is_fourscore:
    lda #1
    sta _fourscore_is_fourscore
    
    ; If no DPCM, we're done (single read is clean)
    lda _fourscore_dpcm_active
    beq done
    
    ;--- Check for both-parity glitch (should NEVER happen) ---
    ; Port 1: if both sig1_A != $10 AND sig1_B != $10, both glitched
    lda _fs_saved_a_sig1
    cmp #FOURSCORE_SIG1
    beq @p1_a_ok                ; A is clean, skip
    lda _fs_sig1
    cmp #FOURSCORE_SIG1
    beq @p1_b_ok                ; B is clean, skip
    ; BOTH are wrong!
    inc _fourscore_both_glitch
@p1_a_ok:
@p1_b_ok:

    ; Port 2: if both sig2_A != $04 AND sig2_B != $04, both glitched
    lda _fs_saved_a_sig2
    cmp #FOURSCORE_SIG2
    beq @p2_a_ok
    lda _fs_sig2
    cmp #FOURSCORE_SIG2
    beq @p2_b_ok
    ; BOTH are wrong!
    inc _fourscore_both_glitch
@p2_a_ok:
@p2_b_ok:

    ; Select P1/P3 based on lower SIG1
    lda _fs_saved_a_sig1
    cmp _fs_sig1
    bcc fs_p1p3_from_a          ; A < B, use A (B was glitched)
    beq fs_p1p3_equal           ; A == B, no glitch on port 1
    ; B < A, use B (A was glitched)
    inc _fourscore_glitch_cnt
    jmp fs_select_p2p4
    
fs_p1p3_from_a:
    inc _fourscore_glitch_cnt   ; B was glitched
    lda _fs_saved_a_p1
    sta _fourscore_pad1
    lda _fs_saved_a_p3
    sta _fourscore_pad3
    jmp fs_select_p2p4
    
fs_p1p3_equal:
    ; Signatures equal - no glitch on this port, use B (already there)
    
fs_select_p2p4:
    ; Select P2/P4 based on lower SIG2
    lda _fs_saved_a_sig2
    cmp _fs_sig2
    bcc fs_p2p4_from_a          ; A < B, use A (B was glitched)
    beq fs_p2p4_equal           ; A == B, no glitch on port 2
    ; B < A, use B (A was glitched)
    inc _fourscore_glitch_cnt
fs_p2p4_equal:
    rts
    
fs_p2p4_from_a:
    inc _fourscore_glitch_cnt   ; B was glitched
    lda _fs_saved_a_p2
    sta _fourscore_pad2
    lda _fs_saved_a_p4
    sta _fourscore_pad4
done:
    rts

;-----------------------------------------------------------------------
; STANDARD MODE - use keep-previous on mismatch
;-----------------------------------------------------------------------
standard_mode:
    lda #0
    sta _fourscore_is_fourscore
    
    ; Zero P3/P4 (no controllers there)
    sta _fourscore_pad3
    sta _fourscore_pad4
    
    ; If no DPCM, single read is clean, done
    lda _fourscore_dpcm_active
    beq done
    
    ; Compare P1: if A != B, keep old
    lda _fs_saved_a_p1
    cmp _fourscore_pad1
    beq @p1_ok
    ; Mismatch - glitch detected, keep previous
    inc _fourscore_glitch_cnt
    lda _fourscore_pad1_old
    sta _fourscore_pad1
@p1_ok:

    ; Compare P2: if A != B, keep old
    lda _fs_saved_a_p2
    cmp _fourscore_pad2
    beq @p2_ok
    ; Mismatch - glitch detected, keep previous
    inc _fourscore_glitch_cnt
    lda _fourscore_pad2_old
    sta _fourscore_pad2
@p2_ok:
    rts

.endproc

;===============================================================================
; read_24bits - Read 24 bits from both ports
;===============================================================================
; Outputs: fourscore_pad1-4, _fs_sig1, _fs_sig2
;===============================================================================
.proc read_24bits
    ; Strobe controllers
    lda #$01
    sta JOYPAD1
    lsr a                       ; A = 0
    sta JOYPAD1

    ; Clear outputs
    sta _fourscore_pad1
    sta _fourscore_pad3
    sta _fs_sig1
    sta _fourscore_pad2
    sta _fourscore_pad4
    sta _fs_sig2

    ; 24 bits total: 3 outer × 8 inner
    ldx #3                      ; Outer loop counter

@outer_loop:
    ldy #8                      ; Inner loop counter
    
@inner_loop:
    lda JOYPAD1
    lsr a
    rol _fs_sig1
    rol _fourscore_pad3
    rol _fourscore_pad1
    lda JOYPAD2
    lsr a
    rol _fs_sig2
    rol _fourscore_pad4
    rol _fourscore_pad2
    dey
    bne @inner_loop
    
    dex
    bne @outer_loop
    rts
.endproc

;===============================================================================
; _fourscore_detected - Returns 1 if Four Score hardware detected
;===============================================================================
.proc _fourscore_detected
    lda _fourscore_is_fourscore
    rts
.endproc