/*
Setting th e attribute table, which controls palette selection
for the nametable. We copy it from an array in ROM to video RAM.
*/
#include "neslib.h"
#include <string.h>
#include <stdlib.h>

// include CC65 NES Header (PPU)
#include <nes.h>

#include "apu.h"

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

#define DEF_METASPRITE_1x1_VARS(name,code)\
DEF_METASPRITE_1x1(name##_a,code,0)\
DEF_METASPRITE_1x1(name##_b,code,1)\
DEF_METASPRITE_1x1(name##_c,code,2)\
DEF_METASPRITE_1x1(name##_d,code,3)\
DEF_METASPRITE_1x1_FLIP(name##_a2,code,0)\
DEF_METASPRITE_1x1_FLIP(name##_b2,code,1)\
DEF_METASPRITE_1x1_FLIP(name##_c2,code,2)\
DEF_METASPRITE_1x1_FLIP(name##_d2,code,3)\
void* name##_sprites[]={\
  &name##_a\
  ,&name##_b\
  ,&name##_c\
  ,&name##_d\
  ,&name##_a2\
  ,&name##_b2\
  ,&name##_c2\
  ,&name##_d2};


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
  

#define DEF_METASPRITE_1x1(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   pal, \
        128};
#define DEF_METASPRITE_1x1_FLIP(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        128};

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
// !crouch
// !better fall through control
// !running inertia
// !ai to avoid falling off(day 3)
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
// !neutral attack (day 5.5)
// !fix bug where green stops when yellow hangs (day 7)
// !implement target speed (day 7.5)
// !optimized text rendering (day 6)
// !implement air kick (day 8)
// *implement hitlag 
// *control hitbox effect during attack(active frames, damage amount) 
// *advanced knockback calculation https://www.ssbwiki.com/Knockback#Formula
// *hitstun based on knockback
// *respawn invincibility
// *fix perf of indexing sprites
// *clarify states and logic more(enums and masks)
// *support coordinates outside of screen
// !KO on arena edges, spawning
// !counting lives
// *restart after winning
// *acceleration per action
// *dash dancing support
// *dash attack
// *crouch dash cancel
// /dash attack animation (day 5.5)
// *double tap dash
// /dash attack
// *more clear attack/animation state
// *Add a separate state for dead


DEF_METASPRITE_2x2_VARS(AIicon,0xc8);
DEF_METASPRITE_2x2_VARS(char1icon,0xd0);

DEF_METASPRITE_1x1_VARS(char1lives4,0x8c);
DEF_METASPRITE_1x1_VARS(char1lives3,0x8d);
DEF_METASPRITE_1x1_VARS(char1lives2,0x8e);
DEF_METASPRITE_1x1_VARS(char1lives1,0x8f);
DEF_METASPRITE_1x1_VARS(char1lives0,0x8b);

DEF_METASPRITE_2x2_VARS(char1neutral,0xd4);
DEF_METASPRITE_2x2_VARS(char1stand,0xd8);
DEF_METASPRITE_2x2_VARS(char1crouch,0xdc);
DEF_METASPRITE_2x2_VARS(char1run,0xec);
DEF_METASPRITE_2x2_VARS(char1jump,0xe0);
DEF_METASPRITE_2x2_VARS(char1fast_fall,0xe4);
DEF_METASPRITE_2x2_VARS(char1dash,0xe8);
DEF_METASPRITE_2x2_VARS(char1ledge,0xf0);
DEF_METASPRITE_2x2_VARS(char1sway,0xf4);
DEF_METASPRITE_2x2_VARS(char1airneutral,0xf8);

DEF_METASPRITE_1x1_VARS(small_hit,0x80);
DEF_METASPRITE_2x2_VARS(horizontal_explosion,0xb0);
DEF_METASPRITE_2x2_VARS(vertical_explosion,0xb4);

DEF_METASPRITE_1x1_VARS(winner,0x81);


void p(byte type, byte x, byte y, byte len)
{
  vram_adr(NTADR_A(x,y));
  vram_fill(0x90+type, len);
  vram_adr(NTADR_A(x+1,y+1));
  vram_fill(0x05, 1);
  vram_fill(0x09, len-4);
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
  ACTION_SPAWNING=11,
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
  ATTACK_AIR_NEUTRAL_LEFT=4,
  ATTACK_AIR_NEUTRAL_RIGHT=5,
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
  bool isAI;
  byte lives;
  byte wins;
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
  byte attack_air_neutral_frames;
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

enum effect_type
{
  HIT=0,
  EXPLOSION_HORIZONTAL=2,
  EXPLOSION_VERTICAL=3,
  WIN=4,
};

struct effect{
  enum effect_type type;
  byte variant;
  byte x;
  byte y;
  byte frames;
  bool isNew;
};

byte current_effect_index;
struct effect effects[4];


#define NUM_ACTORS 3
#define NUM_PLATFORMS 4

byte actor_x[NUM_ACTORS];      // Position
byte actor_y[NUM_ACTORS];
byte actor_xf[NUM_ACTORS];     // Fraction
byte actor_yf[NUM_ACTORS];
byte actor_prev_x[NUM_ACTORS]; // Perevious pos to track going outside
byte actor_prev_y[NUM_ACTORS];
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
void ** a_iconAI;
byte zp_x;
byte zp_y;
struct intent* intentlookup[NUM_ACTORS];
struct state* o_state;
short int tmp_speed_x_value;
short int tmp_target_speed_x;
enum attack_type tmp_attack_type;
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

static const byte icon_pos_x[]={16+4,72+4,128+4,184+4};

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
  short int damage = a_state->damage;
  // TODO: move to setting damage.
  
  if((damage&0b00001111)>=10)
  {
    damage+=6;
    a_state->damage=damage;
  }
  
  if(damage>=160)
  {
    inst->_1_lda_val = 'D';
    inst++;
    inst->_1_lda_val = 'E';
    inst++;
    inst->_1_lda_val = 'D';
    inst++;
  }
  else
  {
    inst->_1_lda_val = '0'+(damage>>4);
    inst++;
    inst->_1_lda_val = '0'+(damage&0b00001111);
    inst++;
    inst->_1_lda_val = '%';
  }
}

void update_player_wins(struct vram_inst* inst)
{
  short int wins = a_state->wins;
  // TODO: move to setting damage.
  
  if((wins&0b00001111)>=10) // Convert to BCD
  {
    wins+=6;
    a_state->wins=wins;
  }
  
  if(wins==0)
  {
    inst->_1_lda_val = ' ';
    inst++;
    inst->_1_lda_val = ' ';
    inst++;
  }
  else
  {
    inst->_1_lda_val = '0'+(wins>>4);
    inst++;
    inst->_1_lda_val = '0'+(wins&0b00001111);
  }
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

#pragma warn (unused-param, push, off)
#if 1
#define log_state_update(a) ;
#define log_start_physics(a) ;
#define log_precalc_hitboxes(a) ;
#define log_process_attacks(a) ;
#define log_collision_calculation(a) ;
#define log_state_updates_after_sim(a) ;
#define log_sprite_selection(a) ;
#define log_update_sprites(a) ;
#define log_end_physics(a) ;
#else
void log_state_update(byte dummy)
{
  return;
}

void log_precalc_hitboxes(byte dummy)
{
  return;
}

void log_start_physics(byte dummy)
{
  return;
}

void log_process_attacks(byte dummy)
{
  return;
}

void log_collision_calculation(byte dummy)
{
  return;
}

void log_state_updates_after_sim(byte dummy)
{
  return;
}
void log_sprite_selection(byte dummy)
{
  return;
}

void log_update_sprites(byte dummy)
{
  return;
}

void log_end_physics(byte dummy)
{
  return;
}


#endif
#pragma warn(unused-param,pop)

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

  a_intent=intentlookup[num];
  
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

    if(isLeft&isRight)
    {
      bool isUnder;
      isUnder=actor_y[num]+17>=cur_platform->y1;
      if(isUnder)
      {
        id_under=j;
        //todo: make sure it is closest under
      }
      else
      {
        __asm__("nop");
        __asm__("nop");
      }
    }else
    {
      if(isLeft)
      {
        id_left=j;
        //todo: make sure it is closest
      }
      else
      {
        __asm__("nop");
        __asm__("nop");
      }
      if(isRight)
      {
        id_right=j;
        //todo: make sure it is closest
      }
      else
      {
        __asm__("nop");
        __asm__("nop");
      }
      
      // balance branches 
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      __asm__("nop");
      //__asm__("nop");
      //__asm__("nop");
      //__asm__("nop");
     

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
      actor_params[num].attack_air_neutral_frames = 16; // try something
      actor_params[num].fall_force = 22;        // 0.11 (M, Grav)
      actor_params[num].fall_limit = 436;       // 2.13 (M)
      actor_params[num].fast_fall = 614;        // 3 (M)
      break;
  }
  actor_state[num].current_action=ACTION_STAND_BY_GROUND;
  actor_state[num].lives = 4;
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
  char pad2 = 0;
  char last_pad2 = 0;
  char pad_rising2 = 0;
  char pad_falling2 =0;
  bool demo_mode_on = true;
  bool player1joined = false;
  bool player2joined = false;
  register char i;
  
  a_intent=actor_intent;
  for(i =0;i<NUM_ACTORS;i++)
  {
     intentlookup[i]=a_intent;a_intent++;
  }

  //
  // INIT
  //
  
  init_vram_line(vram_line);
  init_vram_line(vram_line2);
  
  // set background palette colors
  pal_all(PALETTE);

  initialize_player(0,0,54+10,143);
  actor_state[0].isAI = true;
  actor_state[0].wins = 0;
  //initialize_player(0,0,128,99);
  
  #if NUM_ACTORS>1
  initialize_player(1,0,128,99);
  actor_state[1].isAI = true;
  actor_state[1].wins = 0;
  #endif
  //for(;;)
  #if NUM_ACTORS>2
  initialize_player(2,0,170,99);
  actor_state[2].isAI = true;
  actor_state[2].wins = 0;
  #endif
  #if NUM_ACTORS>3
  initialize_player(3,0,128,99);
  actor_state[3].isAI = true;
  actor_state[3].wins = 0;
  #endif
  
  // Draw bg and set platforms data.
  reset_level_and_bg();
  //
  if (demo_mode_on)
  {
    vram_adr(NTADR_A(1,27));
    vram_write("Press START to join the game.", 30);
  }

  nmi_set_callback(irq_nmi_callback);
  // enable PPU rendering (turn on screen)
  ppu_on_all();
  APU_ENABLE(ENABLE_PULSE0 | ENABLE_PULSE1 | ENABLE_TRIANGLE | ENABLE_NOISE);

  //
  // MAIN LOOP
  //
  while (1) {
    unsigned char i, j; // actor index
    unsigned char oam_id; // sprite ID
    byte background_color=0x1c;//0x1c;
    bool resetLives=false;
    byte deadCount=0;

    
    APU.pulse[0].control=0xff;
    
    // Controls
    last_pad = pad;
    pad = pad_poll(0);
    pad_rising = pad^last_pad&pad;
    pad_falling = pad^last_pad&last_pad;
    
    last_pad2 = pad2;
    pad2 = pad_poll(1);
    pad_rising2 = pad2^last_pad2&pad2;
    pad_falling2 = pad2^last_pad2&last_pad2;
    
    // Player1Join
    if(!player1joined && pad & PAD_START)
    {
      demo_mode_on=false;
      actor_state[0].isAI=false;
      actor_state[0].current_action=ACTION_SPAWNING;
      actor_state[0].damage=0;
      player1joined=true;
      resetLives=true;
    }
    
    if(!player2joined && pad2 & PAD_START)
    {
      demo_mode_on=false;
      actor_state[1].isAI=false;
      actor_state[1].current_action=ACTION_SPAWNING;
      player2joined=true;
      resetLives=true;
    }
    
    a_state=actor_state;
    deadCount = a_state->lives==0;
    #if NUM_ACTORS>1
    a_state++;
    deadCount += a_state->lives==0;
    #endif
    #if NUM_ACTORS>2
    a_state++;
    deadCount += a_state->lives==0;
    #endif
    #if NUM_ACTORS>3
    a_state++;
    deadCount += a_state->lives==0;
    #endif
      
    
    // TODO: fix winning of player 3(green)
    // Winning
    if(deadCount==NUM_ACTORS-1)
    {
      if(actor_state[0].lives>0)
      {
        initialize_player(0,0,54+10,143-20);
        actor_state[0].wins+=1;
        effects[current_effect_index].type=WIN;
        effects[current_effect_index].variant=0;
        effects[current_effect_index].x=actor_x[0]+4;
        effects[current_effect_index].y=actor_y[0]-8;
        effects[current_effect_index].frames=140;
        effects[current_effect_index].isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
        }
      }
      else
      {
        initialize_player(0,0,54+10,143-20);
      }
      actor_state[0].current_action=ACTION_SPAWNING;
      actor_state[0].current_action_frames=140;
      
      #if NUM_ACTORS>1
      
      if(actor_state[1].lives>0)
      {
        initialize_player(1,0,128,99-20);
        actor_state[1].wins+=1;
        effects[current_effect_index].type=WIN;
        effects[current_effect_index].variant=1;
        effects[current_effect_index].x=actor_x[1]+4;
        effects[current_effect_index].y=actor_y[1]-8;
        effects[current_effect_index].frames=140;
        effects[current_effect_index].isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
        }
      }
      else
      {
        initialize_player(1,0,128,99-20);
      }
      actor_state[1].current_action=ACTION_SPAWNING;
      actor_state[1].current_action_frames=140;
      #endif
      
      #if NUM_ACTORS>2
      
      if(actor_state[2].lives>0)
      {
        initialize_player(2,0,170,99-0);
        actor_state[2].wins+=1;
        effects[current_effect_index].type=WIN;
        effects[current_effect_index].variant=2;
        effects[current_effect_index].x=actor_x[2]+4;
        effects[current_effect_index].y=actor_y[2]-8;
        effects[current_effect_index].frames=140;
        effects[current_effect_index].isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
        }
      }
      else
      {
        initialize_player(2,0,170,99-0);
      }
      actor_state[2].current_action=ACTION_SPAWNING;
      actor_state[2].current_action_frames=180;
      #endif
      
      #if NUM_ACTORS>3
      
      if(actor_state[3].lives>0)
      {
        initialize_player(3,0,140,99-20);
        actor_state[3].wins+=1;
        effects[current_effect_index].type=WIN;
        effects[current_effect_index].variant=3;
        effects[current_effect_index].x=actor_x[3]+4;
        effects[current_effect_index].y=actor_y[3]+8;
        effects[current_effect_index].frames=140;
        effects[current_effect_index].isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
        }
      }
      else
      {
        initialize_player(3,0,140,99-20);
      }
      actor_state[3].current_action=ACTION_SPAWNING;
      actor_state[3].current_action_frames=180;
      #endif
      
      {
        a_state=actor_state;
        update_player_wins(vram_line+5);
        
        #if NUM_ACTORS >1
        a_state++;
        update_player_wins(vram_line+12);
        #endif

        #if NUM_ACTORS >2
        a_state++;
        update_player_wins(vram_line+19);
        #endif

        #if NUM_ACTORS >3
        a_state++;
        update_player_wins(vram_line+26);
        #endif
      }
    
      
      resetLives=true;
    }
    
    if(resetLives)
    {
      actor_state[0].damage=0;
      actor_state[0].lives=4;
      actor_state[1].damage=0;
      actor_state[1].lives=4;
      #if NUM_ACTORS >2
      actor_state[2].damage=0;
      actor_state[2].lives=4;
      #endif
      #if NUM_ACTORS >3
      actor_state[3].damage=0;
      actor_state[3].lives=4;
      #endif
    }
    
    
    num_ai=NUM_ACTORS-player1joined-player2joined;
    
    // Control of player intents based on controller or simulation.
    {
      // Reset left/right.
      if(player1joined)
      {
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
      }
      if(player2joined)
      {
        if(pad2 & PAD_LEFT && !(pad2 & PAD_RIGHT))
        {
            actor_intent[1].dir = DIR_LEFT;
        }
        else if(pad2 & PAD_RIGHT)
        {
            actor_intent[1].dir = DIR_RIGHT;
        }
        else
        {
          actor_intent[1].dir = DIR_NONE;
        }

        // Jump / cancel jump when state changes. Let simulation update consume intent in between.
        if(pad_rising2 & PAD_A)
        {
          actor_intent[1].jump = true;
        }
        if((pad2 & PAD_A )==false && actor_intent[0].jump==true)
        {
          actor_intent[1].jump = false;
        }

        actor_intent[1].fast_fall = pad2 & PAD_DOWN?true:false;
        if(pad_rising2 & PAD_DOWN)
        {
          actor_intent[1].crouch = true;
        }
        else if (pad_falling2 & PAD_DOWN)
        {
          actor_intent[1].crouch = false;
        }

        if(pad_rising2 & PAD_B)
        {
          actor_intent[1].attack = true;
        }
      }

      if(num_ai>0)
      {
        do
        {
          if(simulate_i>=NUM_ACTORS-1)
          {
            simulate_i=0;
          }
          else
          {
            simulate_i+=1;
          }
        }while(actor_state[simulate_i].isAI==false);
        
        simulate_player(simulate_i);
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
    for (i=0; i<NUM_ACTORS; i++)  // 10 scanlines per actor
    {
      enum action_state cur_action;
      bool on_ground=false;
      byte action_frames=0;
      log_state_update(0);
      
      // Record initial position to detect going over the edge of the screen
      actor_prev_x[i]=actor_x[i];
      actor_prev_y[i]=actor_y[i];
      
      if(a_state->lives==0)
      {
        // Do not simulate if dead
      }
      else if(a_state->current_action == ACTION_SPAWNING)
      {
        a_state->current_action_frames--;
        if((a_state->current_action_frames < 60
            && (a_intent->fast_fall || a_intent->jump || a_intent->dir!=DIR_NONE)
           )||a_state->current_action_frames == 0)
        {
          a_state->current_action = ACTION_STAND_BY_AIR;
          a_state->current_action_frames=0;
        }
      } else
      {

        cur_action=a_state->current_action;
        action_frames=a_state->current_action_frames;
        on_ground=ON_GROUND(cur_action);

        //Speedx value
        tmp_speed_x_value=*a_speed_x;
        tmp_target_speed_x = tmp_speed_x_value;

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

          // Air attack
          if(a_intent->attack && a_state->current_attack==ATTACK_NONE) // todo: switch to check animated/cancelable attack
          {
            if(a_state->facing_dir == DIR_RIGHT)
            {
              a_state->current_attack = ATTACK_AIR_NEUTRAL_RIGHT;
            } else
            {
              a_state->current_attack = ATTACK_AIR_NEUTRAL_LEFT;
            }
            a_state->current_attack_frames_left = a_params->attack_air_neutral_frames;
            cur_action = ACTION_STAND_BY_AIR;
            
            //APU_NOISE_DECAY(0x7f,0x0,0x6);
            APU_PULSE_DECAY(PULSE_CH0, 0x300, 0x0, 0xc1, 0x2);
            APU_PULSE_SWEEP(0, 0, 2, 0x0);
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
              tmp_target_speed_x=-a_params->run_speed;
            }
            else if(a_intent->dir == DIR_RIGHT)
            {
              tmp_target_speed_x=a_params->run_speed; 
            }
            // todo:skidding
            // todo:dashing
          }
        }
        else // on ground
        {
          // Reset fast fall intent on ground. Todo:move after input.
          a_intent->fast_fall=false;

          // Initiate attack from any non-animated state.
          // Set action to stand by ground.
          if(a_intent->attack 
             && cur_action != ACTION_DASHING_GROUND
             && a_state->current_attack==ATTACK_NONE) // todo: switch to check animated/cancelable attack
          {
            if(a_state->facing_dir == DIR_RIGHT)
            {
              a_state->current_attack = ATTACK_NORMAL_RIGHT;
            } else
            {
              a_state->current_attack = ATTACK_NORMAL_LEFT;
            }
            a_state->current_attack_frames_left = a_params->attack_neutral_frames;
            cur_action = ACTION_STAND_BY_GROUND;
            APU_PULSE_DECAY(PULSE_CH0, 0x130, 0x0, 0xc1, 0x3);
            APU_PULSE_SWEEP(0, 0, 2, 0x0);
          }


          if(cur_action==ACTION_STAND_BY_GROUND)
          {
            // Transition stabdby -> walk
            if(a_intent->dir != DIR_NONE)
            {
              cur_action=ACTION_WALKING_GROUND;
              action_frames=0;
            }
          }
          if(cur_action==ACTION_WALKING_GROUND) // depends on above
          {
            // Count and apply frames to run
            if(action_frames>a_params->frames_to_run)
            {
              cur_action=ACTION_RUNNING_GROUND;
              action_frames=0;
            }
            else
            {

              if(a_state->direction_changed || tmp_speed_x_value==0)
              {
                action_frames=0;
              }
              if(a_intent->dir == DIR_LEFT)
              {
                tmp_target_speed_x=-a_params->walk_speed; // TODO: gradual change
              }
              else if(a_intent->dir == DIR_RIGHT)
              {
                tmp_target_speed_x=a_params->walk_speed;
              }
            }
          }
          if(cur_action==ACTION_RUNNING_GROUND) // depends on above
          {

            // change action to dash if attacking when running
            if(a_intent->attack)
            {
              cur_action=ACTION_DASHING_GROUND;
              action_frames=0;
            }
            else if(a_intent->dir == DIR_LEFT)
            {
              tmp_target_speed_x=-a_params->run_speed; // TODO: gradual change
            }
            else if(a_intent->dir == DIR_RIGHT)
            {
              tmp_target_speed_x=a_params->run_speed;
            }
            else
            {
              // transition directly to stand by if not dashing or intent dir.
              // todo:skidding
              // todo:dashing
              cur_action=ACTION_STAND_BY_GROUND;
              tmp_target_speed_x=0;
              action_frames=0;
            }
          }
          if(cur_action==ACTION_DASHING_GROUND)
          {
            if(a_state->moving_dir == DIR_LEFT)
            {
              tmp_target_speed_x=-a_params->dash_speed;
              tmp_speed_x_value=tmp_target_speed_x; // Skip interpolation
            }
            else if(a_state->moving_dir == DIR_RIGHT)
            {
              tmp_target_speed_x=a_params->dash_speed;
              tmp_speed_x_value=tmp_target_speed_x; // Skip interpolation
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
          else
          {
            if(a_intent->crouch)
            {
              cur_action=ACTION_CROUCHING_GROUND;
              action_frames=0;
            }
          }

          if(cur_action==ACTION_CROUCHING_GROUND
            && a_intent->crouch!=true)
          {
            cur_action=ACTION_STAND_BY_GROUND;
            action_frames=0;
          }

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
              a_state->double_jumps_left=1;
              action_frames=0;
            }
          }
          // Do short jump when cancelling jump early.
          else if (cur_action==ACTION_CROUCHING_TO_JUMP_GROUND)
          {
            *a_speed_y = -a_params->short_hop_force;
            a_intent->jump = false;
            cur_action = ACTION_STAND_BY_AIR;
            a_state->double_jumps_left=1;
            action_frames=0;
          }
          // Note: state might not be on ground anymore.
        } // both air and ground behavior covered.
      
        // Inertia when slowing down
        // TODO: make state specific.
        // TODO: Implement target speed instead of only targeting to 0
        if(a_intent->dir == DIR_NONE)
        {
          tmp_target_speed_x = 0;
        }

        // Interpolate toward target speed.
        if(tmp_speed_x_value>tmp_target_speed_x)
        {
          tmp_speed_x_value= tmp_speed_x_value-20;
          tmp_speed_x_value= MAX(tmp_target_speed_x,tmp_speed_x_value);
        }
        else if(tmp_speed_x_value<tmp_target_speed_x)
        {
          tmp_speed_x_value= tmp_speed_x_value+20;
          tmp_speed_x_value= MIN(tmp_target_speed_x,tmp_speed_x_value);
        }
        *a_speed_x=tmp_speed_x_value;

        a_intent->attack = false;
        a_state->current_action=cur_action;
        ++action_frames;
        a_state->current_action_frames=MIN(action_frames,255);
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
    
    
    log_precalc_hitboxes(0);
    // Precalc hitboxes of the frame
    /*
    a_state=actor_state;
    a_intent=actor_intent;
    a_params=actor_params;
    a_speed_x=actor_speedx;
    a_speed_y=actor_speedy;
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
      // todo:
    }
    */
    log_precalc_hitboxes(0);
    
    // Actor State and intent physics
    
    a_state=actor_state;
    a_intent=actor_intent;
    a_params=actor_params;
    a_speed_x=actor_speedx;
    a_speed_y=actor_speedy;
    a_sprite=actor_sprite;

    // Actor State and intent physics
    for (i=0; i<NUM_ACTORS; i++) 
    {
      
      bool on_ground=false;
      enum action_state cur_action=a_state->current_action;
      
      
      
      o_state = actor_state; // Reset opponent state to first actor
              
      // Process attacks (from others): 3 scanlines
      if(a_state->current_action!=ACTION_SPAWNING)
      {
        unsigned char k;
        for(k = 0; k<NUM_ACTORS;k++) 
        {
          // k is attacking player id
          // i is current player
          log_process_attacks(0);
          if(k==i) // skip attacking self
          {
            o_state++;
            continue;
          }
          tmp_attack_type = o_state->current_attack;
          if(tmp_attack_type != ATTACK_NONE)
          {
            // TODO: precalc hitboxes.
            byte attack_y1; //upper
            byte attack_y2; //lower
            byte attack_x1; //left
            byte attack_x2; //right
            byte offset_y1;
            byte offset_y2;
            byte offset_x1;
            byte offset_x2;
            
            short int force_x;
            short int force_y;
            
            switch(tmp_attack_type)
            {
              case ATTACK_NORMAL_RIGHT:
                offset_y1 = 0;
                offset_y2 = 6;
                offset_x1 = 10;
                offset_x2 = 18;
                break;
              case ATTACK_NORMAL_LEFT:
                offset_y1 = 0;
                offset_y2 = 6;
                offset_x1 = 256-2;
                offset_x2 = 6;
                break;
              case ATTACK_AIR_NEUTRAL_RIGHT:
                offset_y1 = 6;
                offset_y2 = 18;
                offset_x1 = 8;
                offset_x2 = 19;
                break;
              case ATTACK_AIR_NEUTRAL_LEFT:
                offset_y1 = 6;
                offset_y2 = 16;
                offset_x1 = 256-3;
                offset_x2 = 8;
                break;
            }
            
            // Attack box y
            attack_y1=actor_y[k]+offset_y1;
            attack_y2=actor_y[k]+offset_y2;
            // Hit box y comparison
            if(actor_y[i]<attack_y2 // attack bottom lower than head
              && actor_y[i]+17 > attack_y1 // feet lower than attack top
              )
            {
              attack_x1=actor_x[k]+offset_x1;
              attack_x2=actor_x[k]+offset_x2;
              if(actor_x[i]+12>attack_x1 // actor hitbox comparison
                 && actor_x[i]+4<attack_x2
                )
              {
                switch(tmp_attack_type)
                {
                  case ATTACK_NORMAL_RIGHT:
                    effects[current_effect_index].type=HIT;
                    effects[current_effect_index].variant=4; // apply flip
                    effects[current_effect_index].x=attack_x1+6;
                    effects[current_effect_index].y=attack_y1;
                    effects[current_effect_index].isNew=true;
                    force_x=40+a_state->damage;
                    force_y=-(20+(a_state->damage<<1));
                    break;
                  case ATTACK_NORMAL_LEFT:
                    effects[current_effect_index].type=HIT;
                    effects[current_effect_index].variant=0;
                    effects[current_effect_index].x=attack_x1-6;
                    effects[current_effect_index].y=attack_y1;
                    effects[current_effect_index].isNew=true;
                    force_x=-40-a_state->damage;
                    force_y=-(20+(a_state->damage<<1));
                    break;
                  case ATTACK_AIR_NEUTRAL_RIGHT:
                    effects[current_effect_index].type=HIT;
                    effects[current_effect_index].variant=4;
                    effects[current_effect_index].x=attack_x1+2;
                    effects[current_effect_index].y=attack_y1;
                    effects[current_effect_index].isNew=true;
                    force_x=80+a_state->damage;
                    force_y=-(10+(a_state->damage<<2));
                    break;
                  case ATTACK_AIR_NEUTRAL_LEFT:
                    effects[current_effect_index].type=HIT;
                    effects[current_effect_index].variant=0;
                    effects[current_effect_index].x=attack_x1-2;
                    effects[current_effect_index].y=attack_y1;
                    effects[current_effect_index].isNew=true;
                    force_x=-80-a_state->damage;
                    force_y=-(10+(a_state->damage<<2));
                    break;
                }
                
                actor_speedy[i]=force_y;
                //actor_x[i]=(short int)actor_x[i];
                actor_speedx[i]+=force_x;
                a_state->damage+=3;
                a_state->damage_vis_frames+=3;
                
                effects[current_effect_index].frames=6;
                effects[current_effect_index].variant+=i; // Apply player color
                current_effect_index++;
                if(current_effect_index==4)
                {
                  current_effect_index=0;
                }
              }
            }
          }
          o_state++;
        } 
      }
    
      log_start_physics(0); // (3 scanlines before log_collision_calculation)
      // Actor Physics: around 25 scanlines
      {
        short int actorxf;
        short int actoryf;
        bool on_edge;
        bool falling;
        bool on_platform;
        byte actor_feet_x; 
        byte actor_feet_y;
        byte speed_y_in_pixels;
        bool action_on_ground;


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
        
        cur_platform=platforms;

        on_ground = false;
        on_edge = false;
        for(j=0;j<p_count;++j) // heavy on air. 2,5 scanlines min, 5.5 max?
        {
          bool skip_due_to_fall_through;
          
          log_collision_calculation(0);
                    
          falling=*a_speed_y >= 0; // may update for each platform
          actor_feet_x=actor_x[i]+8; // may update for each platform
          actor_feet_y=actor_y[i]+17; // may update for each platform
          
          speed_y_in_pixels=(*a_speed_y>>8);
          on_platform=
             actor_feet_y>=cur_platform->y1-speed_y_in_pixels
             && actor_feet_y<=cur_platform->y2
             && actor_feet_x>cur_platform->x1
             && actor_feet_x<cur_platform->x2;
          
          // Side collision and grab
          if(!on_platform)
          {
            if(cur_platform->has_edge)
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

              grab_box_x1=actor_feet_x-8; //Todo: precalculate? check close enough first?
              grab_box_x2=actor_feet_x+8;
              grab_box_y=actor_y[i];

              if(falling 
                 && grab_box_y>=cur_platform->y1
                 && grab_box_y<=cur_platform->y2
                 && !a_intent->jump 
                 && !a_intent->fast_fall 
                 && !a_intent->crouch // drop
                 )
              {
                if(a_intent->dir!=DIR_LEFT // right+neutral dir
                   && grab_box_x2>cur_platform->x1
                   && grab_box_x1<cur_platform->x1
                  )
                {
                  *a_speed_x=0;*a_speed_y=0;
                  actor_y[i]=cur_platform->y1;
                  actor_x[i]=cur_platform->x1-11;
                  a_state->current_action = ACTION_HANGING_GROUND;
                  a_state->facing_dir = DIR_RIGHT;
                  on_ground = true;
                }
                else if(a_intent->dir!=DIR_RIGHT // left + neutral dir
                   && grab_box_x1<cur_platform->x2
                   && grab_box_x2>cur_platform->x2)
                {
                  *a_speed_x=0;*a_speed_y=0;
                  actor_y[i]=cur_platform->y1;
                  actor_x[i]=cur_platform->x2-5;
                  a_state->current_action = ACTION_HANGING_GROUND;
                  a_state->facing_dir = DIR_LEFT;
                  on_ground = true;
                }
              }
            }
          }
          else // on_platform = true
          {
            // normal collision
            if(falling)
            {
              skip_due_to_fall_through=(cur_platform->can_fall_through && (a_state->current_action==ACTION_CROUCHING_GROUND || a_state->fall_through_triggered));
              if(skip_due_to_fall_through)
              {
                a_state->fall_through_triggered = true;
              }
              else
              {
                actor_y[i] = cur_platform->y1-17;
                *a_speed_y = 0;
                actor_yf[i] = 0;
                on_ground = true;
              }
            }
            // todo: split condition to improve perf
            // on_edge state update based on facing dir
            if(((a_state->facing_dir == DIR_LEFT && actor_feet_x<cur_platform->x1+6) 
                || (a_state->facing_dir == DIR_RIGHT && actor_feet_x>cur_platform->x2-6))
               )
            {
              on_edge=true;
            }
          }
          
          cur_platform++;
          
        }
        
        log_state_updates_after_sim(0);
        
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
          a_state->double_jumps_left=1;
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
        
	log_sprite_selection(0);
        // Select sprite
        // TODO: deduplicate
        // todo: improve facing difection.
        if(a_state->lives ==0)
        {
          *a_sprite=char1fast_fall_sprites[i]; // Use fast falling sprite for dead.
        }
        else if(a_state->current_action == ACTION_SPAWNING)
        {
          *a_sprite=char1fast_fall_sprites[i]; // Use fast falling sprite for spawn. 
        }
        else
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
              *a_sprite = char1neutral_sprites[sprite_var];
            }
            else if(cur_action==ACTION_RUNNING_GROUND)
            {
              *a_sprite = char1run_sprites[sprite_var];
            }
            else if(cur_action==ACTION_CROUCHING_GROUND)
            {
              *a_sprite = char1crouch_sprites[sprite_var];
            }
            else if(cur_action==ACTION_STAND_BY_GROUND 
                    || cur_action==ACTION_WALKING_GROUND)
            {
              if(a_state->on_edge)
              {
                *a_sprite = char1sway_sprites[sprite_var];
              }
              else
              {
                *a_sprite = char1stand_sprites[sprite_var];
              }
            }
            else if(cur_action==ACTION_HANGING_GROUND)
            {
              *a_sprite = char1ledge_sprites[sprite_var];
            }
            else if(cur_action==ACTION_DASHING_GROUND)
            {
              *a_sprite = char1dash_sprites[sprite_var];
            }
            else
            {
              // Todo: handle jump crouch as part of animation instead of last case
              *a_sprite = char1crouch_sprites[sprite_var];
            }
          }
          else
          {
            if(a_state->current_attack == ATTACK_AIR_NEUTRAL_LEFT
               || a_state->current_attack == ATTACK_AIR_NEUTRAL_RIGHT)
            {
              *a_sprite = char1airneutral_sprites[sprite_var];
            }
            else if(a_intent->fast_fall) // Todo: use state instead of intent.
            {
              *a_sprite = char1fast_fall_sprites[sprite_var];
            }
            else
            {
              *a_sprite = char1jump_sprites[sprite_var];
            }
          }
        }
      }
      {
        bool do_respawn=false;
        if(actor_speedy[i]>0
          && actor_prev_y[i]>actor_y[i])
        {
          effects[current_effect_index].type=EXPLOSION_VERTICAL;
          effects[current_effect_index].variant=4+i; // apply flip and player color
          effects[current_effect_index].x=actor_x[i];
          effects[current_effect_index].y=210;
          effects[current_effect_index].frames=30;
          effects[current_effect_index].isNew=true;
          current_effect_index++;
          if(current_effect_index==4)
          {
            current_effect_index=0;
          }
          do_respawn=true;
        }
        else if(actor_speedx[i]>0
          && actor_prev_x[i]>actor_x[i])
        {
          effects[current_effect_index].type=EXPLOSION_HORIZONTAL;
          effects[current_effect_index].variant=4+i; // apply flip and player color
          effects[current_effect_index].x=255-20;
          effects[current_effect_index].y=actor_y[i]-10;
          effects[current_effect_index].frames=30;
          effects[current_effect_index].isNew=true;
          current_effect_index++;
          if(current_effect_index==4)
          {
            current_effect_index=0;
          }
          do_respawn=true;
        }
        else if( actor_speedx[i]<0
          && actor_prev_x[i]<actor_x[i])
        {
          effects[current_effect_index].type=EXPLOSION_HORIZONTAL;
          effects[current_effect_index].variant=0+i; // apply player color
          effects[current_effect_index].x=4;
          effects[current_effect_index].y=actor_y[i];
          effects[current_effect_index].frames=30;
          effects[current_effect_index].isNew=true;
          current_effect_index++;
          if(current_effect_index==4)
          {
            current_effect_index=0;
          }
          do_respawn=true;
        }
        if(do_respawn)
        {
          if(a_state->lives>0)
          {
            actor_x[i]=120;
            actor_y[i]=40;
            a_state->lives-=1;
          }
          actor_speedy[i]=0;
          actor_speedx[i]=0;
          a_state->current_action=ACTION_SPAWNING;
          a_state->current_action_frames=90;
          a_state->current_attack=ATTACK_NONE;
          a_state->damage=0;

        }
      }
            
      if(i<NUM_ACTORS)
      {
        a_state++;
        a_intent++;
        a_params++;
        a_speed_x++;
        a_speed_y++;
        a_sprite++;
      }
      log_end_physics(0);
    }
    log_update_sprites(0);
    oam_id = 0;
    // Update Effect Sprites
    {
      struct effect* current_effect;
      current_effect=effects;
      for (i=0; i<4; i++)
      {
        if(current_effect->frames>0)
        {
          bool visible=true;
          switch(current_effect->type)
          {
            case HIT:
              a_sprite=&small_hit_sprites[current_effect->variant];
              break;
            case EXPLOSION_HORIZONTAL:
            case EXPLOSION_VERTICAL:
              if(current_effect->type == EXPLOSION_HORIZONTAL)
              {
                a_sprite=&horizontal_explosion_sprites[current_effect->variant];
              }
              else
              {
                a_sprite=&vertical_explosion_sprites[current_effect->variant];
              }
              if(current_effect->frames & (byte)0x02)
              {
                visible=false;
              }
              else
              {
                if(current_effect->frames>20)
                {
                  switch(current_effect->variant&0x03){
                    case 0://todo:optimize with array for colors
                      background_color=0x0c;
                      break;
                    case 1:
                      background_color=0x17;
                      break;
                    case 2:
                      background_color=0x19;
                      break;
                    case 3:
                      background_color=0x16;
                      break;
                  }
                }
              }
              break;
            case WIN:
              a_sprite=&winner_sprites[current_effect->variant];
              break;
          }
          if(visible)
          {
            oam_id = oam_meta_spr(current_effect->x, current_effect->y, oam_id, *a_sprite);
          }
          if(current_effect->isNew)
          {
            
            if(current_effect->type == HIT)
            {
              //APU_PULSE_DECAY(PULSE_CH0, 0x120, 0x0, 0xc1, 0x3);
              //APU_PULSE_SWEEP(0, 0, 2, 0x0);
            }
            else if(current_effect->type == WIN)
            {
              APU_PULSE_DECAY(PULSE_CH1, 0x4ff, 0x0, 0xc1, 0xF);
              APU_PULSE_SWEEP(PULSE_CH1, 0X0, 0X2, 0x1);
            }
            else
            {
              APU_PULSE_DECAY(PULSE_CH1, 0x4ff, 0x0, 0xc1, 0xF);
              APU_PULSE_SWEEP(PULSE_CH1, 0X0, 0X3, 0x1);
              
              APU_TRIANGLE_LENGTH(0xff,0x3);
            }
            current_effect->isNew=false;
          }
          else
          {
            if(current_effect->type != HIT)
            {
              APU_TRIANGLE_LENGTH(0xff-current_effect->frames<<2,0x4);
            }
          }
          current_effect->frames--;
        }
        current_effect++;
      }
    }
    // Update Actor Sprites
    {
      a_state=actor_state;
      a_sprite=actor_sprite;
      // start with OAMid/sprite 0
      
      // draw and move all actors
      for (i=0; i<NUM_ACTORS; i++) 
      {
        // TODO: add camera
        if(a_state[i].lives!=0)
        {
          oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, *a_sprite);
        }
        a_sprite++;
      }
      
      a_icon=char1icon_sprites;
      a_iconAI=AIicon_sprites;
      for (i=0; i<NUM_ACTORS; i++) 
      {
        const unsigned char* curIcon;
        if(a_state->isAI)
        {
          curIcon=*a_iconAI;
        }
        else
        {
          curIcon=*a_icon;
        }
        zp_x=icon_pos_x[i];
        oam_id = oam_meta_spr(zp_x+(a_state->damage_vis_frames>>2), 189-(a_state->damage_vis_frames), oam_id, curIcon);
        switch(a_state->lives)
        {
          case 0:
            oam_id = oam_meta_spr(zp_x+11, 204, oam_id, char1lives0_sprites[i]);
            break;
          case 1:
            oam_id = oam_meta_spr(zp_x+11, 204, oam_id, char1lives1_sprites[i]);
            break;
          case 2:
            oam_id = oam_meta_spr(zp_x+11, 204, oam_id, char1lives2_sprites[i]);
            break;
          case 3:
            oam_id = oam_meta_spr(zp_x+11, 204, oam_id, char1lives3_sprites[i]);
            break;
          case 4:
            oam_id = oam_meta_spr(zp_x+11, 204, oam_id, char1lives4_sprites[i]);
            break;
        }
        a_icon++;
        a_iconAI++;
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
      update_player_status(vram_line2+25);
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
          PPU.vram.data = background_color;
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
