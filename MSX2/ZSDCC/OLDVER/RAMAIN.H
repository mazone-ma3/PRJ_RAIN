#ifndef RA_MAIN_H_INCLUDE
#define RA_MAIN_H_INCLUDE

#define CHRPAL_NO 0
#define REVPAL_NO 1
#define BGPAL_NO 2

void bg_roll(void);

short game_run(short);

void wait_vsync(void);
int opening_demo(void);
int title_demo(void);
void init_star(void);
void set_sprite(void);
void put_titlelogo(short x, short y);
void score_displayall(void);
void set_sprite(void);

extern unsigned short title_index;

//extern unsigned char tmp_spr_count;

#endif
