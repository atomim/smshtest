;=====================================================
; Ultra-Compact Metasprite Renderer
; 
; Matches neslib calling pattern - pass oam_id, return new oam_id
;
; unsigned char __fastcall__ oam_spr_2x2(x, y, oam_id, variant, tile);
; unsigned char __fastcall__ oam_spr_1x1(x, y, oam_id, variant, tile);
;
; Stack layout for __fastcall__:
;   A = tile (last param)
;   (sp)+0 = variant
;   (sp)+1 = oam_id
;   (sp)+2 = y
;   (sp)+3 = x
;
; Returns: new oam_id in A
;
; variant: 0-3 = pal 0-3, 4-7 = pal 0-3 flipped
;=====================================================

.importzp sp

OAM_BUF = $0200

.segment "ZEROPAGE"
SPR_X:      .res 1
SPR_Y:      .res 1
SPR_TILE:   .res 1
SPR_ATTR:   .res 1
SPR_X8:     .res 1
SPR_Y8:     .res 1

.segment "CODE"

.export _oam_spr_2x2
.export _oam_spr_1x1

;------------------------------------------------
; unsigned char __fastcall__ oam_spr_1x1(x, y, oam_id, variant, tile)
; Returns: oam_id + 4
;------------------------------------------------
_oam_spr_1x1:
    sta SPR_TILE        ; A = tile

    ; Read variant (offset 0)
    ldy #0
    lda (sp),y
    tax                 ; save in X
    and #$03
    sta SPR_ATTR
    txa
    and #$04
    beq :+
    lda #$40
    ora SPR_ATTR
    sta SPR_ATTR
:
    ; Read oam_id (offset 1)
    iny
    lda (sp),y
    tax                 ; X = oam_id
    
    ; Read y coord (offset 2)
    iny
    lda (sp),y
    sta SPR_Y
    
    ; Read x coord (offset 3)
    iny
    lda (sp),y
    sta SPR_X
    
    ; Write single sprite
    lda SPR_X
    sta OAM_BUF+3,x
    lda SPR_Y
    sta OAM_BUF+0,x
    lda SPR_TILE
    sta OAM_BUF+1,x
    lda SPR_ATTR
    sta OAM_BUF+2,x
    
    ; Return oam_id + 4
    txa
    clc
    adc #4
    tax             ; save return value in X
    
    ; Pop 4 bytes from stack
    lda sp
    clc
    adc #4
    sta sp
    bcc :+
    inc sp+1
:   txa             ; return value back to A
    rts

;------------------------------------------------
; unsigned char __fastcall__ oam_spr_2x2(x, y, oam_id, variant, tile)
; Returns: oam_id + 16
;------------------------------------------------
_oam_spr_2x2:
    sta SPR_TILE        ; A = tile

    ; Read variant (offset 0)
    ldy #0
    lda (sp),y
    pha                 ; save variant
    and #$03
    sta SPR_ATTR
    pla
    and #$04
    bne @do_flip
    
;--- 2x2 Normal ---
    ; Read oam_id
    iny
    lda (sp),y
    tax
    
    ; Read y
    iny
    lda (sp),y
    sta SPR_Y
    clc
    adc #8
    sta SPR_Y8
    
    ; Read x
    iny
    lda (sp),y
    sta SPR_X
    clc
    adc #8
    sta SPR_X8
    
    ; Sprite 0: (0,0)
    lda SPR_X
    sta OAM_BUF+3,x
    lda SPR_Y
    sta OAM_BUF+0,x
    lda SPR_TILE
    sta OAM_BUF+1,x
    lda SPR_ATTR
    sta OAM_BUF+2,x
    
    ; Sprite 1: (0,+8)
    lda SPR_X
    sta OAM_BUF+7,x
    lda SPR_Y8
    sta OAM_BUF+4,x
    lda SPR_TILE
    clc
    adc #1
    sta OAM_BUF+5,x
    lda SPR_ATTR
    sta OAM_BUF+6,x
    
    ; Sprite 2: (+8,0)
    lda SPR_X8
    sta OAM_BUF+11,x
    lda SPR_Y
    sta OAM_BUF+8,x
    lda SPR_TILE
    clc
    adc #2
    sta OAM_BUF+9,x
    lda SPR_ATTR
    sta OAM_BUF+10,x
    
    ; Sprite 3: (+8,+8)
    lda SPR_X8
    sta OAM_BUF+15,x
    lda SPR_Y8
    sta OAM_BUF+12,x
    lda SPR_TILE
    clc
    adc #3
    sta OAM_BUF+13,x
    lda SPR_ATTR
    sta OAM_BUF+14,x
    
    jmp @done

;--- 2x2 Flipped ---
@do_flip:
    lda #$40
    ora SPR_ATTR
    sta SPR_ATTR
    
    ; Read oam_id
    iny
    lda (sp),y
    tax
    
    ; Read y
    iny
    lda (sp),y
    sta SPR_Y
    clc
    adc #8
    sta SPR_Y8
    
    ; Read x
    iny
    lda (sp),y
    sta SPR_X
    clc
    adc #8
    sta SPR_X8
    
    ; Sprite 0: (+8,0)
    lda SPR_X8
    sta OAM_BUF+3,x
    lda SPR_Y
    sta OAM_BUF+0,x
    lda SPR_TILE
    sta OAM_BUF+1,x
    lda SPR_ATTR
    sta OAM_BUF+2,x
    
    ; Sprite 1: (+8,+8)
    lda SPR_X8
    sta OAM_BUF+7,x
    lda SPR_Y8
    sta OAM_BUF+4,x
    lda SPR_TILE
    clc
    adc #1
    sta OAM_BUF+5,x
    lda SPR_ATTR
    sta OAM_BUF+6,x
    
    ; Sprite 2: (0,0)
    lda SPR_X
    sta OAM_BUF+11,x
    lda SPR_Y
    sta OAM_BUF+8,x
    lda SPR_TILE
    clc
    adc #2
    sta OAM_BUF+9,x
    lda SPR_ATTR
    sta OAM_BUF+10,x
    
    ; Sprite 3: (0,+8)
    lda SPR_X
    sta OAM_BUF+15,x
    lda SPR_Y8
    sta OAM_BUF+12,x
    lda SPR_TILE
    clc
    adc #3
    sta OAM_BUF+13,x
    lda SPR_ATTR
    sta OAM_BUF+14,x

@done:
    ; Return oam_id + 16
    txa
    clc
    adc #16
    tax             ; save return value in X
    
    ; Pop 4 bytes from stack
    lda sp
    clc
    adc #4
    sta sp
    bcc :+
    inc sp+1
:   txa             ; return value back to A
    rts