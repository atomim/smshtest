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


// Sprite allocation
// 64 tiles per char. 4 tiles per sprite. 16 Sprites total
// 1 stand
// 2 crouch (used for both starting jump and crouch)
// 3 jump
// 4 fast fall
// 5 run (Frame 1/x)
// 6 dash
// 7 ledge

// Todo:
// !create level (day 1)
// !create shadows (day 1)
// !create sprites (day 1)
// !subpixel physics (day 1.5)
// !move left and righ (day 1.5)
// !jump (day 1.5)
// !collision with level (day 1.5)
// !bot player (day 2)
// !crouch animation when jumping (day 2)
// !demo mode (day 2)
// !jump sprite (day 2)
// !fast fall (day 2.5)
// !fast fall sprite (day 2.5)
// !running (day 2.5)
// !run sprite (day 2.5)
// !fall through (day 2.5)
// !improve input, precalc edge (day 3)
// !optimize ai randomness (day 3)
// !edge grab sprite (day 2.5)
// *edge grab
// *dash
// *more clear attack/animation state 
// *crouch
// *better fall through control
// *crouch dash cancel
// *running inertia
// *dash dancing support
// *dash attack
// !ai to avoid falling off(day 3)
// *neutral attack
// *clarify states and logic more(enums and masks)
// *support coordinates outside of screen
// *KO on edges, spawning
// *Make opposite intents mutually exclusive
// *optimize platform iteration and access
// !warn on frame drops (day 3)
// !optimize player iteration(day 2.5)
// *optimizeintent access
// *optimize physics iteration


DEF_METASPRITE_2x2(char1right,0xd8,true);
DEF_METASPRITE_2x2_FLIP(char1left,0xd8,true);

DEF_METASPRITE_2x2(char1right_crouch,0xdc,true);
DEF_METASPRITE_2x2_FLIP(char1left_crouch,0xdc,true);

DEF_METASPRITE_2x2(char1right_jump,0xe0,true);
DEF_METASPRITE_2x2_FLIP(char1left_jump,0xe0,true);

DEF_METASPRITE_2x2(char1right_fast_fall,0xe4,true);
DEF_METASPRITE_2x2_FLIP(char1left_fast_fall,0xe4,true);

DEF_METASPRITE_2x2(char1right_run,0xe8,true);
DEF_METASPRITE_2x2_FLIP(char1left_run,0xe8,true);

DEF_METASPRITE_2x2(char1right_ledge,0xf0,true);
DEF_METASPRITE_2x2_FLIP(char1left_ledge,0xf0,true);



void p(byte type, byte x, byte y, byte len)
{
  vram_adr(NTADR_A(x,y));
  vram_fill(0x90+type, len);
  vram_adr(NTADR_A(x+1,y+1));
  vram_fill(0x05, 1);
  vram_fill(0x04, len-4);
  vram_fill(0x06, 1);
}

struct state{
  bool on_ground;
  bool moving_left;
  bool moving_right;
  bool direction_changed;
  bool crouching;
  byte jump_crouch_frames;
  byte walk_frames;
  byte dash_frames;
  byte movement_hold_frames; // for crouch canceling dash
  byte attack_hold_frames;   // for attacks
  byte running;
  byte double_jumps_left;
};
struct intent{
  bool left;
  bool right;
  bool jump;
  //bool short_jump; // Cancel jump by releasing during crouch.
  //bool double_jump;
  bool crouch;
  bool dash;
  bool fall_through;
  bool fast_fall;
};

struct params{
  short int jump_force;
  short int short_hop_force;
  short int walk_speed;
  short int run_speed;
  short int dash_speed;
  byte dash_frames;
  byte frames_to_run;
  byte double_jumps;
  byte jump_crouch_frames;
  short int fall_force;
  short int fall_limit;
  short int fast_fall;
};

struct platform{
  byte x1;
  byte x2;
  byte y;
  byte type;
};

#define NUM_ACTORS 2
#define NUM_PLATFORMS 4

byte actor_x[NUM_ACTORS];      // Position
byte actor_y[NUM_ACTORS];
byte actor_xf[NUM_ACTORS];     // Fraction
byte actor_yf[NUM_ACTORS];
short int actor_speedx[NUM_ACTORS]; // Speed
short int actor_speedy[NUM_ACTORS];
void *actor_sprite[NUM_ACTORS];// Which sprite to show
struct state actor_state[NUM_ACTORS];
struct intent actor_intent[NUM_ACTORS];
struct params actor_params[NUM_ACTORS]; // Todo: move to rom
struct platform platforms[NUM_PLATFORMS];

// try to push a frequent pointer to zp.
#pragma bss-name (push,"ZEROPAGE")
#pragma data-name(push,"ZEROPAGE")
struct state* a_state;
struct intent* a_intent;
struct params* a_params;
struct platform* cur_platform;
#pragma bss-name (pop)
#pragma data-name(pop)


byte p_count=0;

void addp(byte type, byte x, byte y, byte len)
{
  platforms[p_count].x1=x*8;
    platforms[p_count].x2=(x+len)*8;
    platforms[p_count].y=y*8;
    platforms[p_count].type=type;
    ++p_count;
}

void reset_level_and_bg()
{
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
  
  // reset platforms count
  p_count = 0;
  
  // Draw platforms BG
  p(0,8,20,16);addp(0,8,20,16);
  p(1,10,17,4);addp(1,10,17,4);
  p(1,18,17,4);addp(1,18,17,4);
  p(1,14,14,4);addp(1,14,14,4);
  
  // Set BG colors
  vram_adr(0x23C0);
  // copy attribute table from PRG ROM to VRAM
  vram_write(ATTRIBUTE_TABLE, sizeof(ATTRIBUTE_TABLE));

}

void simulate_player(unsigned char num)
{
  // TODO: optimize perf
  unsigned int r = rand();
  unsigned char r128 = r&0x7f;
  //unsigned char player =(num&0x300>>8); // move outside
  //if(player==num) // apply on heavy parts or whole logic.
  
  unsigned char j;
  //short int dy=;
  //char dx;
  signed char id_under =-1;
  signed char id_right=-1;
  signed char id_left=-1;
  //unsigned char closest_platform = 0;
  for(j=0;j<p_count;++j)
  {
    bool isLeft;
    bool isRight;
    cur_platform=&platforms[j];
    isLeft = actor_x[num]+8>cur_platform->x1;
    isRight = actor_x[num]+8<cur_platform->x2;

    if(isLeft&&isRight)
    {
      bool isUnder;
      isUnder=actor_y[num]+17>=cur_platform->y;
      if(isUnder)
      {
        id_under=j;
        //todo: make sure it is closest under
      }
    }else
    {
      if(isLeft)
      {
        id_left=j;
        //todo: make sure it is closest
      }
      if(isRight)
      {
        id_right=j;
        //todo: make sure it is closest
      }
    }

  }
  
  switch(r128)
  {
    case 1:
      actor_intent[num].jump = true;
      break;
    case 2:
    case 3:
      actor_intent[num].jump = false;
      break;
    case 4: //1/ stop. 2/
      actor_intent[num].left = false;
      actor_intent[num].right = false;
      actor_intent[num].crouch = false;
      break;
    case 5:
      actor_intent[num].left = true;
      break;
    case 6:
      actor_intent[num].right = true;
      break;
    case 7:
      actor_intent[num].fast_fall = true;
      break;
    case 8:
    case 9:
    case 10:
      actor_intent[num].fast_fall = false;
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17: 
      if(id_under==-1)
      {
        if(id_left!=-1)
        {
          actor_intent[num].left=true;
          actor_intent[num].right=false;
        }
        if(id_right!=-1)
        {
          actor_intent[num].right=true;
          actor_intent[num].left=false;
        }
        actor_intent[num].fast_fall=false;
        if(actor_speedy[num]>0)
        {
          actor_intent[num].jump=true;
        }
      }
      break;
  }
}

void initialize_player(byte num, byte type, byte x, byte y)
{
  actor_x[num]=x;
  actor_y[num]=y;
  actor_xf[num]=0;
  actor_yf[num]=0;
  actor_speedx[num]=0;
  actor_speedy[num]=0;
  switch(type)
  {
    case 0: // L as ref (x/10*8*256)
      actor_params[num].jump_force = 512;      // 2.5 (M)
      actor_params[num].short_hop_force = 307;  // 1.5 (M)
      actor_params[num].walk_speed = 246;       // 1.2 (M)
      actor_params[num].run_speed = 328;        // 1.6 (M, young, 1.2 old)
      actor_params[num].frames_to_run = 30;     // 2s (custom)
      actor_params[num].dash_speed = 367;       // 1.8 (M)
      actor_params[num].dash_frames = 367;       // 1.8 (M)
      actor_params[num].double_jumps = 1;       // 1 (All)
      actor_params[num].jump_crouch_frames = 6; // 6 (M)
      actor_params[num].fall_force = 22;        // 0.11 (M, Grav)
      actor_params[num].fall_limit = 436;       // 2.13 (M)
      actor_params[num].fast_fall = 614;        // 3 (M)
      break;
  }
}


// main function, run after console reset
void main(void) {
  char pad = 0;
  char last_pad = 0;
  char pad_rising = 0;
  char pad_falling =0;
  char clock=0;
  bool demo_mode_on = true;
  
  // set background palette colors
  pal_all(PALETTE);

  initialize_player(0,0,54+10,143);
  initialize_player(1,0,128,99);
  
  // Draw bg and set platforms data.
  reset_level_and_bg();
  
  //
  if (demo_mode_on)
  {
    vram_adr(NTADR_A(1,26));
    vram_write("Press START to stop Demo mode.", 30);
  }

  // enable PPU rendering (turn on screen)
  ppu_on_all();

  // infinite loop
  while (1) {
    unsigned char i, j; // actor index
    unsigned char oam_id; // sprite ID
    unsigned char newclock;
    
    short int speed;

    
    // Controls
    last_pad = pad;
    pad = pad_poll(0);
    pad_rising = pad^last_pad&pad;
    pad_falling = pad^last_pad&last_pad;
    
    
    // Disable demo mode
    if(demo_mode_on && pad & PAD_START)
    {
      ppu_wait_frame();
      ppu_off();
      reset_level_and_bg();
      ppu_on_all();
      //ppu_wait_frame();
      demo_mode_on=false;
    }
    
    // Control of player 1 intent based on controller.
    
    if(demo_mode_on)
    {
      simulate_player(0);
    } 
    else
    {
      // Reset left/right.
      actor_intent[0].left = false;
      actor_intent[0].right = false;
      if(pad & PAD_LEFT)
      {
          actor_intent[0].left = true;
      }
      else if(pad & PAD_RIGHT)
      {
          actor_intent[0].right = true;
      }
      
      // Jump / cancel jump when stat changes. Let simulation update consume intent in between.
      if(pad_rising & PAD_A)
      {
      	actor_intent[0].jump = true;
      }
      if((pad & PAD_A )==false && ((last_pad & PAD_A ) ==true))
      {
      	actor_intent[0].jump = false;
      }

      actor_intent[0].fast_fall = pad & PAD_DOWN;
      actor_intent[0].crouch = pad & PAD_DOWN;

    }
    
    // Simulate player 2 (not affected by demo mode)
    simulate_player(1);
    
    
    // Actor State and intent physics
    //todo: split some state updates out
    for (i=0; i<NUM_ACTORS; i++) 
    {
      a_state=&actor_state[i];
            
      // Walking or running
      a_state->running=false;
      if(a_state->walk_frames>actor_params[i].frames_to_run)
      {
        speed = actor_params[i].run_speed;
        a_state->running=true;
      
      }
      else
      {
        speed = actor_params[i].walk_speed;
      }
      // Todo: speed behaviormin air
      if (actor_intent[i].left)
      { 
         actor_speedx[i]=-speed;
      }
      else if (actor_intent[i].right)
      {
        actor_speedx[i]=speed;
      }
      
      if (a_state->on_ground == false) // on air
      {
        // Reset crouch intent on air.
        actor_intent[i].crouch = false;
        // Fall speed
      	actor_speedy[i] +=actor_params[i].fall_force; 
        actor_speedy[i] = MIN(actor_speedy[i],actor_params[i].fall_limit); 
        if(actor_intent[i].fast_fall && actor_speedy[i]>0)
        {
          actor_speedy[i] = actor_params[i].fast_fall;
        }
        // jump
        if(actor_intent[i].jump)
        {
          if(a_state->double_jumps_left>0)
          {
            actor_speedy[i] = -actor_params[i].jump_force; 
            a_state->double_jumps_left-=1;
          }
          actor_intent[i].jump = false;
        }
        // reset counters if fallen off mid-jump
        a_state->jump_crouch_frames = 0;
        a_state->walk_frames = 0;
      }
      else // on ground
      {
        // Reset fast fall intent on ground:
        actor_intent[i].fast_fall=false;
        if(actor_intent[i].crouch)
        {
          a_state->crouching=true;
          //todo: crouch cancelings here
        }
        else
        {
          a_state->crouching=false;
        }
        // Always reset double jump frames on ground.
        a_state->double_jumps_left=1;
        if(actor_intent[i].jump)
        {
          a_state->jump_crouch_frames++;
          if (a_state->jump_crouch_frames>=6)
          {
            // Do normal jump
            actor_speedy[i] = -actor_params[i].jump_force;
            actor_intent[i].jump = false;
            a_state->jump_crouch_frames = 0;
          }
        }
        else if (a_state->jump_crouch_frames>0) // Do short jump when cancelling jump early
        {
          actor_speedy[i] = -actor_params[i].short_hop_force;
          actor_intent[i].jump = false;
          a_state->jump_crouch_frames = 0;
        }
        
        // count walk frames
        if(a_state->direction_changed || abs(actor_speedx[i])<1)
        {
          a_state->walk_frames = 0;
        }
        else
        {
          a_state->walk_frames=MIN(++a_state->walk_frames,250);
        }
      }
      
      // Inertia
      // TODO: make state specific.
      actor_speedx[i]= actor_speedx[i]*4/5;
        
    }
    
    // Actor Physics
    
    for (i=0; i<NUM_ACTORS; i++) {
      short int actorxf;
      short int actoryf;
      a_state=&actor_state[i];
      actorxf = actor_xf[i]+actor_speedx[i];
      actoryf = actor_yf[i]+actor_speedy[i];
      // TODO: no loops here.
      while(actorxf>=255)
      {
        actor_x[i] +=1;
        actorxf-=0xff;
      }
      while(actorxf<0)
      {
        actor_x[i] -=1;
        actorxf+=0xff;
      }
      
      while(actoryf>=255)
      {
        actor_y[i] +=1;
        actoryf-=0xff;
      }
      while(actoryf<0)
      {
        actor_y[i] -=1;
        actoryf+=0xff;
      }
      
      
      actor_xf[i] = actorxf;
      actor_yf[i] = actoryf;
      
      // Collisions
      
      a_state->on_ground = false;
      for(j=0;j<p_count;++j)
      {
        cur_platform=&platforms[j];
        if(actor_speedy[i] >= 0
           && actor_y[i]+17>=cur_platform->y-(actor_speedy[i]>>8)+4*(cur_platform->type==1)
           && actor_y[i]+17<=cur_platform->y+8
           && actor_x[i]+8>cur_platform->x1
           && actor_x[i]+8<cur_platform->x2
           && (!a_state->crouching || cur_platform->type==0)
          )
        {
          actor_y[i] = cur_platform->y-17+4*(cur_platform->type==1);
          actor_speedy[i] = 0;
          actor_yf[i] = 0;
          a_state->on_ground = true;
        }
      }
      
      // Moving left/right
      a_state->direction_changed = false;
      if(actor_speedx[i]>0)
      {
        if(a_state->moving_right==false)
        {
          a_state->direction_changed = true;
        }
        a_state->moving_right=true;
        a_state->moving_left=false;
      }
      else if(actor_speedx[i]<0)
      {
        if(a_state->moving_left==false)
        {
          a_state->direction_changed = true;
        }
        actor_state[i].moving_left=true;
        actor_state[i].moving_right=false;
      }
      else
      {
        a_state->moving_right=false;
        a_state->moving_left=false;
      }
      
      // Select sprite
      // TODO: deduplicate
      if(actor_speedx[i]>0)//right
      {
        if(a_state->on_ground)
        {
          if(a_state->running)
          {
            actor_sprite[i] = &char1right_run;
          }
          else if(a_state->crouching)
          {
            actor_sprite[i] = &char1right_crouch;
          }
          else if(a_state->jump_crouch_frames==0)
          {
            actor_sprite[i] = &char1right;
          }
          else
          {
            // Todo: handle jump crouch as part of animation instead of last case
            actor_sprite[i] = &char1right_crouch;
          }
        }
        else
        {
          if(actor_intent[i].fast_fall) // Todo: use state instead of intent.
          {
            actor_sprite[i] = &char1right_fast_fall;
          }
          else
          {
            actor_sprite[i] = &char1right_jump;
          }
        }
      }
      else //left
      {
        if(a_state->on_ground)
        {
          if(a_state->running)
          {
            actor_sprite[i] = &char1left_run;
          }
          else if(a_state->crouching)
          {
            actor_sprite[i] = &char1left_crouch;
          }
          else if(a_state->jump_crouch_frames==0)
          {
            actor_sprite[i] = &char1left;
          }
          else
          {
            actor_sprite[i] = &char1left_crouch;
          }
        }
        else
        {
          if(actor_intent[i].fast_fall) // Todo: use state instead of intent.
          {
            actor_sprite[i] = &char1left_fast_fall;
          }
          else
          {
            actor_sprite[i] = &char1left_jump;
          }
        }
      }
    }
    
    
    
    
    // Update Sprites

    // start with OAMid/sprite 0
    oam_id = 0;
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) {
      // TODO: add camera
      oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, actor_sprite[i]);
    }
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
    // wait for next frame
    
    // loop to count extra time in frame
    {
      int i;
      for(i=0;i<150;++i)
      {
      }
    }
    
    ppu_wait_nmi();
    
    // detect frame drops
    newclock = nesclock();
    if(newclock-clock>1)
    {
      pal_col(0,0x16);
    }
    else
      pal_col(0,0x1c);
    clock=newclock;
    
  }
}
