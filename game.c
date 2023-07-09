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
//#define Assert(cond) if(cond)for(;;);
#define Assert(cond) ;

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
// !edge grab (day 5)
// /dash attack animation (day 5.5)
// *double tap dash
// /dash attack
// *more clear attack/animation state 
// !crouch
// !better fall through control
// *crouch dash cancel
// *running inertia
// *Implement target speed and acceleration per action
// *dash dancing support
// *dash attack
// !ai to avoid falling off(day 3)
// *neutral attack (day 5.5)
// *clarify states and logic more(enums and masks)
// *support coordinates outside of screen
// *KO on arena edges, spawning
// !make opposite directions mutually exclusive (day 5)
// !keep last facing direction when stopping (day 5)
// !Fix fast fall falling through (day 5)
// !optimize platform iteration and access (day 3)
// !warn on frame drops (day 3)
// !optimize player iteration(day 2.5)
// !optimize intent access(day 3.5)
// !optimize physics iteration(day 3.5)
// !add sprite for edge sway (day 3)
// !support edge sway (day 5)
// !add debug print (day 4)
// !optimize debug print
// !add frame drop detection (day 3.5)
// !add vegetation (day 4)
// !add bg decoration (day 4)
// !fix failing jump and add asserts (day 4/5)
// !fix fall through (day 5)
// *fix perf of indexing spritess
// *fix bug where green stops when yellow hangs

DEF_METASPRITE_2x2_VARS(char1icon,0xd0);
DEF_METASPRITE_2x2_VARS(char13lives,0xac);

DEF_METASPRITE_2x2_VARS(char1neutral,0xd4);
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
  ACTION_DASHING_GROUND=7, // Todo: Refactor to "Animated attack"
  ACTION_TURNING_AROUND_GROUND=8, // todo
  ACTION_STOPPING_GROUND=9, // todo
  ACTION_FAST_FALLING_AIR=11, 
};

unsigned char action_state_to_char[14] = {'S','A','c','C','H','W','R','D','T','s','F'};

#define ON_GROUND(state) (((state)!=ACTION_STAND_BY_AIR)&&((state)!=ACTION_FAST_FALLING_AIR))

enum dir
{
  DIR_LEFT = 0,
  DIR_RIGHT = 1,
  DIR_NONE = 2,
};

unsigned char dir_to_char[3] = {'L','R','.'};

enum attack_type
{
  ATTACK_NONE=0,
  ATTACK_NORMAL_LEFT=1,
  ATTACK_NORMAL_RIGHT=2,
  ATTACK_DASH=3,
};


struct state{
  enum action_state current_action;
  byte damage;
  //bool on_ground;
  enum dir moving_dir;
  enum dir facing_dir;
  enum attack_type current_attack;
  bool direction_changed;
  // hanging
  //byte movement_hold_frames; // for crouch canceling dash, move to params
  //byte attack_hold_frames;   // for attacks
  //byte running;
  byte current_action_frames;
  byte current_attack_frames_left;
  byte double_jumps_left;
  bool on_edge;
  bool fall_through_triggered;
  byte damage_vis_frames;
};


struct intent{
  enum dir dir;
  bool jump;

  bool crouch;
  bool dash;
  bool fast_fall;
  bool attack;
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
  byte attack_neutral_frames;
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

#define NUM_ACTORS 3
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
void ** a_sprite;
void ** a_icon;
#pragma bss-name (pop)
#pragma data-name(pop)

struct vram_inst
{
  unsigned char _0_lda_opcode_A9;
  unsigned char _1_lda_val;
  unsigned char _2_sta_opcode_8D;
  unsigned char _3_sta_addr_07;
  unsigned char _4_sta_addr_20;
};


#define vram_line_len 32

struct vram_inst vram_line[vram_line_len];
struct vram_inst vram_line2[vram_line_len];

byte icon_pos_x[]={16,72,128,184};

void init_vram_line(struct vram_inst* inst)
{
  byte i;
  for(i=0;i<vram_line_len-1;i++)
  {
    inst->_0_lda_opcode_A9 = 0xA9;
    //inst->_1_lda_val=0;// skip setting data byte.
    inst->_2_sta_opcode_8D = 0x8D;
    inst->_3_sta_addr_07 = 0x07;
    inst->_4_sta_addr_20 = 0x20;
    inst++;
  }
  inst->_0_lda_opcode_A9 = 0x60; // RTS instead of next instr.
}


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

void update_player_status(struct vram_inst* inst)
{
  byte damage = a_state->damage;
  // TODO: move to setting damage.
  if((damage&0b00001111)>=10)
  {
    damage+=6;
    a_state->damage=damage;
  }
  inst->_1_lda_val = '0'+(damage>>4);
  inst++;
  inst->_1_lda_val = '0'+(damage&0b00001111);
  inst++;
  inst->_1_lda_val = '%';
}


void update_debug_info(byte player,struct vram_inst* inst)
{
  enum action_state cur_action=actor_state[player].current_action;

  inst->_1_lda_val = action_state_to_char[cur_action];
  inst++;
  
  inst->_1_lda_val = 0x30+cur_action;
  inst++;
  
  a_intent=&actor_intent[player];
  
  inst->_1_lda_val = dir_to_char[a_intent->dir];
  inst++;
  
  if(a_intent->jump)
  {
    inst->_1_lda_val = 'J';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  inst++;
  
  if(a_intent->crouch)
  {
    inst->_1_lda_val = 'C';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  inst++;
  
  if(a_intent->fast_fall)
  {
    inst->_1_lda_val = 'F';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  
}

char num_ai;

void simulate_player(unsigned char num)
{
  // TODO: optimize perf
  unsigned int r = rand();
  signed char id_under =-1;
  signed char id_right=-1;
  signed char id_left=-1;
  
  
  unsigned char r128;
  unsigned char j;

  Assert(num>NUM_ACTORS);

  a_intent=&actor_intent[num];
  
  // Balance chances based on amount of ai's
  // Todo: convert to switch-case for a tiny bit better perf.
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
  cur_platform=platforms;
  for(j=0;j<p_count;++j)
  {
    bool isLeft = actor_x[num]+8>cur_platform->x1;
    bool isRight = actor_x[num]+8<cur_platform->x2;

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
    cur_platform++;
  }
  {
    switch(r128)
    {
      case 0:
        break;
      case 1: // Jump
        a_intent->jump = true;
        a_intent->crouch = false;
        break;
      case 2:
      case 3: // Release Jump
        a_intent->jump = false;
        break;
      case 4: // Stop
        a_intent->dir = DIR_NONE;
        a_intent->crouch = false;
        break;
      case 5: // Left
        a_intent->dir = DIR_LEFT;
        a_intent->crouch = false;
        break;
      case 6: // Right
        a_intent->dir = DIR_RIGHT;
        a_intent->crouch = false;
        break;
      case 7: // Fast fall / crouch;
        a_intent->fast_fall = true;
        a_intent->crouch = true;
        break;
      case 8:
      case 9: // crouch still
        a_intent->crouch = true;
        a_intent->dir = DIR_NONE;
        break;
      case 10: // Cancel Fast fall / crouch
        a_intent->fast_fall = false;
        a_intent->crouch = false;
        break;
      case 11:
      case 12:
        a_intent->attack = true;
        break;
      case 13:
      case 14:
      case 15:
      case 16:
      case 17: // save when falling off. cost: 3 scanlines.
        if(id_under==-1)
        {
          if(id_left!=-1)
          {
            a_intent->dir = DIR_LEFT;
          }
          if(id_right!=-1)
          {
            a_intent->dir = DIR_RIGHT;
          }
          a_intent->fast_fall=false;
          if(actor_speedy[num]>0)
          {
            a_intent->jump=true;
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
      actor_params[num].dash_frames = 15;       // Z (M)
      actor_params[num].double_jumps = 1;       // 1 (All)
      actor_params[num].jump_crouch_frames = 6; // 6 (M)
      actor_params[num].attack_neutral_frames = 6; // try something
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
  vram_adr(NTADR_A(1,26));
  __asm__("jsr %v",vram_line);
  //vram_adr(NTADR_A(1,26));
  vram_adr(NTADR_A(1,25));
  __asm__("jsr %v",vram_line2);
  //print_state(0,NTADR_A(1,27));

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

  //
  // INIT
  //
  
  init_vram_line(vram_line);
  init_vram_line(vram_line2);
  
  // set background palette colors
  pal_all(PALETTE);

  initialize_player(0,0,54+10,143);  
  //initialize_player(0,0,128,99);
  
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
    vram_adr(NTADR_A(1,27));
    vram_write("Press START to stop Demo mode.", 30);
  }

  nmi_set_callback(irq_nmi_callback);
  // enable PPU rendering (turn on screen)
  ppu_on_all();

  //
  // MAIN LOOP
  //
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
      if(pad & PAD_LEFT && !(pad & PAD_RIGHT))
      {
          actor_intent[0].dir = DIR_LEFT;
      }
      else if(pad & PAD_RIGHT)
      {
          actor_intent[0].dir = DIR_RIGHT;
      }
      else
      {
      	actor_intent[0].dir = DIR_NONE;
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
      if(pad_rising & PAD_DOWN)
      {
      	actor_intent[0].crouch = true;
      }
      else if (pad_falling & PAD_DOWN)
      {
        actor_intent[0].crouch = false;
      }
      
      if(pad_rising & PAD_B)
      {
        actor_intent[0].attack = true;
      }

      num_ai=NUM_ACTORS-1;
      if(num_ai>0)
      {
        simulate_player(simulate_i+1);
        simulate_i+=1;
        #if NUM_ACTORS >1
        if(simulate_i>NUM_ACTORS-2)
        #endif
        {
          simulate_i=0;
        }
        }
    }
    
    //
    // SIMULATION LOOP
    //
    
    // Reset pointers for first iteration. This is faster than indexing.
    a_state=actor_state;
    a_intent=actor_intent;
    a_params=actor_params;
    a_speed_x=actor_speedx;
    a_speed_y=actor_speedy;

    // Actor State and intent physics
    for (i=0; i<NUM_ACTORS; i++) 
    {
      enum action_state cur_action;
      bool on_ground=false;
      byte action_frames=0;
      
      cur_action=a_state->current_action;
      action_frames=a_state->current_action_frames;
      on_ground=ON_GROUND(cur_action);
      
      if(a_state->current_attack_frames_left>0)
      {
      	a_state->current_attack_frames_left--;
      }
      else
      {
        a_state->current_attack = ATTACK_NONE;
      }
      if(a_state->damage_vis_frames>0)
      {
      	a_state->damage_vis_frames--;
      }
      
      
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
          if(a_intent->dir == DIR_LEFT)
          {
            *a_speed_x=-a_params->run_speed;
          }
          else if(a_intent->dir == DIR_RIGHT)
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
        
        if(a_intent->attack && cur_action != ACTION_DASHING_GROUND) // todo: switch to check animated/cancelable attack
        {
          *a_speed_x = 0;
          if(a_state->facing_dir == DIR_RIGHT)
          {
            a_state->current_attack = ATTACK_NORMAL_RIGHT;
          } else
          {
            a_state->current_attack = ATTACK_NORMAL_LEFT;
          }
          a_state->current_attack_frames_left = a_params->attack_neutral_frames;
          cur_action = ACTION_STAND_BY_GROUND;
        }
        
        if(cur_action==ACTION_STAND_BY_GROUND)
        {
          if(a_intent->dir != DIR_NONE)
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
            if(a_intent->dir == DIR_LEFT)
            {
              *a_speed_x=-a_params->walk_speed;
            }
            else if(a_intent->dir == DIR_RIGHT)
            {
              *a_speed_x=a_params->walk_speed;
            }
          }
        }
        if(cur_action==ACTION_RUNNING_GROUND)
        {
          if(a_intent->attack)
          {
            cur_action=ACTION_DASHING_GROUND;
            action_frames=0;
          }
          else if(a_intent->dir == DIR_LEFT)
          {
            *a_speed_x=-a_params->run_speed;
          }
          else if(a_intent->dir == DIR_RIGHT)
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
        if(cur_action==ACTION_DASHING_GROUND)
        {
          if(a_state->moving_dir == DIR_LEFT)
          {
            *a_speed_x=-a_params->dash_speed;
          }
          else if(a_state->moving_dir == DIR_RIGHT)
          {
            *a_speed_x=a_params->dash_speed;
          }
          
          if(action_frames>a_params->dash_frames)
          {
            // todo:skidding
            // todo:dashing
            if(a_intent->dir == a_state->moving_dir)
            {
              cur_action=ACTION_RUNNING_GROUND;
            }
            else
            {
              cur_action=ACTION_STAND_BY_GROUND;
            }
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
      
      // Inertia when slowing down
      // TODO: make state specific.
      // TODO: Implement target speed instead of only targeting to 0
      if(a_intent->dir == DIR_NONE)
      {
        if(*a_speed_x>0)
        {
     	  *a_speed_x= *a_speed_x-20;//*a_speed_x*4/5;
          *a_speed_x= MAX(0,*a_speed_x);
        }
        else if(*a_speed_x<0)
        {
          *a_speed_x= *a_speed_x+20;
          *a_speed_x= MIN(0,*a_speed_x);
        }
      }
      a_intent->attack = false;
      a_state->current_action=cur_action;
      ++action_frames;
      a_state->current_action_frames=MIN(action_frames,255);
      
      if(i<NUM_ACTORS)
      {
        a_state++;
        a_intent++;
        a_params++;
        a_speed_x++;
        a_speed_y++;
      }
    }
    a_state=actor_state;
    a_intent=actor_intent;
    a_params=actor_params;
    a_speed_x=actor_speedx;
    a_speed_y=actor_speedy;

    // Precalc hitboxes of the frame
    // todo:
    for (i=0; i<NUM_ACTORS; i++) 
    {
      if(i<NUM_ACTORS)
      {
        a_state++;
        a_intent++;
        a_params++;
        a_speed_x++;
        a_speed_y++;
      }
    }
    
    // Actor State and intent physics
    
    a_state=actor_state;
    a_intent=actor_intent;
    a_params=actor_params;
    a_speed_x=actor_speedx;
    a_speed_y=actor_speedy;

    // Actor State and intent physics
    for (i=0; i<NUM_ACTORS; i++) 
    {
      bool on_ground=false;
      enum action_state cur_action=a_state->current_action;
      
      
              
      // Process attacks
      {
        unsigned char k;
        for(k = 0; k<NUM_ACTORS;k++)
        {
          struct state* o_state;
          if(k==i)
            continue;
          o_state = &actor_state[k];
          //struct intent* o_intent = &actor_intent[k];
          if(o_state->current_attack != ATTACK_NONE)
          {
            // TODO: precalc hitboxes.
            byte attack_y1;
            byte attack_y2;
            byte attack_x1;
            byte attack_x2;
            short int force_x;
            attack_y1=actor_y[k];
            attack_y2=attack_y1+6;
            if(actor_y[j]<attack_y2
              && actor_y[i]+17 > attack_y1
              )
            {
              
              if (o_state->current_attack == ATTACK_NORMAL_RIGHT)
              {
	        attack_x1=actor_x[k]+10;
                attack_x2=actor_x[k]+18;
                force_x=3;
              }
              else if (o_state->current_attack == ATTACK_NORMAL_LEFT)
              {
                attack_x1=actor_x[k]-2;
                attack_x2=actor_x[k]+6;
                force_x=-3;
              }
              if(actor_x[i]+12>attack_x1
                 && actor_x[i]+4<attack_x2
                )
              {
                actor_speedy[i]=-(50+(a_state->damage<<3));
                actor_x[i]=(short int)actor_x[i]+force_x;
                actor_speedx[i]=force_x<<2;
                a_state->damage+=3;
                a_state->damage_vis_frames+=3;
              }
            }
          }
        }
      }
    
      
      // Actor Physics
      {
        short int actorxf;
        short int actoryf;
        bool on_edge;
        bool action_on_ground=false;
        byte speed_y_in_pixels=(*a_speed_y>>8);


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
          skip_due_to_fall_through=((a_state->current_action==ACTION_CROUCHING_GROUND || a_state->fall_through_triggered) && cur_platform->can_fall_through);
          falling=actor_speedy[i] >= 0;
          
          
          // Side collision and grab
          if(!on_platform && cur_platform->has_edge)
          {
            byte grab_box_x1;
            byte grab_box_x2;
            byte grab_box_y;
            
            // collision to edge
            if(actor_y[i]<cur_platform->y2 
               && actor_feet_y>cur_platform->y1
              )
            {
              if(actor_feet_x>cur_platform->x1
                && actor_feet_x<cur_platform->x1+8)
              {
              	actor_x[i]=cur_platform->x1-8;
              } 
              else if (actor_feet_x<cur_platform->x2
                       && actor_feet_x>cur_platform->x2-8)
              {
                actor_x[i]=cur_platform->x2-8;
              }
            }
            
            // grab

            grab_box_x1=actor_feet_x-8;
            grab_box_x2=actor_feet_x+8;
            grab_box_y=actor_y[i];
            //todo:take direction into account
            //todo: collide with edge
            if(falling 
               && !a_intent->jump 
               && !a_intent->fast_fall
               && !a_intent->crouch
               && grab_box_y>=cur_platform->y1
               && grab_box_y<=cur_platform->y2)
            {
              if(a_intent->dir!=DIR_LEFT
                 && grab_box_x2>cur_platform->x1
                 && grab_box_x1<cur_platform->x1
                )
              {
                a_speed_x[i]=0;a_speed_y[i]=0;
                actor_y[i]=cur_platform->y1;
                actor_x[i]=cur_platform->x1-11;
                a_state->current_action = ACTION_HANGING_GROUND;
                a_state->facing_dir = DIR_RIGHT;
                on_ground = true;
              }
              else if(a_intent->dir!=DIR_RIGHT
                 && grab_box_x2>cur_platform->x2
                 && grab_box_x1<cur_platform->x2)
              {
                a_speed_x[i]=0;a_speed_y[i]=0;
                actor_y[i]=cur_platform->y1;
                actor_x[i]=cur_platform->x2-5;
                a_state->current_action = ACTION_HANGING_GROUND;
                a_state->facing_dir = DIR_LEFT;
                on_ground = true;
              }
            }
            
          }
          // normal collision
          on_platform=
             actor_feet_y>=cur_platform->y1-speed_y_in_pixels
             && actor_feet_y<=cur_platform->y2
             && actor_feet_x>cur_platform->x1
             && actor_feet_x<cur_platform->x2;
          if(falling
             && on_platform
             )
          {
            if(skip_due_to_fall_through)
            {
              a_state->fall_through_triggered = true;
            }
            else
            {
              actor_y[i] = cur_platform->y1-17;
              actor_speedy[i] = 0;
              actor_yf[i] = 0;
              on_ground = true;
            }
          }
          // todo: split condition to improve perf
          // on_edge
          if(on_platform
             && ((actor_feet_x<cur_platform->x1+6 && a_state->facing_dir == DIR_LEFT) 
                 || (actor_feet_x>cur_platform->x2-6&& a_state->facing_dir == DIR_RIGHT))
             )
          {
            on_edge=true;
          }
          
        }
        a_state->on_edge=on_edge;

      	if(on_ground)
        {
          a_state->fall_through_triggered = false;
        }

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

        // Current action may have been invalidated. Set it again.
        // TODO: Simplify.
        cur_action=a_state->current_action;

        // Moving left/right
        a_state->direction_changed = false;
        if(*a_speed_x>0)
        {
          if(a_state->moving_dir == DIR_LEFT)
          {
            a_state->direction_changed = true;
          }
          a_state->moving_dir = DIR_RIGHT;
          a_state->facing_dir = DIR_RIGHT;
        }
        else if(*a_speed_x<0)
        {
          if(a_state->moving_dir == DIR_RIGHT)
          {
            a_state->direction_changed = true;
          }
          a_state->moving_dir = DIR_LEFT;
          a_state->facing_dir = DIR_LEFT;
        }
        else
        {
          a_state->moving_dir = DIR_NONE;
        }

        // Select sprite
        // TODO: deduplicate
        // todo: improve facing difection.
        {         
          unsigned char sprite_var;
          if(a_state->facing_dir == DIR_RIGHT)//right
          {
            sprite_var=i;
          }
          else
          {
            sprite_var=i+4;
          }
          // TODO: Implement LUT instead of a lot of ifs.
          if(action_on_ground)
          {
            if(a_state->current_attack != ATTACK_NONE)
            {
              actor_sprite[i] = char1neutral_sprites[sprite_var];
            }
            else if(cur_action==ACTION_RUNNING_GROUND)
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
            else if(cur_action==ACTION_HANGING_GROUND)
            {
              actor_sprite[i] = char1ledge_sprites[sprite_var];
            }
            else if(cur_action==ACTION_DASHING_GROUND)
            {
              actor_sprite[i] = char1dash_sprites[sprite_var];
            }
            else
            {
              // Todo: handle jump crouch as part of animation instead of last case
              actor_sprite[i] = char1crouch_sprites[sprite_var];
            }
          }
          else
          {
            if(a_intent->fast_fall) // Todo: use state instead of intent.
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
      
      
      
      if(i<NUM_ACTORS)
      {
        a_state++;
        a_intent++;
        a_params++;
        a_speed_x++;
        a_speed_y++;
      }
    }
    
    
    // Update Sprites
    {
      a_state=actor_state;
      a_sprite=actor_sprite;
      // start with OAMid/sprite 0
      oam_id = 0;
      // draw and move all actors
      for (i=0; i<NUM_ACTORS; i++) 
      {
        // TODO: add camera
        oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, *a_sprite);
        a_sprite++;
      }
      
      a_icon=char1icon_sprites;
      for (i=0; i<NUM_ACTORS; i++) 
      {
        byte x=icon_pos_x[i];
        oam_id = oam_meta_spr(x+(a_state->damage_vis_frames>>2), 190-(a_state->damage_vis_frames), oam_id, *a_icon);
        // Todo: enable health indicator after refactoring sprite rendering.
        //oam_id = oam_meta_spr(x+3, 200, oam_id, char13lives_sprites[i]);
        a_icon++;
        a_state++;
      }
      // hide rest of sprites
      // if we haven't wrapped oam_id around to 0
      if (oam_id!=0) oam_hide_rest(oam_id);
    }
    
    {
      a_state=actor_state;
      //update_debug_info(0,vram_line);
      update_player_status(vram_line2+4);

      #if NUM_ACTORS >1
      a_state++;
      //update_debug_info(1,vram_line+8);
      update_player_status(vram_line2+11);
      #endif

      #if NUM_ACTORS >2
      a_state++;
      //update_debug_info(2,vram_line+16);
      update_player_status(vram_line2+18);
      #endif

      #if NUM_ACTORS >3
      a_state++;
      //update_debug_info(3,vram_line+24);
      //update_player_status(vram_line2+25);
      #endif
    }
    
    
    
    // wait for next frame
    {
      // loop to count extra time in frame
      {
        //int i;
        //for(i=0;i<0;++i)
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
