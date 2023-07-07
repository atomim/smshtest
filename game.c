/*
Setting the attribute table, which controls palette selection
for the nametable. We copy it from an array in ROM to video RAM.
*/
#include "neslib.h"
#include <string.h>
#include <stdlib.h>

// include CC65 NES Header (PPU)
#include <nes.h>

//#defi ne CFGFILE test

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
    
  0x0D,0x17,0x32,0x00,	// sprite palette 0
  0x0D,0x17,0x27,0x00,	// sprite palette 1
  0x0D,0x17,0x29,0x00,	// sprite palette 2
  0x0D,0x17,0x26,0x00	// sprite palette 3

};

//switch this to disable asserts.
#define Assert(cond) if(cond)for(;;);
//#define Assert(cond) ;

#define ATTR 0

#define DEF_METASPRITE_2x2_VARS(name,code)\
DEF_METASPRITE_2x2(name##_a,code,0)\
DEF_METASPRITE_2x2(name##_b,code,1)\
DEF_METASPRITE_2x2(name##_c,code,2)\
DEF_METASPRITE_2x2(name##_d,code,3)\
DEF_METASPRITE_2x2_FLIP(name##_a2,code,0)\
DEF_METASPRITE_2x2_FLIP(name##_b2,code,1)\
DEF_METASPRITE_2x2_FLIP(name##_c2,code,2)\
DEF_METASPRITE_2x2_FLIP(name##_d2,code,3)\
void* name##_sprites[]={\
  &name##_a\
  ,&name##_b\
  ,&name##_c\
  ,&name##_d\
  ,&name##_a2\
  ,&name##_b2\
  ,&name##_c2\
  ,&name##_d2};
  





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
// 5 dash
// 6 run
// 7 ledge
// 8 ledge sway
// 9 skid

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
// *KO on arena edges, spawning
// *Make opposite intents mutually exclusive
// !optimize platform iteration and access (day 3)
// !warn on frame drops (day 3)
// !optimize player iteration(day 2.5)
// !optimize intent access(day 3.5)
// !optimize physics iteration(day 3.5)
// !add sprite for edge sway (day 3)
// /support edge sway
// !add debug print (day 4)
// *optimize debug print
// !add frame drop detection (day 3.5)
// !add vegetation
// !add bg decoration
// !fix failing jump and add asserts (day 4/5)

DEF_METASPRITE_2x2_VARS(char1stand,0xd8);
DEF_METASPRITE_2x2_VARS(char1crouch,0xdc);
DEF_METASPRITE_2x2_VARS(char1run,0xec);
DEF_METASPRITE_2x2_VARS(char1jump,0xe0);
DEF_METASPRITE_2x2_VARS(char1fast_fall,0xe4);
DEF_METASPRITE_2x2_VARS(char1dash,0xe8);
DEF_METASPRITE_2x2_VARS(char1ledge,0xf0);
DEF_METASPRITE_2x2_VARS(char1sway,0xf4);


void p(byte type, byte x, byte y, byte len)
{
  vram_adr(NTADR_A(x,y));
  vram_fill(0x90+type, len);
  vram_adr(NTADR_A(x+1,y+1));
  vram_fill(0x05, 1);
  vram_fill(0x04, len-4);
  vram_fill(0x06, 1);
  if(type==0)
  {
    int i;
    int pos=0;
    int max=len>>1;
    for(i=0;i<max;++i)
    {
      pos+=1+rand()%0x03;
      if(pos>len-1)pos-=len;
      vram_adr(NTADR_A(x+pos,y-1));
      vram_fill(0x92+rand()%2, 1);
    }
  }
}

// Todo: convert to defines
enum action_state
{
  ACTION_STAND_BY_GROUND=0,
  ACTION_STAND_BY_AIR=1,
  ACTION_CROUCHING_TO_JUMP_GROUND=2,
  ACTION_CROUCHING_GROUND=3,
  ACTION_HANGING_GROUND=4, //todo
  ACTION_WALKING_GROUND=5,
  ACTION_RUNNING_GROUND=6,
  ACTION_DASHING_GROUND=7,
  ACTION_TURNING_AROUND_GROUND=8, // todo
  ACTION_STOPPING_GROUND=9, // todo
  ACTION_EDGE_GRAB_GROUND=10, //todo
  ACTION_FAST_FALLING_AIR=11, 
  ACTION_ATTACK_GROUND=12, // todo
  ACTION_ATTACK_AIR=13 // todo
};

#define ON_GROUND(state) (((state)!=ACTION_STAND_BY_AIR)&&((state)!=ACTION_FAST_FALLING_AIR)&&((state)!=ACTION_ATTACK_AIR))

struct state{
  enum action_state current_action;
  //bool on_ground;
  bool moving_left;
  bool moving_right;
  bool direction_changed;
  //bool crouching;
  // hanging
  //byte jump_crouch_frames;
  //byte walk_frames;
  //byte dash_frames;
  //byte movement_hold_frames; // for crouch canceling dash
  //byte attack_hold_frames;   // for attacks
  //byte running;
  byte current_action_frames;
  byte double_jumps_left;
  bool on_edge;
};
struct intent{
  bool left; //todo: add directional intent and apply only with compatible actions
  bool right;
  bool jump;
  //bool short_jump; // Cancel jump by releasing during crouch.
  //bool double_jump;
  bool crouch;
  bool dash;
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
  byte y1;
  byte y2;
  byte type;
  byte height;
  byte can_fall_through;
  byte has_edge;
};

#define NUM_ACTORS 4
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
short int* a_speed_x;
short int* a_speed_y;
#pragma bss-name (pop)
#pragma data-name(pop)


byte p_count=0;

void addp(byte type, byte x, byte y, byte len)
{
  platforms[p_count].x1=x*8;
  platforms[p_count].x2=(x+len)*8;
  platforms[p_count].type=type;
  switch(type)
  {
    case 0:
        platforms[p_count].y1=y*8-2;
        platforms[p_count].y2=(y+1)*8;
        platforms[p_count].can_fall_through=false;
        platforms[p_count].has_edge=true;
      break;
    case 1:
        platforms[p_count].y1=y*8+4-2;
        platforms[p_count].y2=(y+1)*8;
        platforms[p_count].can_fall_through=true;
        platforms[p_count].has_edge=false;
      break;
  }
  ++p_count;
}

void reset_level_and_bg()
{
  char i;
  //set_rand(80);
  set_rand(379);
  
  for(i=0;i<19;++i)
  {
    
    vram_adr(NTADR_A(rand8()&0x1f,
                     (rand8()&0x07)+8));
    vram_fill(0x94+2+(i&0b00000011), 1);
  }
  set_rand(800);
  
  
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
  //p(1,8,20,16);addp(1,8,20,16);
  p(1,10,17,4);addp(1,10,17,4);
  p(1,18,17,4);addp(1,18,17,4);
  p(1,14,14,4);addp(1,14,14,4);
  
  // Set BG colors
  vram_adr(0x23C0);
  // copy attribute table from PRG ROM to VRAM
  vram_write(ATTRIBUTE_TABLE, sizeof(ATTRIBUTE_TABLE));

}

void print_state(byte player,short int adr)
{
  enum action_state cur_action=actor_state[player].current_action;
 
  PPU.mask =0x0;
  vram_adr(adr);
  vram_fill(0x30+cur_action, 1);
  //vram_write(":",1);
  
  switch(cur_action)
  {
    case ACTION_STAND_BY_GROUND:
      vram_write("S",1);
      break;
    case ACTION_STAND_BY_AIR:
      vram_write("A",1);
      break;
    case ACTION_CROUCHING_TO_JUMP_GROUND:
      vram_write("c",1);
      break;
    case ACTION_CROUCHING_GROUND:
      vram_write("C",1);
      break;
    //ACTION_HANGING_GROUND=4, //todo
    case ACTION_WALKING_GROUND:
      vram_write("W",1);
      break;
    case ACTION_RUNNING_GROUND:
      vram_write("R",1);
      break;
    case ACTION_DASHING_GROUND:
      vram_write("D",1);
      break;
    //ACTION_TURNING_AROUND_GROUND=8, // todo
    //ACTION_STOPPING_GROUND=9, // todo
    case ACTION_FAST_FALLING_AIR:
      vram_write("F",1);
      break;
    //ACTION_ATTACK_GROUND=11, // todo
    //ACTION_ATTACK_AIR=12 // todo
    default:
      break;
  }
  
  //a_intent=&actor_intent[player];
  vram_write(",",1);
  if(a_intent->left && a_intent->right)
  {
    vram_write("X",1);
  }
  else if(a_intent->right)
  {
    vram_write("R",1);
  }
  else if(a_intent->left)
  {
    vram_write("L",1);
  }
  else
  {
    vram_write(".",1);
  }
  if(a_intent->jump)
  {
    vram_write("J",1);
  }
  else
  {
    vram_write(".",1);
  }
  if(a_intent->crouch)
  {
    vram_write("C",1);
  }
  else
  {
    vram_write(".",1);
  }
  if(a_intent->fast_fall)
  {
    vram_write("F",1);
  }
  else
  {
    vram_write(".",1);
  }
  
  //if(a_state->current_action_frames<10)
  //{
  //vram_adr(adr+10+a_state->current_action_frames);
  //vram_write("-/",2);
  //}
    
    
  vram_adr(NTADR_A(0,0));
  PPU.mask =0b00011110;
    
}

char num_ai;

void simulate_player(unsigned char num)
{
  // TODO: optimize perf
  unsigned int r = rand();
  
  // todo: balance chances based on amount of ai's
  unsigned char r128;
  
  unsigned char j;
  //short int dy=;
  //char dx;
  signed char id_under =-1;
  signed char id_right=-1;
  signed char id_left=-1;
  
  Assert(num>NUM_ACTORS);

  
  if(num_ai==1)
  {
    r128 = r&0x7f;
  }
  else if(num_ai==2)
  {
    r128 = r&0x3f;
  }
  else if(num_ai==3)
  {
    r128 = r&0x7f;
    if(r128>0x5f)
    {
      r128=0;
    }
    else
    {
      r128=r128&0x1f;
    }
  }
  else if(num_ai==4)
  {
    r128 = r&0x1f;
  }
  
  //unsigned char closest_platform = 0;
  // find closest platforms for AI
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
      isUnder=actor_y[num]+17>=cur_platform->y1;
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
  {
    char i;
    for(i=MIN(20,r128);i>0;--i)
    {
    }
    switch(r128)
    {
      case 0:
        break;
      case 1:
        actor_intent[num].jump = true;
        actor_intent[num].crouch = false;
        break;
      case 2:
      case 3:
        actor_intent[num].jump = false;
        break;
      case 4: 
        actor_intent[num].left = false;
        actor_intent[num].right = false;
        actor_intent[num].crouch = false;
        break;
      case 5:
        actor_intent[num].left = true;
        actor_intent[num].crouch = false;
        break;
      case 6:
        actor_intent[num].right = true;
        actor_intent[num].crouch = false;
        break;
      case 7:
        actor_intent[num].fast_fall = true;
        actor_intent[num].crouch = true;
        break;
      case 8:
      case 9:
        actor_intent[num].crouch = true;
        actor_intent[num].left = false;
        actor_intent[num].right = false;
        break;
      case 10:
        actor_intent[num].fast_fall = false;
        actor_intent[num].crouch = false;
        break;
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
        // save when falling off
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
  actor_state[num].current_action=ACTION_STAND_BY_GROUND;
}


char clock=0;
void __fastcall__ irq_nmi_callback(void) 
{
  
  print_state(0,NTADR_A(1,27));
  //print_state(1,NTADR_A(10,27));
  //__asm__("lda #$00");
  //__asm__("sta $2005");
  //__asm__("lda #$ff");
  //__asm__("sta $2005");
  
  
  //PPU.control=0x03;
  //scroll(0,0);

}


// main function, run after console reset
void main(void) {
  unsigned char newclock=0;
  unsigned char simulate_i=0;
  char pad = 0;
  char last_pad = 0;
  char pad_rising = 0;
  char pad_falling =0;
  

  bool demo_mode_on = true;
  
  // set background palette colors
  pal_all(PALETTE);

  //initialize_player(0,0,54+10,143);  
  initialize_player(0,0,128,99);
  
  #if NUM_ACTORS>1
  initialize_player(1,0,128,99);
  #endif
  //for(;;)
  #if NUM_ACTORS>2
  initialize_player(2,0,128,99);
  #endif
  #if NUM_ACTORS>3
  initialize_player(3,0,128,99);
  #endif
  
  // Draw bg and set platforms data.
  reset_level_and_bg();
  //
  if (demo_mode_on)
  {
    vram_adr(NTADR_A(1,26));
    vram_write("Press START to stop Demo mode.", 30);
  }

  nmi_set_callback(irq_nmi_callback);
  // enable PPU rendering (turn on screen)
  ppu_on_all();

  // infinite loop
  while (1) {
    unsigned char i, j; // actor index
    unsigned char oam_id; // sprite ID
    
    
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
      num_ai=NUM_ACTORS;
      simulate_player(simulate_i);
      simulate_i+=1;
      if(simulate_i>NUM_ACTORS-1)
      {
        simulate_i=0;
      }
    
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
      
      // Jump / cancel jump when state changes. Let simulation update consume intent in between.
      if(pad_rising & PAD_A)
      {
      	actor_intent[0].jump = true;
      }
      if((pad & PAD_A )==false && actor_intent[0].jump==true)
      {
      	actor_intent[0].jump = false;
      }

      actor_intent[0].fast_fall = pad & PAD_DOWN?true:false;
      actor_intent[0].crouch = pad & PAD_DOWN?true:false;

      num_ai=NUM_ACTORS-1;
      if(num_ai>0)
      {
      simulate_player(simulate_i+1);
      simulate_i+=1;
      if(simulate_i>NUM_ACTORS-2)
      {
        simulate_i=0;
      }
      }
    }
    
      
    // Actor State and intent physics
    for (i=0; i<NUM_ACTORS; i++) 
    {
      enum action_state cur_action;
      bool on_ground=false;
      byte action_frames=0;
      bool on_edge=false;
      short int actorxf;
      short int actoryf;
      
      a_state=&actor_state[i];
      a_intent=&actor_intent[i];
      a_params=&actor_params[i];
      a_speed_x=&actor_speedx[i];
      a_speed_y=&actor_speedy[i];
      
      cur_action=a_state->current_action;
      action_frames=a_state->current_action_frames;
      on_ground=ON_GROUND(cur_action);
      
      
      if (!on_ground) // on air
      {
        // Reset crouch intent on air.
        a_intent->crouch = false;
        // Fall speed
      	*a_speed_y +=a_params->fall_force; 
        
        if(a_intent->fast_fall && *a_speed_y>0)
        {
          *a_speed_y = a_params->fast_fall;
        }
        else
        {
          *a_speed_y = MIN(*a_speed_y,a_params->fall_limit);
        }
        // jump
        if(a_intent->jump)
        {
          if(a_state->double_jumps_left>0)
          {
            *a_speed_y = -a_params->jump_force; 
            a_state->double_jumps_left-=1;
          }
          a_intent->jump = false;
        }
        
        if(cur_action==ACTION_STAND_BY_AIR)
        {
          if(a_intent->left)
            {
              *a_speed_x=-a_params->run_speed;
            }
            else if(a_intent->right)
            {
              *a_speed_x=a_params->run_speed;
            }
          // todo:skidding
          // todo:dashing
        }
      }
      else // on ground
      {
        // Reset fast fall intent on ground. Todo:move after input.
        a_intent->fast_fall=false;
        
        if(cur_action==ACTION_STAND_BY_GROUND)
        {
          if(a_intent->left || a_intent->right)
          {
            cur_action=ACTION_WALKING_GROUND;
            action_frames=0;
          }
        }
        if(cur_action==ACTION_WALKING_GROUND)
        {
          if(action_frames>a_params->frames_to_run)
          {
            cur_action=ACTION_RUNNING_GROUND;
            action_frames=0;
          }
          else
          {
            
            if(a_state->direction_changed || *a_speed_x==0)
            {
              action_frames=0;
            }
            if(a_intent->left)
            {
              *a_speed_x=-a_params->walk_speed;
            }
            else if(a_intent->right)
            {
              *a_speed_x=a_params->walk_speed;
            }
          }
        }
        if(cur_action==ACTION_RUNNING_GROUND)
        {
          if(a_intent->left)
          {
            *a_speed_x=-a_params->run_speed;
          }
          else if(a_intent->right)
          {
            *a_speed_x=a_params->run_speed;
          }
          else
          {
            // todo:skidding
            // todo:dashing
            cur_action=ACTION_STAND_BY_GROUND;
            action_frames=0;
          }
        }
          
        // Todo:limit start states for crouching. Move inside above ifs.
        if(a_intent->crouch 
           && cur_action!=ACTION_CROUCHING_GROUND)
        {
          cur_action=ACTION_CROUCHING_GROUND;
          action_frames=0;
        }
        if(cur_action==ACTION_CROUCHING_GROUND
          && a_intent->crouch!=true)
        {
          cur_action=ACTION_STAND_BY_GROUND;
          action_frames=0;
        }
        // Always reset double jump frames on ground.
        // todo: move to state transitions off from ground.
        a_state->double_jumps_left=1;
          
        if(a_intent->jump)
        {
          if(cur_action!=ACTION_CROUCHING_TO_JUMP_GROUND)
          {
            cur_action=ACTION_CROUCHING_TO_JUMP_GROUND;
            action_frames=0;
          }
          // crouch always unconditionally here.
          if (action_frames>=a_params->jump_crouch_frames)
          {
            // Do normal jump
            *a_speed_y = -a_params->jump_force;
            a_intent->jump = false;
            cur_action = ACTION_STAND_BY_AIR;
            action_frames=0;
          }
        }
        // Do short jump when cancelling jump early.
        else if (cur_action==ACTION_CROUCHING_TO_JUMP_GROUND)
        {
          *a_speed_y = -a_params->short_hop_force;
          a_intent->jump = false;
          cur_action = ACTION_STAND_BY_AIR;
          action_frames=0;
        }
        // Note: state might not be on ground anymore.
      }
      
      // Inertia
      // TODO: make state specific.
      *a_speed_x= *a_speed_x*4/5;
      a_state->current_action=cur_action;
      ++action_frames;
      a_state->current_action_frames=MIN(action_frames,255);
        
    //}
    
    // Actor Physics
    
    //for (i=0; i<NUM_ACTORS; i++) {
      {
      
      byte speed_y_in_pixels;
      bool action_on_ground=false;
      unsigned char sprite_var;
      speed_y_in_pixels=(*a_speed_y>>8);
      
      //a_state=&actor_state[i];
      actorxf = actor_xf[i]+*a_speed_x;
      actoryf = actor_yf[i]+*a_speed_y;
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
      
      // Collisions and related calculation
      
      on_ground = false;
      on_edge = false;
      for(j=0;j<p_count;++j)
      {
        bool falling=false;
        bool on_platform=false;
        byte actor_feet_x;
        byte actor_feet_y;
        bool skip_due_to_fall_through;
        cur_platform=&platforms[j];
        actor_feet_x=actor_x[i]+8;
        actor_feet_y=actor_y[i]+17;
        skip_due_to_fall_through=(a_state->current_action==ACTION_CROUCHING_GROUND && cur_platform->can_fall_through);
        falling=actor_speedy[i] >= 0;
        /*if(cur_platform->has_edge)
        {
          byte grab_box_x1;
          byte grab_box_x2;
          byte grab_box_y;
          grab_box_x1=actor_feet_x-6;
          grab_box_x2=actor_feet_x+6;
          grab_box_y=actor_y[i]+1;
          //todo:take direction into account
          //todo: collide with edge
          if(actor_feet_y>=cur_platform->y1
           && actor_feet_y<=cur_platform->y2
           && actor_feet_x>cur_platform->x1
           && actor_feet_x<cur_platform->x2
            )
          {
            //todo: grab
          }
        }*/
        // normal collision
        on_platform=
           actor_feet_y>=cur_platform->y1-speed_y_in_pixels
           && actor_feet_y<=cur_platform->y2
           && actor_feet_x>cur_platform->x1
           && actor_feet_x<cur_platform->x2;
        if(falling
           && on_platform
           && !skip_due_to_fall_through
          )
        {
          actor_y[i] = cur_platform->y1-17;
          actor_speedy[i] = 0;
          actor_yf[i] = 0;
          on_ground = true;
        }
        // todo: split condition to improve perf
        // on_edge
        if(on_platform
           && ((actor_feet_x<cur_platform->x1+16 && a_state->moving_left) 
               || (actor_feet_x>cur_platform->x2-16&& a_state->moving_right))
           )
        {
          on_edge=true;
        }
      }
      a_state->on_edge=on_edge;
      
      
      // Fix action based on physical on_ground state.
      action_on_ground = ON_GROUND(a_state->current_action);
      if(!on_ground && action_on_ground) 
      { // fall off edge
        a_state->current_action = ACTION_STAND_BY_AIR;
        a_state->current_action_frames=0;
        action_on_ground=false;
      }
      else if(on_ground && !action_on_ground)
      { // fall on ground
        a_state->current_action = ACTION_STAND_BY_GROUND;
        a_state->current_action_frames=0;
        action_on_ground=true;
      }
      
      // Current action may have been invalidated.
      cur_action=a_state->current_action;
      
      // Moving left/right
      a_state->direction_changed = false;
      if(*a_speed_x>0)
      {
        if(a_state->moving_right==false)
        {
          a_state->direction_changed = true;
        }
        a_state->moving_right=true;
        a_state->moving_left=false;
      }
      else if(*a_speed_x<0)
      {
        if(a_state->moving_left==false)
        {
          a_state->direction_changed = true;
        }
        a_state->moving_left=true;
        a_state->moving_right=false;
      }
      else
      {
        a_state->moving_right=false;
        a_state->moving_left=false;
      }
      
      // Select sprite
      // TODO: deduplicate
      // todo: improve facing difection.
      if(actor_speedx[i]>0)//right
      {
        sprite_var=i;
      }
      else
      {
        sprite_var=i+4;
      }
      if(action_on_ground)
      {
        if(cur_action==ACTION_RUNNING_GROUND)
        {
          actor_sprite[i] = char1run_sprites[sprite_var];
        }
        else if(cur_action==ACTION_CROUCHING_GROUND)
        {
          actor_sprite[i] = char1crouch_sprites[sprite_var];
        }
        else if(cur_action==ACTION_STAND_BY_GROUND 
                || cur_action==ACTION_WALKING_GROUND)
        {
          if(a_state->on_edge)
          {
            actor_sprite[i] = char1sway_sprites[sprite_var];
          }
          else
          {
            actor_sprite[i] = char1stand_sprites[sprite_var];
          }
        }
        else
        {
          // Todo: handle jump crouch as part of animation instead of last case
          actor_sprite[i] = char1crouch_sprites[sprite_var];
        }
      }
      else
      {
        if(actor_intent[i].fast_fall) // Todo: use state instead of intent.
        {
          actor_sprite[i] = char1fast_fall_sprites[sprite_var];
        }
        else
        {
          actor_sprite[i] = char1jump_sprites[sprite_var];
        }
      }
      }
    }
    
    
    // Update Sprites
    {
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
    }
    // wait for next frame
    {
      // loop to count extra time in frame
      {
        int i;
        for(i=0;i<0;++i)
        {
        }
      }
      ppu_wait_nmi();
    }
    
    
    // detect frame drops
    {
      PPU.mask =0x0;

      newclock = nesclock();
      if(newclock-clock>1)
      {
        // Todo: replace by PPU accessor.
        __asm__("lda $2002");
        __asm__("lda #$3F");
        __asm__("sta $2006");
        __asm__("lda #$00");
        __asm__("sta $2006");
        __asm__("lda #$16");
        __asm__("sta $2007");
        //pal_col(0,0x16);

      }
      else
      {
        __asm__("bit $2002");
        PPU.vram.address =0x3f;
        PPU.vram.address =0x00;
        //if(clock&0x1)
        {
          //PPU.vram.data = 0x0c;
        }
        //else
        {
          PPU.vram.data = 0x1c;
        }
      }
      clock=newclock;

      PPU.control=0b11000000;
      PPU.scroll=0x00;
      PPU.scroll=0x02;
      PPU.mask =0b00011110;
    }
    

  }
}
