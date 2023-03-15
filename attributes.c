/*
Setting the attribute table, which controls palette selection
for the nametable. We copy it from an array in ROM to video RAM.
*/
#include "neslib.h"
#include <string.h>
#include <stdlib.h>

// include CC65 NES Header (PPU)
#include <nes.h>

// link the pattern table into CHR ROM
//#link "chr_generic.s"

// attribute table in PRG ROM
const char ATTRIBUTE_TABLE[0x40] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // rows 0-3
  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, // rows 4-7
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, // rows 8-11
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // rows 12-15
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // rows 16-19
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // rows 20-23
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // rows 24-27
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f  // rows 28-29
};

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[33] = { 
  0x1C,			// screen color

  0x0F,0x17,0x10,0x00,	// background palette 0
  0x0F,0x07,0x2D,0x00,	// background palette 1
  0x0F,0x13,0x10,0x00,	// background palette 2
  0x0F,0x16,0x10,0x00,       // background palette 3
    
  0x0D,0x17,0x20,0x00,	// sprite palette 0
  0x0D,0x17,0x20,0x00,	// sprite palette 1
  0x0D,0x17,0x20,0x00,	// sprite palette 2
  0x0D,0x27,0x20,0x00	// sprite palette 3

};

#define TILE 0xd8
#define ATTR 0


// define a 2x2 metasprite
#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   pal, \
        0,      8,      (code)+1,   pal, \
        8,      0,      (code)+2,   pal, \
        8,      8,      (code)+3,   pal, \
        128};

// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};


DEF_METASPRITE_2x2(char1right,0xd8,true);
DEF_METASPRITE_2x2_FLIP(char1left,0xd8,true);

void p(byte type, byte x, byte y, byte len)
{
  vram_adr(NTADR_A(x,y));
  vram_fill(0x90+type, len);
  vram_adr(NTADR_A(x+1,y+1));
  vram_fill(0x05, 1);
  vram_fill(0x04, len-4);
  vram_fill(0x06, 1);
}

#define NUM_ACTORS 2

byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
void *actor_sprite[NUM_ACTORS];

// main function, run after console reset
void main(void) {
  // set background palette colors
  pal_all(PALETTE);

  // Place the player
  actor_x[0]=60;
  actor_y[0]=143;
  actor_sprite[0]=&char1right;
  
  // Place the bot
  actor_x[1]=128;
  actor_y[1]=99;
  actor_sprite[1]=&char1left;
  
  // Draw bg fades
  vram_adr(NAMETABLE_A);
  vram_fill(0x04, 96); // Sky
  vram_fill(0x05, 96);
  vram_adr(NTADR_A(0,22));
  vram_fill(0x07, 10);  // Bottom left 1
  vram_fill(0x04, 12);  // Bottom center 1
  vram_fill(0x07, 9);   // Bottom right 1
  vram_fill(0x08, 11);  // Bottom left 2
  vram_fill(0x04, 12);  // Bottom center 2
  vram_fill(0x08, 11);  // Bottom right 2
  vram_fill(0x04, 32*5);// Bottom dark
  
  
  // Draw platforms BG
  p(0,8,20,16);
  p(1,10,17,4);
  p(1,18,17,4);
  p(1,14,14,4);
  
  // Set BG colors
  vram_adr(0x23C0);
  // copy attribute table from PRG ROM to VRAM
  vram_write(ATTRIBUTE_TABLE, sizeof(ATTRIBUTE_TABLE));

  // enable PPU rendering (turn on screen)
  ppu_on_all();

  // infinite loop
  while (1) {
    char i; // actor index
    char oam_id; // sprite ID
    
    // Controls
    char pad;
    pad = pad_poll(0);
    if(pad & PAD_LEFT)
    {
    	actor_x[0]-= 1;
        actor_sprite[0]=&char1left;
    }
    else if(pad & PAD_RIGHT)
    {
    	actor_x[0]+= 1;
        actor_sprite[0]=&char1right;
    }
    
    // Update Sprites
    

    // start with OAMid/sprite 0
    oam_id = 0;
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) {
      oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, actor_sprite[i]);
      //actor_x[i] += actor_dx[i];
      //actor_y[i] += actor_dy[i];
    }
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
    // wait for next frame
    
    
    ppu_wait_frame();
  }
}
