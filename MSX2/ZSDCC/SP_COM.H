
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>


#include "sp.h"
//#include "subfunc.h"
#include "ramain.h"


#include "list.h"

/*#include "sp_ene.h"*/
#include "sp_sht.h"
#include "sp_ene.h"

/* プロトタイプ指定 */

void game_init(void);		/* ゲームの初期化 */
short game_loop(void);		/* ゲーム本体 */

void wait_vsync(void);
void set_sprite(void);

#define FALSE 0
#define TRUE 1

#define CHRPAL_NO 0
#define REVPAL_NO 1

#define INIT_HSC 5000

unsigned char scrlspd = 0;
long score = 0, hiscore = INIT_HSC;

#define SCREEN2 0

unsigned char hit_check = TRUE;//FALSE;

typedef void (*SUBFUNC)(void);//(unsigned char i); 


#define DMGTIME 3
#define SHOTCOUNT 32
#define URAMAX 4

#define SCORE_MAX 99999999L
#define STAGEMAX 99


/* スプライト表示デ－タをVRAMに書き込むマクロ */
/* 座標をシフトして書き込みデ－タは随時インクリメント */
#define PUT_SP( x, y, no, atr) {\
	write_vram_data(y); \
	write_vram_data(x); \
	write_vram_data(no); \
	write_vram_data(0); \
}

/*
1周目 普通
2周目 打ち返し
3周目 通常弾高速
4周目 打ち返し3方向
5周目以降 インターバルなし
*/

#define HP_MAX 10
#define SELENMAX 75

int command_num;
int waitcount = 0;
int schedule_ptr;
int *command;

unsigned char noshotdmg_flag;
unsigned char uramode;

unsigned char my_hp_flag = TRUE;

unsigned char seflag;


/* 自機情報 */
/* 自機デ－タ */
short my_data_x[2], my_data_y[2];
char my_data_xx[2], my_data_yy[2];
unsigned char my_data_pat_num[2], my_data_atr[2], my_data_count[2], my_data_hp[2];
unsigned char my_data_pal[2];

/* 自機弾情報 */
/* 自機弾座標管理 */
short myshot_x[MAX_MYSHOT];
short myshot_y[MAX_MYSHOT];
int myshot_xx[MAX_MYSHOT];
int myshot_yy[MAX_MYSHOT];
unsigned char myshot_pat_num[MAX_MYSHOT];

unsigned char myshot_next[MAX_MYSHOT + 2];
unsigned char myshot_free[MAX_MYSHOT + 2];

unsigned char mypal;
unsigned char mypal_dmgtime;
unsigned char my_movecount;
unsigned char my_hp;
unsigned char max_my_hp;

/* 敵情報 */
/* 敵座標管理 */
short teki_x[MAX_TEKI];
short teki_y[MAX_TEKI];

unsigned int tkcount[MAX_TEKI];		/* 敵移動用カウンタ */
unsigned char teki_pat[MAX_TEKI];			/* 敵パターン番号 */
unsigned char teki_dir[MAX_TEKI];				/* 敵移動方向保存用変数 */

unsigned char teki_next[MAX_TEKI + 2];			/* 敵表示フラグ */
unsigned char teki_free[MAX_TEKI + 2];			/* 敵表示フラグ */

unsigned int tkshotcount[MAX_TEKI];	/* 敵弾発射カウンタ */

unsigned char teki_hp[MAX_TEKI];
unsigned char teki_pal[MAX_TEKI];
unsigned char teki_dmgtime[MAX_TEKI];

unsigned char teki_move[MAX_TEKI];
unsigned char teki_shotinfo[MAX_TEKI];

unsigned char teki_num = 0;
unsigned char stage;
unsigned char tmp;
short timeup;
unsigned char trgcount = 0;	/* ショット間隔リミッタ */
unsigned char trgcount2 = 0;	/* 連射リミッタ */
unsigned char trgnum = 0;
unsigned char total_count = 0;
unsigned char renshaflag = TRUE; /*FALSE;*/

/* 敵弾情報 */
/* 敵弾座標管理 */
short tkshot_x[MAX_TKSHOT];
short tkshot_y[MAX_TKSHOT];

unsigned char tkshot_next[MAX_TKSHOT + 2];
unsigned char tkshot_free[MAX_TKSHOT + 2];

int tkshot_xx[MAX_TKSHOT];			/* 敵弾移動増分X */
int tkshot_yy[MAX_TKSHOT];			/* 敵弾移動増分Y */

unsigned char tkshot_pat[MAX_TKSHOT];

unsigned char tkshotnum;						/* 敵弾 現在の表示数 */
unsigned char tkshot_c;						/* 敵弾発射間隔 初期値 */

unsigned char scrdspflag;						/* スコア文字更新フラグ */

/* テンポラリ用グローバル変数 */
unsigned char chrnum;
unsigned char chrpal;
const SPR_COMB *sprcomb;

int tmp_x, tmp_y;
int tmp_xx, tmp_yy;

volatile unsigned char tmp_spr_count = 0;
//unsigned char spr_count[2];
unsigned char old_count[2];

COLIGION *p_coli;

short xxx;
unsigned char yyy, sprpal_no; // spratr, patno,

short X, Y;
unsigned char PAT, PAL;

void DEF_SP_SINGLE(void) { //short X, short Y, unsigned char PAT, unsigned char PAL) __sdcccall(1) {
//	patno = PAT;
//	pchr_data = &chr_data[tmp_spr_count];
	xxx = ((X >> SHIFT_NUM) + SPR_OFS_X);
	yyy = ((Y >> SHIFT_NUM_Y) + SPR_OFS_Y - 1);

//	spratr = PAL;
/*	if((xxx >= 256) || (xxx <= -16)){
		yyy = 212;
	}
	else if(xxx >= 0){
		spratr = PAL;
	}else{
		xxx += 32;
		spratr = 0x80 | PAL;
	}*/
__asm
	ld	a,(_PAL)
;	ld	(_spratr),a
	ld	b,a

	ld	a,(_yyy)
	ld	c,a

	ld	a,(_xxx+0)
	ld	d,a

	ld	a,(_xxx+1)
	or	a
	jr	z,spdefend2

	cp	#0xff
	jr	nz,spdef02
	ld	a,d ;(_xxx+0)
	cp	#0xf0
	jr	nc,spdef12
spdef02:
:	ld	a,212
;	ld	(_yyy),a
	ld	c,212
	jr	spdefend2
spdef12:
;	ld	a,32
;	ld	c,32
	ld	a,d ;(_xxx+0)
	add	a,32
	ld	d,a ;(_xxx+0),a
	ld	a,b ;(_spratr)
	or	a,#0x80
	ld	b,a	;(_spratr),a
spdefend2:

	ld	hl,(_pchr_data)
;	ld	a,c ;(_yyy)
	ld	(hl),c
	inc	hl
;	ld	a,d ;(_xxx)
	ld	(hl),d
;	inc	hl		; (*)
;	inc	de
;	ld	(hl),d
	inc	hl
;	ld	a,(_patno)
	ld	a,(_PAT)
	ld	(hl),a
	inc	hl
;	ld	a,b	;(_spratr)
	ld	(hl),b
	ld	hl,_tmp_spr_count
	inc	(hl)
__endasm;
/*
__asm
	ld	hl,_tmp_spr_count
	ld	a,(hl)
	cp	32
	jr	nc,defspend
	inc	(hl)

	ld	a,(_VDP_writeadr)
	ld	c,a

	ld	a,(_yyy)
	out	(c),a
	ld	a,(_xxx)
	out	(c),a
	ld	a,(_patno)
	out	(c),a
	out	(c),a
	ld	hl,(_pchr_data)
	ld	(hl),a
	inc	hl
	ld	a,(_spratr)
	ld	(hl),a
defspend:
__endasm;*/

	++pchr_data;
}

void DEF_SP_DOUBLE(void) { //short X, short Y, unsigned char PAT, unsigned char PAL) __sdcccall(1) {
//	patno = PAT * 8;
//	pchr_data = &chr_data[tmp_spr_count];
	xxx = ((X >> SHIFT_NUM) + SPR_OFS_X);
	yyy = ((Y >> SHIFT_NUM_Y) + SPR_OFS_Y - 1);

//	spratr = PAL;
/*	if((xxx >= 256) || (xxx <= -16)){
		yyy = 212;
	}
	else if(xxx >= 0){
		spratr = PAL;
	}else{
		xxx += 32;
		spratr = 0x80 | PAL;
	}*/
__asm
	ld	a,(_PAL)
;	ld	(_spratr),a
	ld	b,a

	ld	a,(_yyy)
	ld	c,a

	ld	a,(_xxx+0)
	ld	d,a

	ld	a,(_xxx+1)
	or	a
	jr	z,spdefend

	cp	#0xff
	jr	nz,spdef0
	ld	a,d
	cp	#0xf0
	jr	nc,spdef1
spdef0:
	ld	a,212
	ld	c,a ;(_yyy),a
	jr	spdefend
spdef1:
;	ld	a,32
;	ld	c,32
	ld	a,d ;(_xxx+0)
	add	a,32
	ld	(_xxx+0),a
	ld	a,b	;(_spratr)
	or	a,#0x80
;	ld	(_spratr),a
	ld	b,a
spdefend:
	ld	hl,4
	ld	de,(_pchr_data)
	add	hl,de
;	ld	a,c ;(_yyy)
	ld	(de),c
	ld	(hl),c
	inc	hl
	inc	de
	ld	a,(_xxx)
	ld	(de),a
	ld	(hl),a
;	inc	hl		; (*)
;	inc	de		; (*)
	inc	hl
	inc	de
;	ld	a,(_patno)
	ld	a,(_PAT)
	add	a,a
	add	a,a
	add	a,a
	ld	(de),a
;	ld	c,4
	add	a,4
	ld	(hl),a
	inc	hl
	inc	de
;	ld	a,b	;(_spratr)
	ld	(de),b
	ld	(hl),b

	ld	hl,_tmp_spr_count
	inc	(hl)
	inc	(hl)
__endasm;
/*
__asm
	ld	hl,_tmp_spr_count
	ld	a,(hl)
	cp	31
	jr	nc,defspend2
	ld	b,2
	add	a,b
	ld	(hl),a

;	ld	hl,_chr_data
;	ld	a,(_tmp_spr_count)
;	add	a,a
;	ld	e,a
;	ld	d,0
;	add	hl,de
;	ld	(_pchr_data),hl

	ld	a,(_VDP_writeadr)
	ld	c,a

	ld	a,(_yyy)
	out	(c),a
	ld	a,(_xxx)
	out	(c),a
	ld	a,(_patno)
	out	(c),a
	out	(c),a

	ld	hl,(_pchr_data)
	ld	(hl),a
	inc	hl
	ld	a,(_spratr)
	ld	(hl),a

	ld	a,(_yyy)
	out	(c),a
	ld	a,(_xxx)
	out	(c),a
	ld	a,(_patno)
	ld	b,4
	add	a,b
	out	(c),a
	out	(c),a
	ld	hl,(_pchr_data)
	ld	de,2
	add	hl,de
	ld	(hl),a
	inc	hl
	ld	a,(_spratr)
	ld	(hl),a
defspend2:
__endasm;*/

	pchr_data += 2;
}

#ifdef DEBUG2
void DEF_SP_FUNC(short X, short Y, unsigned char pat, unsigned char chrnum, unsigned char sprpal_no)  __sdcccall(1) 
{
//	chrnum = spr_info[pat].patmax;
//	sprcomb = spr_info[pat].data;
//	sprpal_no = pal_no;

//	DI();
//	write_vram_adr(spr_page, 0x7600 + tmp_spr_count * 4);
	patno = pat * 8;

	pchr_data = &chr_data[tmp_spr_count];

	while(chrnum--){
/*		DEF_SP_SINGLE(X + (sprcomb->x << SHIFT_NUM),
					 Y + (sprcomb->y << SHIFT_NUM_Y),
					sprcomb->patno, pal_no
					);*/
//		patno = sprcomb->patno;
//		xxx = (( (X + (sprcomb->x << SHIFT_NUM)) >> SHIFT_NUM) + SPR_OFS_X);
//		yyy = (((Y + (sprcomb->y << SHIFT_NUM_Y)) >> SHIFT_NUM_Y) + SPR_OFS_Y - 1);
		xxx = (X >> SHIFT_NUM) + SPR_OFS_X;
		yyy = (Y >> SHIFT_NUM_Y) + SPR_OFS_Y;
		if((xxx >= 256) ||(xxx <= -16)){
			yyy = 212;
//			spratr = 0x80 | pal_no;
		}
		else if(xxx < 0){
			xxx += 32;
			spratr = 0x80 | sprpal_no;
		}else{
			spratr = sprpal_no;
		}
		sprcomb++;
/*__asm
	push	bc
	push	hl
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,(_yyy)
	out	(c),a
	ld	a,(_xxx)
	out	(c),a
	ld	a,(_patno)
	out	(c),a
	xor	a,a
	out	(c),a
*/
__asm
	push	bc
	push	hl
	ld	hl,(_pchr_data)
	ld	a,(_yyy)
	ld	(hl),a
	inc	hl
	ld	a,(_xxx)
	ld	(hl),a
	inc	hl
	ld	a,(_patno)
	ld	(hl),a
	ld	b,4
	add	a,b
	ld	(_patno),a
	inc	hl
	ld	a,(_spratr)
	ld	(hl),a
	inc	hl
	ld	(_pchr_data),hl
	ld	hl,_tmp_spr_count
	inc	(hl)

	pop	hl
	pop	bc
__endasm;

/*
	ld	hl,(_pchr_data)
	ld	a,(_yyy)
	ld	(hl),a
	inc	hl
	ld	a,(_xxx)
	ld	(hl),a
;	inc	hl
;	inc	de
;	ld	(hl),d
	inc	hl
	ld	a,(_patno)
	ld	(hl),a
	inc	hl
	ld	a,(_atr)
	ld	(hl),a
	ld	hl,_tmp_spr_count
	inc	(hl)
__endasm;*/
//		pchr_data++;

//		patno += 4;
	}
//	EI();
}
#endif

/******************************************************************************/

/* 敵弾消去マクロ */
//	tkshot_y[NO] = tmp_y = SPR_DEL_Y;\

#define del_tkshot(NO) {\
	tkshotnum--;\
}

#define put_my_hp() my_hp_flag = TRUE;

unsigned char str_temp[11];

//int i,j;
unsigned char i,j, k, j2;
//int k;


void put_my_hp_dmg(void)
{
	unsigned char i, j = 0;

	if(my_hp < HP_MAX)
		for(i = my_hp; i < HP_MAX; i++)
			str_temp[j++] = ' ';
	for(i = 0; i < my_hp; i++)
		str_temp[j++] = '`';
	str_temp[j] = '\0';

	put_strings(SCREEN2, 13, 24, str_temp, CHRPAL_NO);

	my_hp_flag = TRUE;
}

void my_dmg(void)
{
	if(my_hp)
	{
		if(!mypal_dmgtime){
			--my_hp;
			put_my_hp();
			seflag = 1;
		}
	}
	mypal = REVPAL_NO;
	mypal_dmgtime = DMGTIME * 2;

	noshotdmg_flag = TRUE;
}


/*****************************/
/* 敵機弾移動方向決定(32way) */
/*****************************/

//int
unsigned char l = 0;

unsigned char tekishot_dir(void)
{
//	unsigned char l = 0;
	l = 0;

	tmp_yy = my_data_x[0] - tmp_x;
	tmp_xx = my_data_y[0] - tmp_y + scrlspd;

	if(tmp_xx < 0){
		tmp_xx = -tmp_xx;
		if(tmp_yy < 0){
			l = 16;
			tmp_yy = -tmp_yy;
		}else{
			l = 24;
		}
	}else if(tmp_yy < 0){
		l = 8;
		tmp_yy = -tmp_yy;
	}

	if((l == 24) || (l == 8)){
		if(tmp_yy < tmp_xx){
			tmp_yy *= 8;
			if((tmp_yy) < tmp_xx)
				l += 0;
			else if((tmp_yy) < tmp_xx * 3)
				l += 1;
			else if((tmp_yy) < (tmp_xx * 5))
				l += 2;
			else if((tmp_yy) < (tmp_xx * 7))
				l += 3;
			else
				l += 4;
		}else{
			tmp_xx *= 8;
			if((tmp_yy * 7) < (tmp_xx))
				l += 4;
			else if((tmp_yy * 5) < (tmp_xx))
				l += 5;
			else if((tmp_yy * 3) < (tmp_xx))
				l += 6;
			else if((tmp_yy) < (tmp_xx))
				l += 7;
			else{
				l += 8;
				l %= 32;
			}
		}
	}else{
		if(tmp_yy < tmp_xx){
			tmp_yy *= 8;
			if((tmp_yy) < tmp_xx){
				l += 8;
				l %= 32;
			}
			else if((tmp_yy) < (tmp_xx * 3)){
				l += 7;
			}
			else if((tmp_yy) < (tmp_xx * 5))
				l += 6;
			else if((tmp_yy) < (tmp_xx * 7))
				l += 5;
			else
				l += 4;
		}else{
			tmp_xx *= 8;
			if((tmp_yy * 7) < (tmp_xx))
				l += 4;
			else if((tmp_yy * 5) < (tmp_xx))
				l += 3;
			else if((tmp_yy * 3) < (tmp_xx))
				l += 2;
			else if(tmp_yy < (tmp_xx))
				l += 1;
		}
	}
	return l;
}

/**********************/
unsigned char dir, pat;

void shottome(void) //unsigned char dir, unsigned char pat)
{
//	int j;
//	unsigned char j;

	if(tkshot_free[MAX_TKSHOT] != END_LIST){
		ADD_LIST(MAX_TKSHOT, k, tkshot_next, tkshot_free);

		if(uramode > 1){
			tkshot_x[k] = tmp_x + (4 << SHIFT_NUM) + (tkshot_xx[k] = direction[dir][0] * 3 / 2);
			tkshot_y[k] = tmp_y + (4 << SHIFT_NUM_Y) + (tkshot_yy[k] = direction[dir][1] * 3 / 2);
		}else{
			tkshot_x[k] = tmp_x + (4 << SHIFT_NUM) + (tkshot_xx[k] = direction[dir][0]);
			tkshot_y[k] = tmp_y + (4 << SHIFT_NUM_Y) + (tkshot_yy[k] = direction[dir][1]);
		}
		if(pat == PAT_TKSHOT1)
			if(tkshot_xx[k] > 0)
				pat = PAT_TKSHOT3;
		tkshot_pat[k] = pat;
		tkshotnum++;
	}
}

void shottome2(void) //unsigned char dir, unsigned char pat)
{
//	int j;
//	unsigned char j;
//	return;

	if(tkshot_free[MAX_TKSHOT] != END_LIST){
		ADD_LIST(MAX_TKSHOT, k, tkshot_next, tkshot_free);
		tkshot_x[k] = tmp_x + (4 << SHIFT_NUM) + (tkshot_xx[k] = direction[dir][0] * 3 / 2);
		tkshot_y[k] = tmp_y + (4 << SHIFT_NUM_Y) + (tkshot_yy[k] = direction[dir][1] * 3 / 2);

		if(pat == PAT_TKSHOT1)
			if(tkshot_xx[k] > 0)
				pat = PAT_TKSHOT3;
		tkshot_pat[k] = pat;
		tkshotnum++;
	}

}

/**********************/
/* 敵機弾発射サブ関数 */
/**********************/
void shotcheck1(void)//(unsigned char i)	/* NORMAL */
{
	if(tkshotcount[i] != 0){
		tkshotcount[i]--;
		return;
	}

	tkshotcount[i] = tkshot_c;
	dir = tekishot_dir();
	pat = PAT_TKSHOT1;
	shottome(); //tekishot_dir(), PAT_TKSHOT1);
}

void shotcheck2(void)//(unsigned char i)	/* 3 WAY */
{
//	int l;
//	unsigned char l;

	if(tkshotcount[i] != 0){
		tkshotcount[i]--;
		return;
	}

	tkshotcount[i] = tkshot_c;
	l = tekishot_dir();

	dir = l;
	pat = PAT_TKSHOT1;
	shottome(); //l, PAT_TKSHOT1);
	dir = (l + 2) % 32;
	pat = PAT_TKSHOT1;
	shottome(); //(l + 2) % 32, PAT_TKSHOT1);
	dir = (l - 2 + 32) % 32;
	pat = PAT_TKSHOT1;
	shottome(); //(l - 2 + 32) % 32, PAT_TKSHOT1);
}

void shotcheck3(void)//(unsigned char i)	/* RANDOM */
{
	if(tkshotcount[i] != 0){
		tkshotcount[i]--;
		return;
	}

	tkshotcount[i] = tkshot_c;
	dir = rand() % 32;
	pat = PAT_TKSHOT1;
	shottome(); //rand() % 32, PAT_TKSHOT1);
}

void shotcheck4(void)//(unsigned char i)	/* LASER */
{
	if(tkshotcount[i] == 0){
		return;
	}
	if(tkshotcount[i]-- < (SHOTCOUNT - 1)){
		if(tkshotcount[i] & 2){
			dir = DIR_DOWN;
			pat = PAT_TKSHOT2;
			shottome(); //DIR_DOWN, PAT_TKSHOT2);
		}
	}
}

void dummy(void)//(unsigned char i)
{
}

/************/

#define SHOTCHECKMAX 7

SUBFUNC shotcheck_tbl[SHOTCHECKMAX] ={
	shotcheck1,
	shotcheck2,
	shotcheck3,
	shotcheck4,
	dummy,
	dummy,
	dummy,
};

void do_shotcheck(void)//(unsigned char i)
{
/*	SUBFUNC p;*/
//	int j;

	j = teki_shotinfo[i];
	if(j < SHOTCHECKMAX)
		if(j >= 0)
//			shotcheck1();
			shotcheck_tbl[j](); //(i);
}

/******************/
/* 敵機弾移動サブ */
/******************/

void move_tekishot(void)
{
//	int i, j, k;
//	unsigned char i, j, k;

	SEARCH_LIST2(MAX_TKSHOT, i, j, tkshot_next){
		k = tkshot_pat[i];
		tmp_x = (tkshot_x[i] += (tkshot_xx[i]));
//		tmp_y = tkshot[i].y;

//		DEF_SP(tmp_x, tmp_y, k, CHRPAL_NO);

		/* 敵機弾画面外消去 */
		if((unsigned short)(tmp_x) > ((SCREEN_MAX_X + 16) << SHIFT_NUM)){
			del_tkshot(i);
			tmp_y = tkshot_y[i];
			DEL_LIST(MAX_TKSHOT, i, j, tkshot_next, tkshot_free);
		}else{
			tmp_y = tkshot_y[i] += (tkshot_yy[i] - scrlspd);
			if((unsigned short)(tmp_y) > ((SCREEN_MAX_Y + 16) << SHIFT_NUM_Y)){
				del_tkshot(i);
				DEL_LIST(MAX_TKSHOT, i, j, tkshot_next, tkshot_free);

			}/* 敵機弾と自機の当たり判定→自機ダメージ */
			else{
//				if(hit_check == TRUE){

					 if((tmp_xx = my_data_x[0] - tmp_x) < ((TKD_OFS_X - MY_OFS_X + TKD_SIZE_X) << SHIFT_NUM)){
						if(tmp_xx > ((TKD_OFS_X - MY_OFS_X - MY_SIZE_X) << SHIFT_NUM)){
							if((tmp_yy = my_data_y[0] - tmp_y) < ((TKD_OFS_Y - MY_OFS_Y + TKD_SIZE_Y) << SHIFT_NUM_Y)){
								if(tmp_yy > ((TKD_OFS_Y - MY_OFS_Y - MY_SIZE_Y) << SHIFT_NUM_Y)){
									/* 自機ダメージ */
									del_tkshot(i);
									DEL_LIST(MAX_TKSHOT, i, j, tkshot_next, tkshot_free);
									my_dmg();
								}
							}
						}
//					}
				}
				X = tmp_x;
				Y =  tmp_y + (0 << SHIFT_NUM_Y);
				PAT = k * 8;
				PAL  = CHRPAL_NO;
				DEF_SP_SINGLE(); //tmp_x, tmp_y + (0 << SHIFT_NUM_Y), k * 8, CHRPAL_NO);
//				DEF_SP_SINGLE(tmp_x, tmp_y, 8 * 4, CHRPAL_NO);

			}
		}
	}
}

void check_revshot(void)
{
//	int l;
	return;
	if(uramode > 2){
		l = tekishot_dir();

		dir = l;
		pat = PAT_TKSHOT1;
		shottome2(); //l, PAT_TKSHOT1);
		dir = (l + 2) % 32;
		pat = PAT_TKSHOT1;
		shottome2(); //(l + 2) % 32, PAT_TKSHOT1);
		dir = (l - 2 + 32) % 32;
		pat = PAT_TKSHOT1;
		shottome2(); //(l - 2 + 32) % 32, PAT_TKSHOT1);
	}else{
		dir = tekishot_dir();
		pat = PAT_TKSHOT1;
		shottome2(); //tekishot_dir(), PAT_TKSHOT1);
	}
}

/***********************/
/* 敵爆発パターンその1 */
/***********************/
void move_teki_bomb(void)//(unsigned char i)
{
	switch(tkcount[i]){
		case 0:
			seflag = 1;
//			teki_pat[i] = PAT_BAKU1;
			teki_pal[i] = REVPAL_NO;
			break;

		case 5:
//			teki_pat[i] = PAT_BAKU2;
			teki_pal[i] = REVPAL_NO;
			break;

		case 10:
//			teki_pat[i] = PAT_BAKU3;
			teki_pal[i] = REVPAL_NO;
			if(uramode)
				check_revshot();
			break;

		case 13:
			if(uramode)
				check_revshot();
			break;

		case 15:
			if(uramode)
				check_revshot();
			teki_hp[i] = 0;
//			teki_y[i] = SPR_DEL_Y;
			break;
	}

	tkcount[i]++;
}

//#define DEBUG

//#ifndef DEBUG

void move_teki_pat0(void)//(unsigned char i)
{
//	tmp_x -= sinwave[tkcount[i] / SINRATE] * ((3 << SHIFT_NUM) / 2);

//	if(++tkcount[i] >= (SINFREQ * SINRATE))
//		tkcount[i] = 0;

//	tmp_x += sinwave[tkcount[i] / SINRATE] * ((3 << SHIFT_NUM) / 2);

	/* 敵機縦方向移動処理 */
//	if(!(tmp_y > (128 << SHIFT_NUM_Y)))
//		tmp_y += ((3 << SHIFT_NUM_Y) / 2);	/* ゆっくり */
//	else
		tmp_y += (2 << SHIFT_NUM_Y);	/* 高速 */

	/* 敵機画面外消去(下方向) */
	if(tmp_y > (SPR_MAX_Y)){
		teki_hp[i] = 0;
	}
}

/***********************/
/* 敵移動パターンその1 */
/* HENTAI SINWAVE      */
/***********************/
void move_teki_pat1(void)//(unsigned char i)
{
	tmp_x -= sinwave[tkcount[i] / SINRATE] * ((3 << SHIFT_NUM) / 2);

	if(++tkcount[i] >= (SINFREQ * SINRATE))
		tkcount[i] = 0;

	tmp_x += sinwave[tkcount[i] / SINRATE] * ((3 << SHIFT_NUM) / 2);

	/* 敵機縦方向移動処理 */
	if(!(tmp_y > (128 << SHIFT_NUM_Y)))
		tmp_y += ((3 << SHIFT_NUM_Y) / 2);	/* ゆっくり */
	else
		tmp_y += (2 << SHIFT_NUM_Y);	/* 高速 */

	/* 敵機画面外消去(下方向) */
	if(tmp_y > (SPR_MAX_Y)){
		teki_hp[i] = 0;
	}
}

/***********************/
/* 敵移動パターンその2 */
/* FIRE2               */
/***********************/
//int dir, 
unsigned char dir2;

void move_teki_pat2(void)//(unsigned char i)
{

	if(++tkcount[i] < 800){
	if(tmp_x < (16 << SHIFT_NUM))
		teki_dir[i] = tekishot_dir();

	if(tmp_y < (16 << SHIFT_NUM_Y))
		teki_dir[i] = tekishot_dir();

	if(tmp_x > (SCREEN_MAX_X << SHIFT_NUM))
		teki_dir[i] = tekishot_dir();

	if(tmp_y > (SCREEN_MAX_Y << SHIFT_NUM_Y))
		teki_dir[i] = tekishot_dir();
	}else{
		/* 敵機画面外消去(左方向) */
		if(tmp_x < 0)
			teki_hp[i] = 0;

		/* 敵機画面外消去(右方向) */
		if(tmp_x > (SPR_MAX_X))
			teki_hp[i] = 0;

		/* 敵機画面外消去(上方向) */
		if(tmp_y < 0)
			teki_hp[i] = 0;

		/* 敵機画面外消去(下方向) */
		if(tmp_y > (SPR_MAX_Y))
			teki_hp[i] = 0;
	}
	dir = teki_dir[i];
	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);
}

/***********************/
/* 敵移動パターンその3 */
/* GURUGURU            */
/***********************/
void move_teki_pat3(void)//(unsigned char i)
{
//	int dir, dir2;

	if(tkcount[i] > 800){
		dir = teki_dir[i];
		/* 敵機画面外消去(左方向) */
		if(tmp_x < 0)
			teki_hp[i] = 0;

		/* 敵機画面外消去(右方向) */
		if(tmp_x > (SPR_MAX_X))
			teki_hp[i] = 0;

	}else if((++tkcount[i] % 7) == 1){
		dir2 = tekishot_dir();
		dir = teki_dir[i];
//		if( (DIR_OFF + dir - dir2 + DIR_OFF/2) % DIR_OFF > (DIR_DOWN + DIR_OFF/2) )
		if((32 + dir - dir2) % 32 > 16)
			dir++;
		else
			dir--;
		dir = teki_dir[i] = (dir + DIR_OFF) % DIR_OFF;
	}else
		dir = teki_dir[i];

	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);

//	teki_pat[i] = teki5_pat[dir / 2];

	/* 敵機画面外消去(上方向) */
	if(tmp_y < 0)
		if(direction[dir][1] < 0)
			teki_hp[i] = 0;

	/* 敵機画面外消去(下方向) */
	if(tmp_y > (SPR_MAX_Y))
		teki_hp[i] = 0;


}

/***********************/
/* 敵移動パターンその4 */
/* FIRE1               */
/***********************/
void move_teki_pat4(void)//(unsigned char i)
{
//	int dir;

	if(tkcount[i] > 800){
		dir = teki_dir[i];
		tmp_x +=  (direction[dir][0]);
		tmp_y +=  (direction[dir][1]);

		/* 敵機画面外消去(左方向) */
		if(tmp_x < 0)
			teki_hp[i] = 0;

		/* 敵機画面外消去(右方向) */
		if(tmp_x > (SPR_MAX_X))
			teki_hp[i] = 0;

	}else if(!tkcount[i] || !(tkcount[i] % 192)){
		++tkcount[i];
		dir = teki_dir[i] = tekishot_dir();

		tmp_x +=  (direction[dir][0]);
		tmp_y +=  (direction[dir][1]);
	}else if(((tkcount[i] % 192) < 80)){ 
		++tkcount[i];
		dir = teki_dir[i];

		tmp_x +=  (direction[dir][0]);
		tmp_y +=  (direction[dir][1]);

		if(tmp_x < (16 << SHIFT_NUM))
			teki_dir[i] = tekishot_dir();
		if(tmp_y < (16 << SHIFT_NUM_Y))
			teki_dir[i] = tekishot_dir();
		if(tmp_x > (SCREEN_MAX_X << SHIFT_NUM))
			teki_dir[i] = tekishot_dir();
		if(tmp_y > (SCREEN_MAX_Y << SHIFT_NUM_Y))
			teki_dir[i] = tekishot_dir();

	}else
		++tkcount[i];

	/* 敵機画面外消去(上方向) */
	if(tmp_y < 0)
		if(direction[dir][1] < 0)
			teki_hp[i] = 0;

	/* 敵機画面外消去(下方向) */
	if(tmp_y > (SPR_MAX_Y))
		teki_hp[i] = 0;

}

/***********************/
/* 敵移動パターンその5 */
/* HERIZAKO            */
/***********************/
void move_teki_pat5(void)//(unsigned char i)
{
//	int dir;

	if(!tkcount[i]){
		dir = teki_dir[i] = tekishot_dir();
		++tkcount[i];

	}else{
		dir = teki_dir[i];
		if(dir != DIR_UP){
			if(++tkcount[i] > 102){
				tkcount[i] = 96;
				dir = DIR_UP;
//				if(direction[dir][0] > 0){
//					++dir;	/* 左回転 */
//					dir %= DIR_OFF;
//				}else{
//					--dir;	/* 右回転 */
//				}
				teki_dir[i] = dir;
			}
		}
	}
	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);


	/* 敵機画面外消去(上方向) */
	if(dir == DIR_UP){
		if(tmp_y < 0){
			teki_hp[i] = 0;
		}
	}
	if(tmp_y > (SPR_MAX_Y)){
		teki_hp[i] = 0;
	}
}

/***********************/
/* 敵移動パターンその6 */
/* レーザー            */
/***********************/
void move_teki_pat6(void)//(unsigned char i)
{
//	int dir;

	dir = teki_dir[i];
	if(tkcount[i] == (120 + 16)){
		dir = tekishot_dir();
		teki_shotinfo[i] = 0;
	}else if(tkcount[i] == (40 + 16)){
		dir = DIR_OFF;
		tkshotcount[i] = SHOTCOUNT * 2;
		teki_shotinfo[i] = 3;
	}

	teki_dir[i] = dir;
	++tkcount[i];

	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);


	/* 敵機画面外消去(下方向) */
	if(tmp_y > (SPR_MAX_Y)){
		teki_hp[i] = 0;
	}
	/* 敵機画面外消去(上方向) */
	if(tmp_y < 0){
		if(direction[dir][1] < 0){
			teki_hp[i] = 0;
		}
	}
	/* 敵機画面外消去(左方向) */
	if(tmp_x < 0)
		teki_hp[i] = 0;

	/* 敵機画面外消去(右方向) */
	if(tmp_x > (SPR_MAX_X))
		teki_hp[i] = 0;
}

/*************************/

void boss_tkappend(int i, int pat, int move, int dir, int shot)
{
//	int j;

	if(teki_free[MAX_TEKI] == END_LIST)
		return;

	ADD_LIST(MAX_TEKI, j, teki_next, teki_free);

	teki_pat[j] = pat;
	tkcount[j] = 0;
	tkshotcount[j] = SHOTCOUNT - 2;

	teki_move[j] = move;
	teki_y[j] = teki_y[i];
	teki_x[j] = teki_x[i];

	teki_hp[j] = 4;
	teki_pal[j] = CHRPAL_NO;
	teki_shotinfo[j] = shot;
	teki_dir[j] = dir;
	teki_num++;
}

/*************************/
/* ボス移動パターンその1 */
/*************************/
void move_boss_pat1(void)//(unsigned char i)
{
//	int dir;
	dir = teki_dir[i];

	if(++tkcount[i] == (40 - 4)){
		dir = DIR_RIGHT;
	}else if((tkcount[i] % 600) == (40 - 4)){
		tkshotcount[i] = SHOTCOUNT * 3 / 2;
		dir = DIR_DOWN;
	}else if(tkcount[i] > (40 - 4)){
		switch(dir){
			case DIR_UP:
				if(tmp_y < ((64-8) << SHIFT_NUM_Y)){
					dir = DIR_OFF;
					tkshotcount[i] = SHOTCOUNT * 3 / 2;
					boss_tkappend(i, PAT_TEKI4, 6, DIR_DOWN - 4, 0);
					boss_tkappend(i, PAT_TEKI4, 6, DIR_DOWN + 4, 0);
					boss_tkappend(i, PAT_TEKI4, 6, DIR_DOWN, 0);
				}
				break;

			case DIR_RIGHT:
				if(tmp_x > ((256 - 24) << SHIFT_NUM)){
					dir = DIR_LEFT;
				}
				break;

			case DIR_DOWN:
				if(tmp_y > (192 << SHIFT_NUM_Y)){
					tkshotcount[i] = SHOTCOUNT * 3 / 2;
					if(tkcount[i] < 3500)
						dir = DIR_UP;
				}
				break;

			case DIR_LEFT:
				if(tmp_x < (40 << SHIFT_NUM)){
					dir = DIR_RIGHT;
				}
				break;

			case DIR_OFF:
				if((tkcount[i] % 200) == 50){
					dir = DIR_LEFT;
				}
				break;
		}
	}
	/* 敵機画面外消去(下方向) */
	if(tmp_y > (SPR_MAX_Y))
		teki_hp[i] = 0;

	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);
	teki_dir[i] = dir;

	if(total_count % 2)
		teki_pat[i] = PAT_BOSS1_2;
	else
		teki_pat[i] = PAT_BOSS1;
}
/************/

#define MOVETEKIMAX 10

SUBFUNC moveteki_tbl[MOVETEKIMAX] = 
{
	move_teki_bomb,
	move_teki_pat1,//	move_teki_pat1,
	move_teki_pat2,//	move_teki_pat2,
	move_teki_pat3,//	move_teki_pat3,
	move_teki_pat4,//	move_teki_pat4,
	move_teki_pat5,//	move_teki_pat5,
	move_teki_pat6,//	move_teki_pat6,
	dummy,
	dummy,
	move_boss_pat1,//	move_boss_pat1,
};

/************/
/* 敵機移動 */
/************/
void move_teki(void)
{
//	int i, j, k, l;
//	unsigned char i, j, k, l;

	SEARCH_LIST2(MAX_TEKI, i, k, teki_next){

		/* 敵機移動処理 */
		tmp_x = teki_x[i];
		tmp_y = teki_y[i];

		l = teki_move[i];

		if(l >= 0)
			if(l < MOVETEKIMAX)
				moveteki_tbl[l](); //(i);
			else
				return;
//				move_teki_pat0(); //i);

		teki_x[i] = tmp_x;
		teki_y[i] = tmp_y;

		if(teki_hp[i] > 2){
//			if(hit_check == TRUE){

				/* 敵機と自機弾の接触判定 → 敵機、自機弾消滅 スコア増加 */
				SEARCH_LIST2(MAX_MYSHOT, j, j2, myshot_next){
//					if((tmp_yy = myshot[j].y - tmp_x) < (16 << SHIFT_NUM_Y)){
//						if(tmp_yy > ((TK_OFS_Y - MY_OFS_Y - 8) << SHIFT_NUM_Y)){
					if((tmp_xx = myshot_x[j] - tmp_x) < (16 << SHIFT_NUM)){
//						if(tmp_xx > (-20 << SHIFT_NUM)){
						if(tmp_xx > (-(4 << SHIFT_NUM))){
							if((tmp_yy = myshot_y[j] - tmp_y) < (16 << SHIFT_NUM_Y)){
//								if(tmp_yy > (-20 << SHIFT_NUM_Y)){
								if(tmp_yy > (-(4 << SHIFT_NUM_Y))){
									if(myshot_y[j] != SPR_DEL_Y){

										/* 敵ダメージ */
										teki_pal[i] = REVPAL_NO;
										teki_dmgtime[i] = DMGTIME;

//										if(teki_hp[i] > 2)
											--teki_hp[i];

										if(uramode){
											dir = tekishot_dir();
											pat = PAT_TKSHOT1;
											shottome2(); //tekishot_dir(), PAT_TKSHOT1);
										}

										/* ノーマル弾なら画面外に消去 */
//										myshot_x[j] = SPR_DEL_X;
//										myshot_y[j] = SPR_DEL_Y; //0; //SPR_DEL_Y;	/* 0 */
										/*myshot[j].yy = 0;*/

										trgnum--;
										DEL_LIST(MAX_MYSHOT, j, j2, myshot_next, myshot_free);

										score += 10;		/* 敵の点数を足す */
										scrdspflag = TRUE;
										break;
									}
								}
							}
						}
					}
				}
//			}
		}

		if(teki_hp[i] > 2){
//			if(hit_check == TRUE){
				/* 敵機と自機の接触判定 → 自機ダメージ */
				p_coli = &hantei[0];
				if((tmp_xx = my_data_x[0] - tmp_x) < p_coli->x1){
					if(tmp_xx > p_coli->x2){
						if((tmp_yy = my_data_y[0] - tmp_y) < p_coli->y1){
							if(tmp_yy > p_coli->y2){

								/* 自機ダメージ */
								my_dmg();
							}
						}
					}
				}
//			}

			do_shotcheck(); //i);
		}
		X = teki_x[i];
		Y =teki_y[i] ;
		PAT = teki_pat[i];
		PAL = teki_pal[i];
		DEF_SP_DOUBLE(); //teki_x[i], teki_y[i], teki_pat[i], teki_pal[i]);
//		DEF_SP_SINGLE(teki_x[i], teki_y[i], 2*4, teki_pal[i]);
//		DEF_SP_SINGLE(teki_x[i], teki_y[i], 3*4, teki_pal[i]);

		/* 敵消滅 → 爆発パターンへ */
		if(teki_hp[i] <= 0){
//			teki_y[i] = SPR_DEL_Y;
			DEL_LIST(MAX_TEKI, i, k, teki_next, teki_free);
			--teki_num;
		}
		else if(teki_hp[i] == 2){
			teki_hp[i] = 1;
			teki_move[i] = 0;
			tkcount[i]  = 0;
			teki_dmgtime[i] = 0;
			teki_pal[i] = CHRPAL_NO;

			score += 100;		/* 敵の点数を足す */
			scrdspflag = TRUE;

		}else if(teki_dmgtime[i]){
			teki_dmgtime[i]--;
			if(!teki_dmgtime[i])
				teki_pal[i] = CHRPAL_NO;
		}
//		DEF_SP(teki[i].x, teki[i].y, teki_pat[i], teki_pal[i]);
	}
}

/**********/
/* 敵出現 */
/**********/
long teki_x2;

void com_tkappend(void) //unsigned char teki_no)
{
//	int i;
//	teki_no = i;

	if(teki_free[MAX_TEKI] == END_LIST)
		return;

	ADD_LIST(MAX_TEKI, j, teki_next, teki_free);

	teki_pat[j] = i; //teki_no;
	tkcount[j] = 0;
	tkshotcount[j] = SHOTCOUNT - 2;

	teki_move[j] = command[schedule_ptr + 2];
	teki_x2 = (((command[schedule_ptr + 3] - 7) << SHIFT_NUM));
	teki_x2 *= 256;
	teki_x2 /= 144;
	teki_x[j] = teki_x2;
	teki_y[j] = (command[schedule_ptr + 4] << SHIFT_NUM_Y);

	teki_hp[j] = command[schedule_ptr + 5];
	teki_pal[j] = CHRPAL_NO;

	teki_shotinfo[j] = command[schedule_ptr + 6];

	teki_dir[j] = DIR_DOWN;
	teki_num++;
}


/**************************************/

void com_waitcount(void)//(unsigned char i)
{
	waitcount = i;
}

void com_tkalldel(void)//(unsigned char i)
{
	waitcount = 1;
}

void com_bgmchange(void)//(unsigned char i)
{
/*	playbgm(i, 0);*/
}

void com_bgmfadeout(void)//(unsigned char i)
{
/*	S_IL_FUNC(bgm_fadeout());*/
}

void com_dummy(void)//(unsigned char i)
{
	waitcount = 1;
}

void com_putmessage(void)//(unsigned char i)
{
	put_strings(SCREEN2, command[schedule_ptr + 1], command[schedule_ptr + 2], (char *)message[command[schedule_ptr + 3]], CHRPAL_NO);
}

void com_putstage(void)//(unsigned char i)
{
	unsigned char str_x = command[schedule_ptr + 1];
	unsigned char str_y = command[schedule_ptr + 2];

	if(stage < STAGEMAX)
		++stage;

	put_strings(SCREEN2, str_x, str_y, "STAGE", CHRPAL_NO);
	if(stage > 0){
		if(stage < 10)
			put_numd((long)stage, 1);
		else
			put_numd((long)stage, 2);
		put_strings(SCREEN2, str_x + 6, str_y, str_temp, CHRPAL_NO);
	}
}

void com_end(void)//(unsigned char i)
{
	waitcount = 1;
}

void com_se(void)//(unsigned char i)
{
	seflag = command[schedule_ptr + 1];
}

/**************************************/
/* スケジュール解析で行うコマンド
・敵追加
・処理待ち
・BGM変更(F-Out)
・メッセージ表示
・各種エフェクト
*/
/**************************************/

/* スケジュール用ジャンプテーブル enumの値と対応する */
#define COMMAND_MAX 10

SUBFUNC command_tbl[COMMAND_MAX] = {
	com_dummy,
	com_waitcount,
	com_tkalldel,
	com_tkappend,
	com_bgmchange,
	com_bgmfadeout,
	com_putmessage,
	com_putstage,
	com_end,
	com_se,
};

/********************/
/* スケジュール実行 */
/********************/
void do_schedule(void)
{
/*	SUBFUNC p;*/

	while(1){
//	printf("Test %d %d\n", command_num, waitcount);

	/* イベント処理待ちなら戻る */
	if(waitcount){
		switch(command_num){
			case COM_WAITCOUNT:
				--waitcount;
				return;
				break;

			case COM_TKALLDEL:
				if(teki_next[MAX_TEKI] == END_LIST)
					waitcount = 0;
				return;
				break;

			case COM_DUMMY:
				return;
				break;

			case COM_END:
				--waitcount;
				schedule_ptr = 0;
				if(tkshot_c > (1 << SHIFT_NUM))
					tkshot_c /= 2;
				else{
					if(uramode != URAMAX){
						uramode++;
						if(uramode != URAMAX)
							tkshot_c = 6 << SHIFT_NUM;
						else
							tkshot_c = 0;
					}
				}
				if(!noshotdmg_flag){	/* PACIFIST */
					score += 5000; //10000;
					scrdspflag = TRUE;
					seflag = 4;
				}
				noshotdmg_flag = FALSE;

				if(my_hp < max_my_hp)
					my_hp = max_my_hp;
				else
					my_hp += 1;

				if(my_hp > HP_MAX)
					my_hp = HP_MAX;
				put_my_hp();
				break;
		}
	}

	command_num = command[schedule_ptr];	/* 次に実行するコマンド */

	if(command_num >= 0)
		if(command_num < COMMAND_MAX)
/*			if((p = command_tbl[command_num]) != NULL)*/
				i = command[schedule_ptr + 1];
				command_tbl[command_num]();//command[schedule_ptr + 1]);

	schedule_ptr += command_length[command_num];	/* 引数の数だけ先に進める */

	}
}


//#endif
