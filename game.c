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

#include "opt_macros.h"

#include "fourscore.h"

#include "avg8.h"

#include "attrib_parallax.h"

#include "oam_ultra.h"


//#defi ne CFGFILE test

// link the pattern table into CHR ROM
//#link "chr_generic.s"
//#link "attrib_parallax.c"
//#link "oam_ultra.s"


// attribute table in PRG ROM
const char ATTRIBUTE_TABLE[0x40] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // rows 0-3
  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, // rows 4-7
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, // rows 8-11
  0xff, 0xff, 0xff, 0xaa, 0xff, 0xff, 0xff, 0xff, // rows 12-15
  0x00, 0x01, 0x0a, 0x01, 0x0f, 0x0f, 0x06, 0x07, // rows 16-19
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // rows 20-23
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // rows 24-27
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f  // rows 28-29
};

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[33] = { 
  0x1C,			// screen color

  0x0F,0x02,0x10,0x00,	// background palette 0
  0x0F,0x03,0x2D,0x00,	// background palette 1
  0x0F,0x04,0x10,0x00,	// background palette 2
  0x0F,0x0A,0x10,0x00,       // background palette 3
    
  0x0D,0x17,0x32,0x00,	// sprite palette 0
  0x0D,0x17,0x27,0x00,	// sprite palette 1
  0x0D,0x17,0x29,0x00,	// sprite palette 2
  0x0D,0x17,0x26,0x00	// sprite palette 3

};

//const char sky_default = 0x1C;
const char sky_default = 0x13;

const signed char hit_lag_random_shake[32] = 
{
  2,   3, -2,  1,  0, -3, -1, -3, 
  -1,  1,  2  -2,  0,  2,  0,  2,
  -3, -1,  2,  3, -2,  3, -2,  0,
  2,  -3, -1,  1, -1,  1,  3,  -2
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
// !implement hitlag 
// !control hitbox effect during attack(active frames, damage amount) 
// !respawn invincibility
// *fix perf of indexing sprites
// *clarify states and logic more(enums and masks)
// *support coordinates outside of screen
// !KO on arena edges, spawning
// !counting lives
// !restart after winning
// *acceleration per action
// *dash dancing support
// *dash attack
// *crouch dash cancel
// /dash attack animation (day 5.5)
// *double tap dash
// /dash attack
// *more clear attack/animation state
// !make AI more aggressive
// !hitlag shake animation
// *advanced knockback calculation https://www.ssbwiki.com/Knockback#Formula
// *hitstun based on knockback
// *flich animation for hit stun
// *

// 1





#define AI_ICON 0xc8
#define CHAR1_ICON 0xd0

#define LIVES0 0x8b
#define LIVES1 0x8f
#define LIVES2 0x8e
#define LIVES3 0x8d
#define LIVES4 0x8c

// Character 1 tiles (2x2 metasprites)
#define CHAR1_NEUTRAL    0xd4
#define CHAR1_STAND      0xd8
#define CHAR1_CROUCH     0xdc
#define CHAR1_RUN        0xec
#define CHAR1_JUMP       0xe0
#define CHAR1_FAST_FALL  0xe4
#define CHAR1_DASH       0xe8
#define CHAR1_LEDGE      0xf0
#define CHAR1_SWAY       0xf4
#define CHAR1_AIRNEUTRAL 0xf8
#define CHAR1_FLINCH     0xfc

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
      byte r=rand()%2;
      pos+=1+rand()%0x03;
      if(pos>len-1)pos-=len;
      vram_adr(NTADR_A(x+pos,y-1));
      vram_fill(0x92+r, 1);
      vram_adr(NTADR_C(x+pos+1,y-1));
      vram_fill(0x92+r, 1);
    }
    // Ends
    vram_adr(NTADR_A(x,y));
    vram_fill(0x90+0x10, 1);
    vram_adr(NTADR_A(x+len-1,y));
    vram_fill(0x90+0x11, 1);
  }
  
  x=x+1;
  
  vram_adr(NTADR_C(x,y));
  vram_fill(0x90+type, len);
  vram_adr(NTADR_C(x+1,y+1));
  vram_fill(0x05, 1);
  vram_fill(0x09, len-4);
  vram_fill(0x06, 1);
  
  if(type==0)
  {
    //int i;
    int pos=0;
    int max=len>>1;
    //for(i=0;i<max;++i)
    //{
    //  pos+=1+rand()%0x03;
    //  if(pos>len-1)pos-=len;
    //  vram_adr(NTADR_C(x+pos,y-1));
    //  vram_fill(0x92+rand()%2, 1);
    //}
    // Ends
    vram_adr(NTADR_C(x,y));
    vram_fill(0x90+0x10, 1);
    vram_adr(NTADR_C(x+len-1,y));
    vram_fill(0x90+0x11, 1);
  }
}

// Todo: convert to defines
enum action_state
{
  ACTION_STAND_BY_GROUND=0,
  ACTION_STAND_BY_AIR=1,
  ACTION_CROUCHING_TO_JUMP_GROUND=2,
  ACTION_CROUCHING_GROUND=3,
  ACTION_HANGING_GROUND=4,
  ACTION_WALKING_GROUND=5,
  ACTION_RUNNING_GROUND=6,
  ACTION_DASHING_GROUND=7, // Todo: Refactor to "Animated attack"
  ACTION_TURNING_AROUND_GROUND=8, // todo
  ACTION_STOPPING_GROUND=9, // todo
  ACTION_FAST_FALLING_AIR=11, 
  ACTION_SPAWNING=11,
  ACTION_HIT_STUN_AIR=12,
  ACTION_HIT_STUN_GROUND=13,
};

unsigned char action_state_to_char[14] = {'G','A','c','C','H','W','R','D',' ',' ','F','!','x','X'};

#define ON_GROUND(state) (((state)!=ACTION_STAND_BY_AIR)&&((state)!=ACTION_FAST_FALLING_AIR)&&((state)!=ACTION_HIT_STUN_AIR))

enum dir
{
  DIR_LEFT = 0,
  DIR_RIGHT = 1,
  DIR_NONE = 2,
};

enum vdir
{
  VDIR_UP = 0,
  VDIR_DOWN = 1,
  VDIR_NONE = 2,
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
  byte hit_lag_frames_left;
  byte double_jumps_left;
  bool on_edge;
  bool fall_through_triggered;
  byte damage_vis_frames;
  bool isAI;
  byte lives;
  byte wins;
};



// Converted to arrays
/*
struct intent{
  enum dir dir;
  enum vdir vdir;
  bool jump;

  bool crouch;
  bool dash;
  bool fast_fall;
  bool attack;
};
*/


struct params{
  short unsigned int jump_force;
  short unsigned int short_hop_force;
  short unsigned int walk_speed;
  short unsigned int run_speed;
  short unsigned int dash_speed;
  byte dash_frames;
  byte frames_to_run;
  byte double_jumps;
  byte jump_crouch_frames;
  byte attack_neutral_frames;
  byte attack_air_neutral_frames;
  short unsigned int fall_force;
  short unsigned int fall_limit;
  short unsigned int fast_fall;
};


// Converted to arrays.
/*
struct platform{
  byte x1;
  byte x2;
  byte y1;
  byte y2;
  byte can_fall_through;
  byte has_edge;
};
*/

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


struct effect effects[4];
byte current_effect_index;

// 2

#define NUM_ACTORS 4
#define NUM_PLATFORMS 4

byte actor_x[NUM_ACTORS];      // Position
byte actor_y[NUM_ACTORS];
byte actor_xf[NUM_ACTORS];     // Fraction
byte actor_yf[NUM_ACTORS];
byte actor_prev_x[NUM_ACTORS]; // Perevious pos to track going outside
byte actor_prev_y[NUM_ACTORS];
short int actor_speedx[NUM_ACTORS]; // Speed
short int actor_speedy[NUM_ACTORS];
//void *actor_sprite[NUM_ACTORS];// Which sprite to show
unsigned char actor_tile[NUM_ACTORS];
unsigned char actor_variant[NUM_ACTORS];

struct state actor_state[NUM_ACTORS];
struct params actor_params[NUM_ACTORS]; // Todo: move to rom

//struct intent actor_intent[NUM_ACTORS];
byte actor_intent_dir[NUM_ACTORS];
byte actor_intent_vdir[NUM_ACTORS];
byte actor_intent_jump[NUM_ACTORS];
byte actor_intent_crouch[NUM_ACTORS];
byte actor_intent_dash[NUM_ACTORS];
byte actor_intent_fast_fall[NUM_ACTORS];
byte actor_intent_attack[NUM_ACTORS];

//struct platform platforms[NUM_PLATFORMS];
byte platform_x1[NUM_PLATFORMS];
byte platform_x2[NUM_PLATFORMS];
byte platform_y1[NUM_PLATFORMS];
byte platform_y2[NUM_PLATFORMS];
byte platform_has_edge[NUM_PLATFORMS];
byte platform_can_fall_through[NUM_PLATFORMS];


// 3

// try to push a frequent pointer to zp.
#pragma bss-name (push,"ZEROPAGE")
#pragma data-name(push,"ZEROPAGE")
struct state* a_state;
struct params* a_params;
struct effect* current_effect;
short int* a_speed_x;
short int* a_speed_y;
void ** a_sprite;
byte zp_x;
byte zp_y;
struct intent* intentlookup[NUM_ACTORS];
struct state* o_state;
short int tmp_speed_x_value;
short int tmp_target_speed_x;
short int tmp_speed_y_value;
enum attack_type tmp_attack_type;
short int attack_force_x;
short int attack_force_y;
unsigned char i,j,k;
signed char scroll_nudge_x;

byte abs_a, abs_b; // This is for optimized abs,min,max,clamp

#pragma bss-name (pop)
#pragma data-name(pop)



// 4

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
  platform_x1[p_count]=x*8;
  platform_x2[p_count]=(x+len)*8;
  switch(type)
  {
    case 0:
        platform_y1[p_count] = y*8-2;
        platform_y2[p_count] = (y+1)*8;
        platform_can_fall_through[p_count]=false;
        platform_has_edge[p_count]=true;
      break;
    case 1:
        platform_y1[p_count] = y*8+4-2;
        platform_y2[p_count] = (y+1)*8;
        platform_can_fall_through[p_count]=true;
        platform_has_edge[p_count]=false;
      break;
  }
  ++p_count;
}

void reset_level_and_bg()
{
  char i;
  //set_rand(80);
  set_rand(293);
  for(i=0;i<19;++i)
  {
    byte a=rand8()&0x1f;
    byte b=(rand8()&0x07)+8;
    vram_adr(NTADR_A(a,b));
    vram_fill(0x94+2+(i&0b00000011), 1);
    vram_adr(NTADR_C(a+1,b));
    vram_fill(0x94+2+(i&0b00000011), 1);
  }

  
  // Draw bg fades
  vram_adr(NAMETABLE_A);
  vram_fill(0x04, 96); // Sky
  vram_fill(0x05, 96);
  vram_adr(NTADR_A(0,22));
  vram_fill(0x07, 10);  // Bottom left 1
  vram_fill(0x04, 11+1);  // Bottom center 1
  vram_fill(0x07, 9+1);   // Bottom right 1
  vram_fill(0x08, 11-1-1);  // Bottom left 2
  vram_fill(0x04, 12+2);  // Bottom center 2
  vram_fill(0x08, 9);  // Bottom right 2
  vram_fill(0x04, 32*1);// Bottom dark
  
  vram_adr(NAMETABLE_C);
  vram_fill(0x04, 96); // Sky
  vram_fill(0x05, 96);
  vram_adr(NTADR_C(0,22));
  vram_fill(0x07, 11);  // Bottom left 1
  vram_fill(0x04, 11+1);  // Bottom center 1
  vram_fill(0x07, 9);   // Bottom right 1
  vram_fill(0x08, 11-1);  // Bottom left 2
  vram_fill(0x04, 12+2);  // Bottom center 2
  vram_fill(0x08, 9-1);  // Bottom right 2
  vram_fill(0x04, 32*1);// Bottom dark
  
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
  vram_adr(0x2BC0);
  // copy attribute table from PRG ROM to VRAM
  vram_write(ATTRIBUTE_TABLE, sizeof(ATTRIBUTE_TABLE));


}
/*
void update_player_status(struct vram_inst* inst)
{
  byte damage = a_state->damage;
  if(a_state->current_action == ACTION_SPAWNING)
  {
    return;
  }
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
*/
// Pre-calculated byte offsets into vram_line2
// Formula: (5 + player*7) * 5 + 1 (offset of _1_lda_val within struct)
const byte hud_tens_offset[4] = { 26, 61, 96, 131 };
const byte hud_ones_offset[4] = { 31, 66, 101, 136 };
const byte hud_pct_offset[4]  = { 36, 71, 106, 141 };

void update_all_hud(void)
{
    byte i, damage;
    byte* vram = (byte*)vram_line2;  // Cache cast once
    
    for (i = 0; i < NUM_ACTORS; i++) {
        if (actor_state[i].current_action == ACTION_SPAWNING)
            continue;
        
        damage = actor_state[i].damage;
        
        // BCD adjustment
        if ((damage & 0x0F) >= 10) {
            damage += 6;
            actor_state[i].damage = damage;
        }
        
        // Cap damage to DED*1.25 to avoid overflows.
        // If base damage needs to be more than 54, either drop (byte) 
        // or avoid the overvflow with shifts.
        if(damage >=200) 
        {
       	    damage = 200;
            actor_state[i].damage = damage;
        }
        
        if (damage >= 160) {
            // "DED" for dead
            vram[hud_tens_offset[i]] = 'D';
            vram[hud_ones_offset[i]] = 'E';
            vram[hud_pct_offset[i]]  = 'D';
        } else {
            // "XX%" for damage
            vram[hud_tens_offset[i]] = '0' + (damage >> 4);
            vram[hud_ones_offset[i]] = '0' + (damage & 0x0F);
            vram[hud_pct_offset[i]]  = '%';
        }
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
    inst->_1_lda_val = ' ';
    inst++;
  }
  else
  {
    if(wins>>4<1)
    {
      inst->_1_lda_val = '0'+(wins&0b00001111);
      inst++;
      inst->_1_lda_val = 0x81;
      inst++;
      inst->_1_lda_val = ' ';
    }
    else
    {
      inst->_1_lda_val = ' ';+(wins>>4);
      inst++;
      inst->_1_lda_val = '0'+(wins&0b00001111);
      inst++;
      inst->_1_lda_val = 0x81;
    }
    
  }
}



void update_debug_info(byte player,struct vram_inst* inst)
{
  enum action_state cur_action=actor_state[player].current_action;

  inst->_1_lda_val = action_state_to_char[cur_action];
  inst++;
  
  inst->_1_lda_val = 0x30+cur_action;
  inst++;
  
  inst->_1_lda_val = dir_to_char[actor_intent_dir[i]];
  inst++;
  
  if(actor_intent_jump[i])
  {
    inst->_1_lda_val = 'J';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  inst++;
  
  if(actor_intent_crouch[i])
  {
    inst->_1_lda_val = 'C';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  inst++;
  
  if(actor_intent_fast_fall[i])
  {
    inst->_1_lda_val = 'F';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  
}

void update_debug_info2(byte /*player*/,struct vram_inst* inst)
{
  enum action_state cur_action=a_state->current_action;

  inst->_1_lda_val = action_state_to_char[cur_action];
  inst++;
  
  //inst->_1_lda_val = 0x30+cur_action;
  //inst++;
  inst->_1_lda_val = 0x30+a_state->hit_lag_frames_left;
  inst++;
  inst->_1_lda_val = 0x30+a_state->current_attack_frames_left;
  inst++;
  
  
  //inst->_1_lda_val = dir_to_char[actor_intent_dir[player]];
  //inst++;
  /*
  if(actor_intent_jump[player])
  {
    inst->_1_lda_val = 'J';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  inst++;
  
  if(actor_intent_crouch[player])
  {
    inst->_1_lda_val = 'C';
  }
  else
  {
    inst->_1_lda_val = '.';
  }
  inst++;
  
  if(actor_intent_fast_fall[player])
  {
    inst->_1_lda_val = 'F';
  }
  else
  {
    inst->_1_lda_val = '.';
  }*/
  
}

// 5

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

unsigned char num_ai;

// 6

void simulate_player(unsigned char num)
{
  // TODO: optimize perf
  unsigned int r = rand();
  unsigned char platform_id_under =255;
  unsigned char platform_id_right=255;
  unsigned char platform_id_left=255;
  unsigned char actor_id_closest=255;
  byte closest_distance=255;
  
  unsigned char r128;

 
  byte actor_center_x=(byte)(actor_x[num]+8);
  
  Assert(num>NUM_ACTORS);
  
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
  
  // Closest actors
  a_state=actor_state;
  for(j=0;j<NUM_ACTORS;++j)
  {
    byte distx;
    byte disty;
    byte dist;
    if(j==num || a_state->current_action==ACTION_SPAWNING)
    {
      a_state++;
      continue;
    }
    //distx=abs(actor_x[j]-actor_x[num]);
    //disty=abs(actor_x[j]-actor_x[num]);
    ABS_DIFF(actor_x,j,num,distx);
    ABS_DIFF(actor_y,j,num,disty);
    dist=MAX(distx,disty);
    if(dist<closest_distance)
    {
      
      actor_id_closest=j;
      closest_distance=distx;
    }
    
    a_state++;
  }
  
  //unsigned char closest_platform = 0;
  // find closest platforms for AI
  for(j=0;j<p_count;++j)
  {
    bool isLeft = actor_center_x>platform_x1[j];
    bool isRight = actor_center_x<platform_x2[j];

    if(isLeft&isRight)
    {
      bool isUnder;
      isUnder=(byte)(actor_y[num]+17)>=platform_y1[j];
      if(isUnder)
      {
        platform_id_under=j;
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
        platform_id_left=j;
        //todo: make sure it is closest
      }
      else
      {
        __asm__("nop");
        __asm__("nop");
      }
      if(isRight)
      {
        platform_id_right=j;
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
  }
  {
    switch(r128)
    {
      case 0:
        break;
      case 1: // Jump
        actor_intent_jump[num] = true;
        actor_intent_crouch[num] = false;
        break;
      case 2:
      case 3: // Release Jump
        actor_intent_jump[num] = false;
        break;
      case 4: // Stop
        if(platform_id_under==255)
        {
          break;
        }
        actor_intent_dir[num] = DIR_NONE;
        actor_intent_crouch[num] = false;
        break;
      case 5: // Left
        actor_intent_dir[num]= DIR_LEFT;
        actor_intent_crouch[num] = false;
        break;
      case 6: // Right
        actor_intent_dir[num] = DIR_RIGHT;
        actor_intent_crouch[num] = false;
        break;
      case 7: // Fast fall / crouch;
        if(actor_y[actor_id_closest]>(byte)(actor_y[num]+2))
        {
          actor_intent_fast_fall[num] = true;
          actor_intent_crouch[num] = true;
        }
        break;
      case 8:
      case 9: // crouch still or go toward center
        if(128-32<actor_x[num]&&actor_x[num]<128+32)
        {
          actor_intent_crouch[num] = true;
          actor_intent_dir[num] = DIR_NONE;
        }
        else
        {
          if(actor_x[num]>128)
          {
           actor_intent_dir[num] = DIR_LEFT; 
          }
          else
          {
            actor_intent_dir[num] = DIR_RIGHT;
          }
        }
        break;
      case 10: // Cancel Fast fall / crouch
        actor_intent_fast_fall[num] = false;
        actor_intent_crouch[num] = false;
        break;
      case 11:
      case 12:
        //actor_intent_attack[num] = true;
        //break;
      case 13:
      case 14:
      case 15:
      case 16:
      case 17: // save when falling off. cost: 3 scanlines.
      case 18:
      case 19:
        if(platform_id_under==255)
        {
          if(platform_id_left!=255)
          {
            actor_intent_dir[num] = DIR_LEFT;
          }
          if(platform_id_right!=255)
          {
            actor_intent_dir[num] = DIR_RIGHT;
          }
          actor_intent_fast_fall[num]=false;
          if(actor_speedy[num]>0)
          {
            actor_intent_jump[num]=true;
          }
        }
        break;
      case 20:
      case 21:// Approach closest player
      case 22:
      case 23:
      case 24:
      case 25:
        if(platform_id_under==255)
        {
          break;
        }
        actor_intent_crouch[num] = false;
        if(actor_x[actor_id_closest]>actor_x[num])
        {
          actor_intent_dir[num] = DIR_RIGHT;
        }
        else
        {
          actor_intent_dir[num] = DIR_LEFT;
        }
        if(actor_y[actor_id_closest]<actor_y[num])
        {
          actor_intent_jump[num]=true;
          actor_intent_fast_fall[num]=false;
        }
        else
        {
          actor_intent_jump[num]=false;
          actor_intent_fast_fall[num]=true;
        }
        break;
      case 26:
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:
        // Attack if close enough
        {
          unsigned char xdiff;
          xdiff=(unsigned char)(actor_x[actor_id_closest]-actor_x[num]); // positive means right side
          if(closest_distance<16)
          {
            if(a_state->facing_dir==DIR_LEFT)
            {
              if(actor_x[actor_id_closest]<(byte)(actor_x[num]+4)){
                actor_intent_attack[num] = true;
              }
              else
              {
                actor_intent_dir[num]=DIR_RIGHT;
              }
            }
            else
            {
              if(actor_x[actor_id_closest]>(byte)(actor_x[num]-4)){
                actor_intent_attack[num] = true;
              }
              else
              {
                actor_intent_dir[num]=DIR_LEFT;
              }
            }
            if(r128&0x01)
            {
              actor_intent_dir[num]=DIR_NONE;
            }
          }
          
        }
        break;
    }
  }
}

enum RelationToPlatforms
{
  PLATFORM_UNDER,
  PLATFORM_LEFT,
  PLATFORM_RIGHT,
};

enum RelationToEnemy
{
  testtttt,
};
enum CurrentBehavior
{
  testtttttt,
};

void simulate_player_new(unsigned char num)
{
  // TODO: optimize perf
  unsigned char platform_id_under =255;
  unsigned char platform_id_right=255;
  unsigned char platform_id_left=255;
  unsigned char actor_id_closest=255;
  byte closest_distance=255;
  
  unsigned int r = rand();
  
  
  
  unsigned char r128;

  Assert(num>NUM_ACTORS);
  
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
  
  // Closest actors
  a_state=actor_state;
  for(j=0;j<NUM_ACTORS;++j)
  {
    byte distx;
    byte disty;
    byte dist;
    if(j==num || a_state->current_action==ACTION_SPAWNING)
    {
      a_state++;
      continue;
    }
    //TODO: replace with fast macros
    //distx=abs(actor_x[j]-actor_x[num]);
    //disty=abs(actor_x[j]-actor_x[num]);
    ABS_DIFF(actor_x,j,num,distx);
    ABS_DIFF(actor_y,j,num,disty);
    dist=MAX(distx,disty);
    if(dist<closest_distance)
    {
      
      actor_id_closest=j;
      closest_distance=distx;
    }
    
    a_state++;
  }
  
  //unsigned char closest_platform = 0;
  // find closest platforms for AI
  for(j=0;j<p_count;++j)
  {
    // Todo use (byte) to optimize
    bool isLeft = actor_x[num]+8>platform_x1[j];
    bool isRight = actor_x[num]+8<platform_x2[j];

    if(isLeft&isRight)
    {
      bool isUnder;
      isUnder=actor_y[num]+17>=platform_y1[j];
      if(isUnder)
      {
        platform_id_under=j;
        //todo: make sure it is closest under
      }
    }else
    {
      if(isLeft)
      {
        platform_id_left=j;
        //todo: make sure it is closest
      }
      if(isRight)
      {
        platform_id_right=j;
        //todo: make sure it is closest
      }
    }
  }
  
  
}

// 8

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
      actor_params[num].attack_neutral_frames = 12; // try something
      actor_params[num].attack_air_neutral_frames = 18; // try something
      actor_params[num].fall_force = 22;        // 0.11 (M, Grav)
      actor_params[num].fall_limit = 436;       // 2.13 (M)
      actor_params[num].fast_fall = 614;        // 3 (M)
      break;
  }
  actor_state[num].current_action=ACTION_STAND_BY_GROUND;
  actor_state[num].lives = 4;
}


char clock=0;
const byte* current_attrib;
void __fastcall__ irq_nmi_callback(void) 
{
  if(clock&0x01)
  {
    // copy attribute table from PRG ROM to VRAM
    vram_adr(NTADR_A(0,26));
    __asm__("jsr %v",vram_line);
    //vram_adr(NTADR_A(1,26));
    vram_adr(NTADR_A(0,25));
    __asm__("jsr %v",vram_line2);
    //print_state(0,NTADR_A(1,27));
    vram_adr(0x23C0+5*8+2);
    vram_write(current_attrib+3, 5);
  }
  else
  {
    // copy attribute table from PRG ROM to VRAM
    vram_adr(NTADR_C(0,26)+1);
    __asm__("jsr %v",vram_line);
    //vram_adr(NTADR_A(1,26));
    vram_adr(NTADR_C(0,25)+1);
    __asm__("jsr %v",vram_line2);
    //print_state(0,NTADR_A(1,27));
    vram_adr(0x2BC0+5*8+2);
    vram_write(current_attrib+3, 5);
  }
    
    


}

// main function, run after console reset
void main(void) {
  unsigned char newclock=0;
  unsigned char simulate_i=0;
  struct state* current_simulate_index_state;
  unsigned char pad = 0;
  unsigned char last_pad = 0;
  unsigned char pad_rising = 0;
  unsigned char pad_falling =0;
  unsigned char pad2 = 0;
  unsigned char last_pad2 = 0;
  unsigned char pad_rising2 = 0;
  unsigned char pad_falling2 =0;
  /*char pad3 = 0;
  char last_pad3 = 0;
  char pad_rising3 = 0;
  char pad_falling3 =0;
  */

  bool demo_mode_on = true;
  bool player1joined = false;
  bool player2joined = false;
  //bool player3joined = false;
  
  register unsigned char i;
  current_simulate_index_state=actor_state;
  
  current_effect=effects;
  
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
    vram_adr(NTADR_C(1,27)+1);
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
    unsigned char oam_id; // sprite ID
    byte background_color=sky_default;//0x00;//0x13;//0x03;//0x1c;
    bool resetLives=false;
    byte deadCount=0;
    byte x_avg_tmp=0;
    byte camera_offset_x;
    if(scroll_nudge_x>0)
    {
    scroll_nudge_x--; //todo: move out of zp
    }
    else if(scroll_nudge_x<0)
    {
      scroll_nudge_x++;
    }
    
    APU.pulse[0].control=0xff;
    
    // Test fourscore
    
    //last_pad = fourscore_pad1;
    //fourscore_read();
    // pad = fourscore_pad1;
    
    // Controls
    last_pad = pad;
    pad = pad_poll(0);
    pad_rising = pad^last_pad&pad;
    pad_falling = pad^last_pad&last_pad;
    
    last_pad2 = pad2;
    pad2 = pad_poll(1);
    pad_rising2 = pad2^last_pad2&pad2;
    pad_falling2 = pad2^last_pad2&last_pad2;
    
    //last_pad3 = pad3;
    //pad3 = pad_poll(2);
    //pad_rising3 = pad3^last_pad3&pad3;
    //pad_falling3 = pad3^last_pad3&last_pad3;
    
    
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
    
    /*if(!player3joined && pad3 & PAD_START)
    {
      demo_mode_on=false;
      actor_state[2].isAI=false;
      actor_state[2].current_action=ACTION_SPAWNING;
      player3joined=true;
      resetLives=true;
    }
    */
    
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
        current_effect->type=WIN;
        current_effect->variant=0;
        current_effect->x=actor_x[0]+4;
        current_effect->y=actor_y[0]-8;
        current_effect->frames=140;
        current_effect->isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
          current_effect=effects;
        }
        else
        {
          current_effect++;
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
        current_effect->type=WIN;
        current_effect->variant=1;
        current_effect->x=actor_x[1]+4;
        current_effect->y=actor_y[1]-8;
        current_effect->frames=140;
        current_effect->isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
          current_effect=effects;
        }
        else
        {
          current_effect++;
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
        current_effect->type=WIN;
        current_effect->variant=2;
        current_effect->x=actor_x[2]+4;
        current_effect->y=actor_y[2]-8;
        current_effect->frames=140;
        current_effect->isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
          current_effect=effects;
        }
        else
        {
          current_effect++;
        }
      }
      else
      {
        initialize_player(2,0,170,99-0);
      }
      actor_state[2].current_action=ACTION_SPAWNING;
      actor_state[2].current_action_frames=140;
      #endif
      
      #if NUM_ACTORS>3
      
      if(actor_state[3].lives>0)
      {
        initialize_player(3,0,140,99-20);
        actor_state[3].wins+=1;
        current_effect->type=WIN;
        current_effect->variant=3;
        current_effect->x=actor_x[3]+4;
        current_effect->y=actor_y[3]+8;
        current_effect->frames=140;
        current_effect->isNew=true;
        current_effect_index++;
        if(current_effect_index==4)
        {
          current_effect_index=0;
          current_effect=effects;
        }
        else
        {
          current_effect++;
        }
      }
      else
      {
        initialize_player(3,0,140,99-20);
      }
      actor_state[3].current_action=ACTION_SPAWNING;
      actor_state[3].current_action_frames=140;
      #endif
      
      {
        a_state=actor_state;
        update_player_wins(vram_line+6);
        
        #if NUM_ACTORS >1
        a_state++;
        update_player_wins(vram_line+13);
        #endif

        #if NUM_ACTORS >2
        a_state++;
        update_player_wins(vram_line+20);
        #endif

        #if NUM_ACTORS >3
        a_state++;
        update_player_wins(vram_line+27);
        #endif
      }
    
      
      resetLives=true;
    }
    
    if(resetLives)
    {
      actor_state[0].damage=0;
      actor_state[0].lives=4;
      #if NUM_ACTORS >1
      actor_state[1].damage=0;
      actor_state[1].lives=4;
      #endif
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
    
    // 9
    
    // Control of player intents based on controller or simulation.
    {
      // Reset left/right.
      if(player1joined)
      {
        if(pad & PAD_LEFT && !(pad & PAD_RIGHT))
        {
            actor_intent_dir[0] = DIR_LEFT;
        }
        else if(pad & PAD_RIGHT)
        {
            actor_intent_dir[0] = DIR_RIGHT;
        }
        else
        {
          actor_intent_dir[0] = DIR_NONE;
        }

        // Jump / cancel jump when state changes. Let simulation update consume intent in between.
        if(pad_rising & PAD_A)
        {
          actor_intent_jump[0] = true;
        }
        if((pad & PAD_A )==false && actor_intent_jump[0]==true)
        {
          actor_intent_jump[0] = false;
        }

        actor_intent_fast_fall[0] = pad & PAD_DOWN?true:false;
        if(pad_rising & PAD_DOWN)
        {
          actor_intent_crouch[0] = true;
        }
        else if (pad_falling & PAD_DOWN)
        {
          actor_intent_crouch[0] = false;
        }

        if(pad_rising & PAD_B)
        {
          actor_intent_attack[0] = true;
        }
      }
      if(player2joined)
      {
        if(pad2 & PAD_LEFT && !(pad2 & PAD_RIGHT))
        {
            actor_intent_dir[1] = DIR_LEFT;
        }
        else if(pad2 & PAD_RIGHT)
        {
            actor_intent_dir[1] = DIR_RIGHT;
        }
        else
        {
          actor_intent_dir[1] = DIR_NONE;
        }

        // Jump / cancel jump when state changes. Let simulation update consume intent in between.
        if(pad_rising2 & PAD_A)
        {
          actor_intent_jump[1] = true;
        }
        if((pad2 & PAD_A )==false && actor_intent_jump[1]==true)
        {
          actor_intent_jump[1] = false;
        }

        actor_intent_fast_fall[1] = pad2 & PAD_DOWN?true:false;
        if(pad_rising2 & PAD_DOWN)
        {
          actor_intent_crouch[1] = true;
        }
        else if (pad_falling2 & PAD_DOWN)
        {
          actor_intent_crouch[1] = false;
        }

        if(pad_rising2 & PAD_B)
        {
          actor_intent_attack[1] = true;
        }
      }
      
      /*if(player3joined)
      {
        if(pad3 & PAD_LEFT && !(pad3 & PAD_RIGHT))
        {
            actor_intent[2].dir = DIR_LEFT;
        }
        else if(pad3 & PAD_RIGHT)
        {
            actor_intent[2].dir = DIR_RIGHT;
        }
        else
        {
          actor_intent[2].dir = DIR_NONE;
        }

        // Jump / cancel jump when state changes. Let simulation update consume intent in between.
        if(pad_rising3 & PAD_A)
        {
          actor_intent[2].jump = true;
        }
        if((pad3 & PAD_A )==false && actor_intent[2].jump==true)
        {
          actor_intent[2].jump = false;
        }

        actor_intent[2].fast_fall = pad3 & PAD_DOWN?true:false;
        if(pad_rising3 & PAD_DOWN)
        {
          actor_intent[2].crouch = true;
        }
        else if (pad_falling3 & PAD_DOWN)
        {
          actor_intent[2].crouch = false;
        }

        if(pad_rising3 & PAD_B)
        {
          actor_intent[2].attack = true;
        }
      }
      */

      if(num_ai>0)
      {
        do
        {
          if(simulate_i>=NUM_ACTORS-1)
          {
            simulate_i=0;
            current_simulate_index_state=actor_state;
          }
          else
          {
            simulate_i+=1;
            current_simulate_index_state++;
          }
        }while(current_simulate_index_state->isAI==false);
        
        simulate_player(simulate_i);
      }
    }
    

    
    //
    // SIMULATION LOOP
    //
    
    // 10
    
    // Reset pointers for first iteration. This is faster than indexing.
    a_state=actor_state;
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
      
      // 11
      
      // Reset outlines
      pal_col((i<<2)+1+16,0x0d);
      
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
            && (actor_intent_fast_fall[i] || actor_intent_jump[i] || actor_intent_dir[i]!=DIR_NONE)
           )||a_state->current_action_frames == 0)
        {
          a_state->current_action = ACTION_STAND_BY_AIR;
          a_state->current_action_frames=0;
        }
        if(clock&0x02)pal_col((i<<2)+1+16,0x32);
      } else
      {
        // Update average x for camera, reduced precision.
	x_avg_tmp+=actor_x[i] >> 2;
          
        cur_action=a_state->current_action;
        action_frames=a_state->current_action_frames;
        on_ground=ON_GROUND(cur_action);

        //Speedx value
        tmp_speed_x_value=*a_speed_x;
        tmp_target_speed_x = tmp_speed_x_value;
        tmp_speed_y_value=*a_speed_y;

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
        
        if(!on_ground)
        {
          
        }
        
        // State simulation

	if(a_state->current_action==ACTION_HIT_STUN_AIR 
           || a_state->current_action==ACTION_HIT_STUN_GROUND)
        {
          if(a_state->current_attack_frames_left==0)
          {
            cur_action=ACTION_STAND_BY_GROUND;
          }
          a_state->current_attack=ATTACK_NONE;
          tmp_speed_y_value +=a_params->fall_force; 
        }
        else if (!on_ground) // on air
        {
          // Reset crouch intent on air.
          actor_intent_crouch[i] = false;
          // Fall speed
          tmp_speed_y_value +=a_params->fall_force; 

          if(actor_intent_fast_fall[i] && tmp_speed_y_value > 0)
          {
            tmp_speed_y_value= a_params->fast_fall;
          }
          else
          {
            tmp_speed_y_value = MAX(tmp_speed_y_value,a_params->fall_limit);
          }

          // Air attack
          if(actor_intent_attack[i] && a_state->current_attack==ATTACK_NONE) // todo: switch to check animated/cancelable attack
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
          if(actor_intent_jump[i])
          {
            if(a_state->double_jumps_left>0)
            {
              tmp_speed_y_value = -a_params->jump_force; 
              a_state->double_jumps_left-=1;
            }
            actor_intent_jump[i] = false;
          }

          if(cur_action==ACTION_STAND_BY_AIR)
          {
            if(actor_intent_dir[i] == DIR_LEFT)
            {
              tmp_target_speed_x=-a_params->run_speed;
            }
            else if(actor_intent_dir[i] == DIR_RIGHT)
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
          actor_intent_fast_fall[i]=false;

          // Initiate attack from any non-animated state.
          // Set action to stand by ground.
          if(actor_intent_attack[i] 
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
            if(actor_intent_dir[i] != DIR_NONE)
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
              if(actor_intent_dir[i] == DIR_LEFT)
              {
                tmp_target_speed_x=-a_params->walk_speed; // TODO: gradual change
              }
              else if(actor_intent_dir[i] == DIR_RIGHT)
              {
                tmp_target_speed_x=a_params->walk_speed;
              }
            }
          }
          if(cur_action==ACTION_RUNNING_GROUND) // depends on above
          {

            // change action to dash if attacking when running
            if(actor_intent_attack[i])
            {
              cur_action=ACTION_DASHING_GROUND;
              action_frames=0;
            }
            else if(actor_intent_dir[i] == DIR_LEFT)
            {
              tmp_target_speed_x=-a_params->run_speed; // TODO: gradual change
            }
            else if(actor_intent_dir[i] == DIR_RIGHT)
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
              if(actor_intent_dir[i] == a_state->moving_dir)
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
            if(actor_intent_crouch[i])
            {
              cur_action=ACTION_CROUCHING_GROUND;
              action_frames=0;
            }
          }

          if(cur_action==ACTION_CROUCHING_GROUND
            && actor_intent_crouch[i]!=true)
          {
            cur_action=ACTION_STAND_BY_GROUND;
            action_frames=0;
          }

          if(actor_intent_jump[i])
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
              tmp_speed_y_value = -a_params->jump_force;
              actor_intent_jump[i] = false;
              cur_action = ACTION_STAND_BY_AIR;
              a_state->double_jumps_left=1;
              action_frames=0;
            }
          }
          // Do short jump when cancelling jump early.
          else if (cur_action==ACTION_CROUCHING_TO_JUMP_GROUND)
          {
            tmp_speed_y_value = -a_params->short_hop_force;
            actor_intent_jump[i] = false;
            cur_action = ACTION_STAND_BY_AIR;
            a_state->double_jumps_left=1;
            action_frames=0;
          }
          // Note: state might not be on ground anymore.
        } // both air and ground behavior covered.
      
        // Inertia when slowing down
        // TODO: make state specific.
        // TODO: Implement target speed instead of only targeting to 0
        if(actor_intent_dir[i] == DIR_NONE)
        {
          tmp_target_speed_x = 0;
        }

        if(a_state->hit_lag_frames_left<=1)
        {
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
          *a_speed_y=tmp_speed_y_value;
        }

        actor_intent_attack[i] = false;
        a_state->current_action=cur_action;
        ++action_frames;
        a_state->current_action_frames=MIN(action_frames,255);
      }
      if(i<NUM_ACTORS)
      {
        a_state++;
        a_params++;
        a_speed_x++;
        a_speed_y++;
      }
    }
    
    // Count average and convert to screen x.
    switch (4-deadCount) {
      case 0: x_avg_tmp = 0;break;
      case 1: x_avg_tmp = x_avg_tmp; break;
      case 2: x_avg_tmp = x_avg_tmp << 1; break;
      case 3: x_avg_tmp = x_avg_tmp + (x_avg_tmp>>2) + (x_avg_tmp>>4); break;
      case 4: x_avg_tmp = x_avg_tmp; break;
    }
    x_avg_tmp = ((x_avg_tmp >> 2)&0b11111111) - 32;
    // Clamp to ±16
    if (x_avg_tmp < 128) {
        // Positive side (0-31 range)
        if (x_avg_tmp > 16) x_avg_tmp = 16;
    } else {
        // Negative side (224-255 range, i.e. -32 to -1)
        if (x_avg_tmp < 240) x_avg_tmp = 240;  // 240 = -16 as byte
    }
    //if( (clock & 0x01) == 0)
    {
      // Smooth toward target (byte subtraction handles direction)
      byte delta = x_avg_tmp - camera_offset_x;

      // Deadzone: only move if delta > 1 or delta < 255 (i.e., -1)
      if (delta > 0 && delta < 128) {
          camera_offset_x++;  // target is right
      } else if (delta > 128 && delta < (255-0)) {
          camera_offset_x--;  // target is left (delta is "negative")
      }
    }
    current_attrib = get_attrib_ptr(camera_offset_x);

    
    // 12
    
    // Actor State and intent physics
    
    a_state=actor_state;
    a_params=actor_params;
    a_speed_x=actor_speedx;
    a_speed_y=actor_speedy;
    //a_sprite=actor_sprite;

    // Actor State and intent physics
    for (i=0; i<NUM_ACTORS; i++) 
    {
      
      bool on_ground=false;
      enum action_state cur_action=a_state->current_action;
      
      
      
      o_state = actor_state; // Reset opponent state to first actor
              
      // Process attacks (from others): 3 scanlines
      if(a_state->current_action!=ACTION_SPAWNING)
      {
        bool isCrouching = a_state->current_action==ACTION_CROUCHING_GROUND;
        for(k = 0; k<NUM_ACTORS;k++) 
        { 
          // k is attacking player id
          // i is current player
          // todo: move the attack to be calculated only once.
          log_process_attacks(0);
          if(k==i || a_state->hit_lag_frames_left>0 || !o_state->hit_lag_frames_left&0x80) // skip attacking self, or while hit_lagging
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
            byte offset_y1; //offsets from player coordinate
            byte offset_y2;
            byte offset_x1;
            byte offset_x2;
            byte attackFrame;
            bool hitboxActive=false;
            
            attackFrame=o_state->current_attack_frames_left;
            
            switch(tmp_attack_type)
            {
              case ATTACK_NORMAL_RIGHT:
                offset_y1 = 0;
                offset_y2 = 6;
                offset_x1 = 10;
                offset_x2 = 18;
                // 6 frames: active on 4 and 3, counting down.
                if(attackFrame==4||attackFrame==3)
                {
                  hitboxActive=true;
                }
                break;
              case ATTACK_NORMAL_LEFT:
                offset_y1 = 0;
                offset_y2 = 6;
                offset_x1 = 256-2;
                offset_x2 = 6;
                if(attackFrame==4||attackFrame==3)
                {
                  hitboxActive=true;
                }
                //hitboxActive=true;
                break;
              case ATTACK_AIR_NEUTRAL_RIGHT:
                offset_y1 = 6;
                offset_y2 = 18;
                offset_x1 = 8;
                offset_x2 = 19;
                // 12 frames. Active on frames 10-8
                if(attackFrame<=10&&attackFrame>=8)
                {
                  hitboxActive=true;
                }
                //hitboxActive=true;
                break;
              case ATTACK_AIR_NEUTRAL_LEFT:
                offset_y1 = 6;
                offset_y2 = 16;
                offset_x1 = 256-3;
                offset_x2 = 8;
                // 16 frames. Active on frames 10-7
                if(attackFrame<=10&&attackFrame>=7)
                {
                  hitboxActive=true;
                }
                break;
            }
            
            if(!hitboxActive)
            {
              o_state++;
              continue;
            }
            else
            {
              pal_col((k<<2)+1+16,0x26);
            }
            
            // 13
            
            // Attack box y
            attack_y1=actor_y[k]+offset_y1;
            attack_y2=actor_y[k]+offset_y2;
            // Hit box y comparison
            if(actor_y[i]<attack_y2 // attack bottom lower than head
              && (byte)(actor_y[i]+17) > attack_y1 // feet lower than attack top
              )
            {
              attack_x1=actor_x[k]+offset_x1;
              attack_x2=actor_x[k]+offset_x2;
              if((byte)(actor_x[i]+12)>attack_x1 // actor hitbox comparison
                 && (byte)(actor_x[i]+4)<attack_x2
                )
              {
                byte damage;
                switch(tmp_attack_type)
                {
                  case ATTACK_NORMAL_RIGHT:
                    current_effect->type=HIT;
                    current_effect->variant=4; // apply flip
                    current_effect->x=attack_x1+6;
                    current_effect->y=attack_y1;
                    current_effect->isNew=true;
                    //attack_force_x=20+isCrouching?a_state->damage>>1:a_state->damage;
                    //attack_force_y=-(20+(isCrouching?a_state->damage:a_state->damage<<1));
                    attack_force_x=(byte)(30+a_state->damage);
                    attack_force_y=-((byte)(30+a_state->damage)<<1);
                    scroll_nudge_x+=2;
                    damage=11;
                    break;
                  case ATTACK_NORMAL_LEFT:
                    current_effect->type=HIT;
                    current_effect->variant=0;
                    current_effect->x=attack_x1-6;
                    current_effect->y=attack_y1;
                    current_effect->isNew=true;
                    //attack_force_x=-20-(isCrouching?a_state->damage>>1:a_state->damage);
                    //attack_force_y=-(20+(isCrouching?a_state->damage:a_state->damage<<1));
                    attack_force_x=-(byte)(30+a_state->damage);
                    attack_force_y=-((byte)(30+a_state->damage)<<1);
                    scroll_nudge_x-=2;
                    damage=11;
                    break;
                  case ATTACK_AIR_NEUTRAL_RIGHT:
                    current_effect->type=HIT;
                    current_effect->variant=4;
                    current_effect->x=attack_x1+2;
                    current_effect->y=attack_y1;
                    current_effect->isNew=true;
                    // Must drop (byte) if using values over 55 due to overflow. 
                    // Damage capped at 200 (BCD DED * 1.25)
                    // See update_all_hud()
                    attack_force_x=(byte)(50+a_state->damage); 
                    attack_force_y=-((byte)(20+a_state->damage)<<1);
                    damage=7;
                    scroll_nudge_x+=2;
                    break;
                  case ATTACK_AIR_NEUTRAL_LEFT:
                    current_effect->type=HIT;
                    current_effect->variant=0;
                    current_effect->x=attack_x1-2;
                    current_effect->y=attack_y1;
                    current_effect->isNew=true;
                    attack_force_x=-(byte)(50+a_state->damage);
                    attack_force_y=-((byte)(20+a_state->damage)<<1);
                    scroll_nudge_x-=2;
                    damage=7;
                    break;
                }
                a_state->damage+=damage;
                
                o_state->hit_lag_frames_left=4+0x80; 
                // Crouch cancel
                if(isCrouching)
                {
                  a_state->hit_lag_frames_left=((1+4)>>1)+0x80; //(damage/3+4)/2, Use highest bit to signify first frame to make it fair.
                }
                else
                {
                  a_state->hit_lag_frames_left=(1+4)+0x80; //damage/3+4;
                }
                a_state->damage_vis_frames+=3;

                if(isCrouching)
                {
                  actor_speedy[i]=attack_force_y;
                  actor_speedx[i]=attack_force_x<<1;
                }
                else
                {
                  actor_speedy[i]=attack_force_y<<1;
                  actor_speedx[i]=(attack_force_x<<2); // eliminate extra jsr
                }
   
                current_effect->frames=6;
                current_effect->variant+=i; // Apply player color
                current_effect_index++;
                if(current_effect_index==4)
                {
                  current_effect_index=0;
                  current_effect=effects;
                }
                else
                {
                  current_effect++;
                }
                
                if(on_ground)
                {
                  a_state->current_action=ACTION_HIT_STUN_GROUND;
                }
                else
                {
                  a_state->current_action=ACTION_HIT_STUN_AIR;
                }
                {
                  //reuse attack frame for hit stun.
                  byte stunframes=((byte)(attack_force_x>0?attack_force_x:-attack_force_x)>>2)+4;
                  a_state->current_attack_frames_left=MIN(100,stunframes);
                }
                switch(i)
                {
                  case 3:
                    background_color=0x16;
                    break;
                  case 2:
                    background_color=0x1a;
                    break;
                  case 1:
                    background_color=0x28;
                    break;
                  case 0:
                    background_color=0x21;
                    break;
                   
                }
              }
            }
          }
          o_state++;
        } 
      }
    
      
      // 14
      
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


        if(a_state->hit_lag_frames_left==0 || a_state->current_action==ACTION_SPAWNING)
        {
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
          for(j=0;j<p_count;++j) // heavy on air. 2,5 scanlines min, 5.5 max?
          {
            bool skip_due_to_fall_through;

            log_collision_calculation(0);

            falling = *a_speed_y >= 0; // may update for each platform
            actor_feet_x = actor_x[i]+8; // may update for each platform
            actor_feet_y = actor_y[i]+17; // may update for each platform

            speed_y_in_pixels=(*a_speed_y>>8);
            on_platform=
              (byte)(actor_feet_y+speed_y_in_pixels)>=platform_y1[j]
               && actor_feet_y<=platform_y2[j]
               && actor_feet_x>platform_x1[j]
               && actor_feet_x<platform_x2[j];

            // Side collision and grab
            if(!on_platform)
            {
              if(platform_has_edge[j])
              {
                byte grab_box_x1;
                byte grab_box_x2;
                byte grab_box_y;

                // collision to edge
                if(actor_y[i]<platform_y2[j]
                   && actor_feet_y>platform_y1[j]
                  )
                {
                  if(actor_feet_x>platform_x1[j]
                    && actor_feet_x<(byte)(platform_x1[j]+8))
                  {
                    actor_x[i]=platform_x1[j]-8;
                  } 
                  else if (actor_feet_x<platform_x2[j]
                           && actor_feet_x>(byte)(platform_x2[j]-8))
                  {
                    actor_x[i]=platform_x2[j]-8;
                  }
                }

                // grab

                grab_box_x1=actor_feet_x-8; //Todo: precalculate? check close enough first?
                grab_box_x2=actor_feet_x+8;
                grab_box_y=actor_y[i];

                if(falling 
                   && grab_box_y>=platform_y1[j]
                   && grab_box_y<=platform_y2[j]
                   && !actor_intent_jump[i] 
                   && !actor_intent_fast_fall[i] 
                   && !actor_intent_crouch[i] // drop
                   )
                {
                  if(actor_intent_dir[i]!=DIR_LEFT // right+neutral dir
                     && grab_box_x2>platform_x1[j]
                     && grab_box_x1<platform_x1[j]
                    )
                  {
                    *a_speed_x=0;*a_speed_y=0;
                    actor_y[i]=platform_y1[j];
                    actor_x[i]=platform_x1[j]-11;
                    a_state->current_action = ACTION_HANGING_GROUND;
                    a_state->facing_dir = DIR_RIGHT;
                    on_ground = true;
                  }
                  else if(actor_intent_dir[i]!=DIR_RIGHT // left + neutral dir
                     && grab_box_x1<platform_x2[j]
                     && grab_box_x2>platform_x2[j])
                  {
                    *a_speed_x=0;*a_speed_y=0;
                    actor_y[i]=platform_y1[j];
                    actor_x[i]=platform_x2[j]-5;
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
                skip_due_to_fall_through=(platform_can_fall_through[j] && (a_state->current_action==ACTION_CROUCHING_GROUND || a_state->fall_through_triggered));
                if(skip_due_to_fall_through)
                {
                  a_state->fall_through_triggered = true;
                }
                else
                {
                  actor_y[i] = platform_y1[j]-17;
                  *a_speed_y = 0;
                  actor_yf[i] = 0;
                  on_ground = true;
                }
              }
              // todo: split condition to improve perf
              // on_edge state update based on facing dir
              if(((a_state->facing_dir == DIR_LEFT && actor_feet_x<(byte)(platform_x1[j]+6)) 
                  || (a_state->facing_dir == DIR_RIGHT && actor_feet_x>(byte)(platform_x2[j]-6)))
                 )
              {
                on_edge=true;
              }
            }


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
        }
        else
        {
          a_state->hit_lag_frames_left=(a_state->hit_lag_frames_left&0x7f)-1;
        }
        
	log_sprite_selection(0);
        // Select sprite
        // TODO: deduplicate
        // todo: improve facing difection.
        // Compute variant once (palette = i, flip if facing left)
        actor_variant[i] = (a_state->facing_dir == DIR_RIGHT) ? i : (i | 4);

        if(a_state->lives == 0)
        {
            actor_tile[i] = CHAR1_FAST_FALL; // Use fast falling sprite for dead.
        }
        else if(a_state->current_action == ACTION_SPAWNING)
        {
            actor_tile[i] = CHAR1_FAST_FALL; // Use fast falling sprite for spawn.
        }
        else
        {
            if(action_on_ground)
            {
                if(a_state->current_attack != ATTACK_NONE)
                {
                    actor_tile[i] = CHAR1_NEUTRAL;
                }
                else if(cur_action==ACTION_RUNNING_GROUND)
                {
                    actor_tile[i] = CHAR1_RUN;
                }
                else if(cur_action==ACTION_CROUCHING_GROUND)
                {
                    actor_tile[i] = CHAR1_CROUCH;
                }
                else if(cur_action==ACTION_STAND_BY_GROUND 
                        || cur_action==ACTION_WALKING_GROUND)
                {
                    if(a_state->on_edge)
                    {
                        actor_tile[i] = CHAR1_SWAY;
                    }
                    else
                    {
                        actor_tile[i] = CHAR1_STAND;
                    }
                }
                else if(cur_action==ACTION_HANGING_GROUND)
                {
                    actor_tile[i] = CHAR1_LEDGE;
                }
                else if(cur_action==ACTION_DASHING_GROUND)
                {
                    actor_tile[i] = CHAR1_DASH;
                }
                else if(cur_action==ACTION_HIT_STUN_GROUND)
                {
                    actor_tile[i] = CHAR1_FLINCH;
                }
                else
                {
                    // Todo: handle jump crouch as part of animation instead of last case
                    actor_tile[i] = CHAR1_CROUCH;
                }
            }
            else
            {
                if(a_state->current_attack == ATTACK_AIR_NEUTRAL_LEFT
                   || a_state->current_attack == ATTACK_AIR_NEUTRAL_RIGHT)
                {
                    actor_tile[i] = CHAR1_AIRNEUTRAL;
                }
                else if(actor_intent_fast_fall[i]) // Todo: use state instead of intent.
                {
                    actor_tile[i] = CHAR1_FAST_FALL;
                }
                else if(cur_action==ACTION_HIT_STUN_AIR)
                {
                    actor_tile[i] = CHAR1_FLINCH;
                }
                else
                {
                    actor_tile[i] = CHAR1_JUMP;
                }
            }
        }
      }
      {
        bool do_respawn=false;
        if(actor_prev_y[i]>actor_y[i]
           && actor_speedy[i]>0
           )
        {
          current_effect->type=EXPLOSION_VERTICAL;
          current_effect->variant=4+i; // apply flip and player color
          current_effect->x=actor_x[i];
          current_effect->y=210;
          current_effect->frames=30;
          current_effect->isNew=true;
          current_effect_index++;
          if(current_effect_index==4)
          {
            current_effect_index=0;
            current_effect=effects;
          }
          else
          {
            current_effect++;
          }
          do_respawn=true;
        }
        else if(actor_prev_x[i]>actor_x[i]
                && actor_speedx[i]>0
                && actor_prev_x[i] >(255-32))
        {
          current_effect->type=EXPLOSION_HORIZONTAL;
          current_effect->variant=4+i; // apply flip and player color
          current_effect->x=255-20;
          current_effect->y=actor_y[i]-10;
          current_effect->frames=30;
          current_effect->isNew=true;
          current_effect_index++;
          if(current_effect_index==4)
          {
            current_effect_index=0;
            current_effect=effects;
          }
          else
          {
            current_effect++;
          }
          do_respawn=true;
        }
        else if( actor_prev_x[i]<actor_x[i]
                && actor_speedx[i]<0
                && actor_prev_x[i] < 32)
        {
          current_effect->type=EXPLOSION_HORIZONTAL;
          current_effect->variant=0+i; // apply player color
          current_effect->x=4;
          current_effect->y=actor_y[i];
          current_effect->frames=30;
          current_effect->isNew=true;
          current_effect_index++;
          if(current_effect_index==4)
          {
            current_effect_index=0;
            current_effect=effects;
          }
          else
          {
            current_effect++;
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
          a_state->current_action_frames=140;
          a_state->current_attack=ATTACK_NONE;
          a_state->damage=0;

        }
      }
            
      if(i<NUM_ACTORS)
      {
        a_state++;
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
      register struct effect* current_effect;
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
            zp_x = current_effect->x-camera_offset_x;
            oam_id = oam_meta_spr(zp_x, current_effect->y, oam_id, *a_sprite);
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
    
    // 15
    
  // Update Actor Sprites
  {
      char parallax_offset;
      char parallax_offset2;
      a_state = actor_state;

      // draw and move all actors
      for (i = 0; i < NUM_ACTORS; i++) 
      {
          zp_x = actor_x[i] - camera_offset_x;
          if (a_state->lives != 0)
          {
              if (a_state->hit_lag_frames_left > 0 && a_state->damage_vis_frames > 0)
              {
                  oam_id = oam_spr_2x2(
                      (byte)(zp_x + hit_lag_random_shake[(clock + i << 2) & 0x0f]),
                      actor_y[i] + hit_lag_random_shake[(clock + 13 + i << 2) & 0x0f],
                      oam_id,
                      actor_variant[i],
                      actor_tile[i]
                  );
              }
              else
              {
                  oam_id = oam_spr_2x2(zp_x, actor_y[i], oam_id, actor_variant[i], actor_tile[i]);
              }
          }
          a_state++;
      }
      a_state = actor_state;
      
      // Scale parallax correctly
      if (camera_offset_x < 128) {
        parallax_offset = camera_offset_x >> 4;
        parallax_offset2 = (byte)(camera_offset_x+(byte)(camera_offset_x>>1)) >> 4;
      } else {
        // Masks: >>1:0x80, >>2:0xC0, >>3:0xE0, >>4:0xF0 (Sign Extension)
        parallax_offset = 0xF0 | (camera_offset_x >> 4);  // Preserve sign bits
      	parallax_offset2 = 0xF0 | ((byte)(camera_offset_x+(byte)(0xF0 | camera_offset_x>>1)) >> 4);  // Preserve sign bits

      }
      

      for (i=0; i<NUM_ACTORS; i++) 
      {
        zp_x=icon_pos_x[i]-camera_offset_x;
        
        if(a_state->isAI)
        {
          oam_id = oam_spr_2x2(zp_x-parallax_offset2+(a_state->damage_vis_frames>>2), 189-(a_state->damage_vis_frames),oam_id, SPR_VARIANT(i,0), AI_ICON);

        }
        else
        {
          oam_id = oam_spr_2x2(zp_x-parallax_offset2+(a_state->damage_vis_frames>>2), 189-(a_state->damage_vis_frames),oam_id, SPR_VARIANT(i,0), CHAR1_ICON);

        }

        // move hearts on "behind" parallax plane and offset.
        zp_x+=parallax_offset+11+2;
        
        if(!(a_state->current_action==ACTION_SPAWNING
          && a_state->current_action_frames&0x1))
        {
          switch(a_state->lives)
          {
            case 0:
              oam_id = oam_spr_1x1(zp_x, 204, oam_id,SPR_VARIANT(i,0),LIVES0);
              break;
            case 1:
              oam_id = oam_spr_1x1(zp_x, 204, oam_id,SPR_VARIANT(i,0),LIVES1);
              break;
            case 2:
              oam_id = oam_spr_1x1(zp_x, 204, oam_id,SPR_VARIANT(i,0),LIVES2);
              break;
            case 3:
              oam_id = oam_spr_1x1(zp_x, 204, oam_id,SPR_VARIANT(i,0),LIVES3);
              break;
            case 4:
              oam_id = oam_spr_1x1(zp_x, 204, oam_id,SPR_VARIANT(i,0),LIVES4);
              break;
          }
        }
        a_state++;
      }
      // hide rest of sprites
      // if we haven't wrapped oam_id around to 0
      if (oam_id!=0) oam_hide_rest(oam_id);
    }
    
    update_all_hud();
    /*{
      a_state=actor_state;
      //update_debug_info2(0,vram_line);
      //update_player_status(vram_line2+4);
      update_player_status(vram_line2+4);

      #if NUM_ACTORS >1
      a_state++;
      //update_debug_info2(1,vram_line+7);
      update_player_status(vram_line2+11);
      #endif

      #if NUM_ACTORS >2
      a_state++;
      //update_debug_info2(2,vram_line+14);
      update_player_status(vram_line2+18);
      #endif

      #if NUM_ACTORS >3
      a_state++;
      //update_debug_info2(3,vram_line+23);
      update_player_status(vram_line2+25);
      #endif
    }*/
    
    
    
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
      PPU.control=(clock&0x01)?0b10000010:0b11000000;
      PPU.scroll=-scroll_nudge_x+0x00+(camera_offset_x)+((clock&0x01)<<3);
      PPU.scroll=0x02;
      PPU.mask =0b00011110;
    }
    

  }
}

//#link "fourscore.s"
