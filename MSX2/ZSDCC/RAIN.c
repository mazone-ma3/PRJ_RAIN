/* Project RAIN �Q�|���{�� for MSX2 */

#define MAIN
#define MSX2

#define DEBUG

enum {
	BGMMAX = 2,
	SEMAX = 4
};

void wait_vsync(void);
void put_strings(unsigned char scr, unsigned char x, unsigned char y,  char *str, unsigned char pal);
void put_numd(long j, unsigned char digit) __sdcccall(1);
unsigned char spr_page = 1;
void write_vram_adr(unsigned char highadr, int lowadr) __sdcccall(1);

void set_int(void);
void reset_int(void);

#define DI() {\
__asm\
	di\
__endasm;\
}

#define EI() {\
__asm\
	ei\
__endasm;\
}

/******************************************************************************/
#include "sp_com.h"
#include "spr_col.h"
/******************************************************************************/

unsigned char wake_count = 0;
unsigned char muki = PAT_JIKI_C;

#define HMMM 0xD0
#define LMMM 0x90

enum {
	VDP_READDATA = 0,
	VDP_READSTATUS = 1
};

enum {
	VDP_WRITEDATA = 0,
	VDP_WRITECONTROL = 1,
	VDP_WRITEPAL = 2,
	VDP_WRITEINDEX = 3
};

#define VDP_readport(no) (VDP_readadr + no)
#define VDP_writeport(no) (VDP_writeadr + no)

unsigned char VDP_readadr;
unsigned char VDP_writeadr;

#define MAXCOLOR 16

/* R G B */
unsigned char org_pal[MAXCOLOR][3] = {
	{  0,  0,  0},
	{  0,  0,  0},
	{  3, 13,  3},
	{  7, 15,  7},
	{  3,  3, 15},
	{  5,  7, 15},
	{ 11,  3,  3},
	{  5, 13, 15},
	{ 15,  3,  3},
	{ 15,  7,  7},
	{ 13, 13,  3},
	{ 13, 13,  7},
	{  3,  9,  3},
	{ 13,  5, 11},
	{ 11, 11, 11},
	{ 15, 15, 15},
};

FILE *stream[2];


/* MSX BLOAD�f�[�^���t�@�C�����烁�����ɓǂݍ��� */
short msxload(char *loadfil, unsigned short offset)
{
	unsigned short size;
	unsigned char *address;
	unsigned char buffer[2];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
//		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}

	fread( buffer, 1, 1, stream[0]);
	fread( buffer, 1, 2, stream[0]);
	address = (unsigned short *)(buffer[0] + buffer[1] * 256);
	fread( buffer, 1, 2, stream[0]);
	size = (buffer[0] + buffer[1] * 256) - (unsigned short)address;
	fread( buffer, 1, 2, stream[0]);
	address -= offset;
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address, size, (unsigned short)address + size);

	fread( address , 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

char checkbgm(void) __sdcccall(1)
{
__asm
	ld	hl,_checkbgm
	ld	a,0x80
	cp	h
	jr	c,bgmerr1

	ld	hl,#0xbc00
	ld	a,(hl)
	cp	#0xc3
	jr	nz,bgmon
	inc	hl
	ld	a,(hl)
	cp	#0x0d
	jr	nz,bgmon
	inc	hl
	ld	a,(hl)
	cp	#0xbc
	jr	nz,bgmon
	ld	a,1
	ret

bgmerr1:
	ld	hl,#0xdc00
	ld	a,(hl)
	cp	#0x2a
	jr	nz,bgmerr2
	inc	hl
	ld	a,(hl)
	cp	#0xf8
	jr	nz,bgmerr2
	inc	hl
	ld	a,(hl)
	cp	#0xf7
	jr	nz,bgmerr2
	ld	a,2
	ret
bgmerr2:
	xor	a
	ret

bgmon:
__endasm;
//	if(msxload("psgtone.dat", 0x2000*1) == ERROR)
//		return 0;

	if(msxload("NOTITLE.PLY", 0x2000*1) == ERROR)
		return 0;

	if(msxload("PSGMSXD.MSX", 0) == ERROR)
		return 0;

	return 1;

__asm
__endasm;
	return 0;
}

void play_bgm(unsigned char mode) __sdcccall(1)
{
__asm
	ld	(#0xf7f8),a
	call	#0xdc00
__endasm;
}

void playbgm(void) __sdcccall(1)
{
	unsigned char a;
	a = checkbgm();
	if(!a)
		return;
	if(a==1){
__asm
	call #0xbc00
__endasm;
	}else if(a==2){
		play_bgm(0);
	}
}

void stopbgm(void) __sdcccall(1)
{
	unsigned char a;
	a = checkbgm();
	if(!a)
		return;
	if(a==1){
__asm
	call #0xbc03
__endasm;
	}else if(a==2){
		play_bgm(-1);
	}
}

void set_vol(unsigned char vol) __sdcccall(1)
{
}


void write_psg(unsigned char reg, unsigned char tone) __sdcccall(1)
{
__asm
;	ld	hl, #2
;	add	hl, sp
	push	ix

	ld	h,a

	ld	a,(#0xfcc1)	; exptbl
	ld	b,a
	ld	c,0
	push	bc
	pop	iy

;	ld	c,(hl)
;	inc	hl
;	ld	b,(hl)	;bc = reg
;	inc	hl
;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; de = tone

;	ld	a,e
;	ld	e,d
	ld	a,h
	ld	e,l

	ld ix,#0x0093	; WRTPSG(MAINROM)
	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

/* screen��BIOS�؂�ւ� */
void set_screenmode(unsigned char mode) __sdcccall(1)
{
__asm
;	ld	 hl, 2
;	add	hl, sp

	push	ix
	ld	b,a

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x005f	; CHGMOD(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,b

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void set_screencolor(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x0062	; CHGCLR(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void key_flush(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x0156	; KILBUF(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void cls(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x00c3	; CLS(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

/* mainrom�̎w��Ԓn�̒l�𓾂� */
unsigned char  read_mainrom(unsigned short adr) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
;	push	ix

;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; de=adr
;	ld	h,d
;	ld	l,e	; hl=adr

	ld	a,(#0xfcc1)	; exptbl
	call	#0x000c	; RDSLT

;	ld	l,a
;	ld	h,#0

;	pop	ix
__endasm;
}

void write_VDP(unsigned char regno, unsigned char data) __sdcccall(1)
{
//	outp(VDP_writeport(VDP_WRITECONTROL), data);
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x80 | regno);
__asm
	ld	h,a
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	ld	a,l
	out	(c),a
	ld	a,h
	set 7,a
	out	(c),a
__endasm;
}

void write_vram_adr(unsigned char highadr, int lowadr) __sdcccall(1)
{
__asm
	push	de
__endasm;
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm
	pop	de
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	out	(c),e
	ld	a,d
	and	a,0x3f
	set	6,a
	out	(c),a
__endasm;
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data) __sdcccall(1)
{
__asm
//	outp(VDP_writeport(VDP_WRITEDATA), data);
	ld	b,a
	ld	a,(_VDP_writeadr)
	ld	c,a
	out	(c),b
__endasm;
}

void read_vram_adr(unsigned char highadr, int lowadr) __sdcccall(1)
{
__asm
	push	de
__endasm;
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm
	pop	de
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	out	(c),e
	ld	a,d
	and	a,0x3f
	out	(c),a
__endasm;
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x00 | ((lowadr >> 8) & 0x3f));
}

unsigned char read_vram_data(void) __sdcccall(1)
{
__asm
	ld	a,(_VDP_readadr)
	ld	c,a
	in	a,(c)
__endasm;
//	return inp(VDP_readport(VDP_READDATA));
}


//#define read_vram_data() inp(VDP_readport(VDP_READDATA))


void set_displaypage(int page) __sdcccall(1)
{
	DI();
	write_VDP(2, (page << 5) & 0x60 | 0x1f);
	EI();
}

unsigned char read_VDPstatus(unsigned char no) __sdcccall(1)
{
	unsigned char data;
	DI();
	write_VDP(15, no);
//	data = inp(VDP_readport(VDP_READSTATUS));
__asm
	ld	a,(_VDP_readadr)
	inc	a
	ld	c,a
	in a,(c)
	push	af
__endasm;
	write_VDP(15, 0);
__asm
	pop	af
	ei
__endasm;
//	return data;
}

unsigned char port,port2;

void wait_VDP(void) {
//	unsigned char data;
	port = VDP_writeport(VDP_WRITECONTROL);
	port2 = VDP_readport(VDP_READSTATUS);

/*	do{
__asm
	EI
__endasm;
__asm
	DI
__endasm;
		outp(port, 2);
		outp(port, 0x80 + 15);

		data = inp(port2);

		outp(port, 0);
		outp(port, 0x80 + 15);
	}while((data & 0x01));
*/
__asm
waitloop:
	ei
	nop
	di
	ld	a,(_port)
	ld	c,a
	ld	a,2
	out	(c),a
	ld	a,#0x80 + 15
	out	(c),a
	ld	b,c

	ld	a,(_port2)
	ld	c,a
	in a,(c)
	ld	c,b
	ld	b,a

	xor	a,a
	out	(c),a
	ld	a,#0x80 + 15
	out	(c),a

	ld	a,b
	and	a,#0x01
	jr	nz,waitloop
__endasm;
}

void boxfill(int dx, int dy, int nx, int ny, unsigned char dix, unsigned char diy, unsigned char data)
{
	unsigned char port = VDP_writeport(VDP_WRITEINDEX);
	unsigned char port2 = VDP_writeport(VDP_WRITECONTROL);

	wait_VDP();

//	write_vdp(17, 36);
	outp(port2, 36);
	outp(port2, 0x80 | 17);

	outp(port, dx & 0xff);
	outp(port, (dx >> 8) & 0x01);
	outp(port, dy & 0xff);
	outp(port, (dy >> 8) & 0x03);
	outp(port, nx & 0xff);
	outp(port, (nx >> 8) & 0x01);
	outp(port, ny & 0xff);
	outp(port, (ny >> 8) & 0x03);
	outp(port, data);
	outp(port, ((diy << 3) & 0x80) | ((diy << 2) & 0x40));
	outp(port, 0xc0);

	wait_VDP();

	EI();
}

unsigned char port3, port4;

unsigned char sx, sy, dx, dy; //, nc, ny, dix, diy, 
unsigned char VDPcommand;
unsigned char APAGE,VPAGE,XSIZE,XSIZA,YSIZE;

void VDPsetAREA2(void)
/*unsigned short sx, unsigned short sy, unsigned short dx, unsigned short dy, unsigned short nx, unsigned short ny, unsigned char dix, unsigned char diy, unsigned char command)*/
{
	port3 = VDP_writeport(VDP_WRITEINDEX);
	port4 = VDP_writeport(VDP_WRITECONTROL);

//	vdpdata[0] = (sx & 0xff);		/* 32 */
//	vdpdata[1] = ((sx >> 8) & 0x01);	/* 33 */
//	vdpdata[2] = (sy & 0xff);		/* 34 */
//	vdpdata[3] = ((sy >> 8) & 0x03);	/* 35 */
//	vdpdata[4] = (dx & 0xff);		/* 36 */
//	vdpdata[5] = ((dx >> 8) & 0x01);	/* 37 */
//	vdpdata[6] = (dy & 0xff);		/* 38 */
//	vdpdata[7] = ((dy >> 8) & 0x03);	/* 39 */
//	vdpdata[8] = (nx & 0xff);		/* 40 */
//	vdpdata[9] = ((nx >> 8) & 0x01);	/* 41 */
//	vdpdata[0xa] = (ny & 0xff);		/* 42 */
//	vdpdata[0xb] = ((ny >> 8) & 0x03);	/* 43 */
//	vdpdata[0xc] = 0;
//	vdpdata[0xd] = ((diy << 3) & 0x08) | ((dix << 2) & 0x04);	/* 45 */
//	vdpdata[0xe] = VDPcommand;
__asm
	ld	a,(_sx)	;SX
	ld	h,a
	ld	a,(_sy)	;SY
	ld	l,a
;	ld	a,(_vdpdata+3)
;	ld	(_APAGE),a
	ld	a,(_dx)	;DX
	ld	d,a
	ld	a,(_dy)	;DY
	ld	e,a
;	ld	a,(_vdpdata+7)
;	ld	(_VPAGE),a
;	ld	a,(_vdpdata+8)
;	ld	(_XSIZE),a
;	ld	a,(_vdpdata+9)
;	ld	(_XSIZA),a
;	ld	a,(_vdpdata+0xa)
;	ld	(_YSIZE),a
;	exx

__endasm;
	wait_VDP();
/*
	outp(port4, 32);
	outp(port4, 0x80 | 17);
*/
//__asm
//__endasm;

//	outp(port3, data0);			/* 32 */
//	outp(port3, data1);			/* 33 */
//	outp(port3, data2);			/* 34 */
//	outp(port3, data3);			/* 35 */
//	outp(port3, data4);			/* 36 */
//	outp(port3, data5);			/* 37 */
//	outp(port3, data6);			/* 38 */
//	outp(port3, data7);			/* 39 */
//	outp(port3, data8);			/* 40 */
//	outp(port3, data9);			/* 41 */
//	outp(port3, dataa);			/* 42 */
//	outp(port3, datab);			/* 43 */
//	outp(port3, 0);				/* 44 */

//	outp(port3, datad);	/* 45 */

//	outp(port3, VDPcommand);
__asm
;	exx
	ld	a,(_port4)
	ld	c,a
	ld	a,32
	out	(c),a
	ld	a,#0x80 | 17
	out	(c),a

	ld	b,0x0f
	ld	a,(_port3)
	ld	c,a
;	ld	hl,_vdpdata

	XOR	A
	OUT	(C),H	;SX
	OUT	(C),A	
	LD	A,(_APAGE)
	OUT	(C),L	;SY
	OUT	(C),A	

	XOR	A
	OUT	(C),D	;DX
	OUT	(C),A	
	LD	A,(_VPAGE)
	OUT	(C),E	;DY
	OUT	(C),A
	LD	A,(_XSIZE)
	LD	B,A
	LD	A,(_XSIZA)
	OUT	(C),B
	OUT	(C),A
	LD	A,(_YSIZE)
	LD	B,A
	XOR	A
	OUT	(C),B
	OUT	(C),A
	OUT	(C),A	;DUMMY

	LD	A,H
	SUB	D
	LD	A,0
	JR	C,DQ
DQ:	OR	2

	OUT	(C),A	;DIX and DIY

	ld	a,(_VDPcommand)
	out	(C),a	/* com */
	ei
__endasm;
}

void spr_on(void)
{
	DI();
	write_VDP(8, 0x08);
	EI();
}

void spr_off(void)
{
	DI();
	write_VDP(8, 0x0a);
	EI();
}

void set_spr_atr_adr(unsigned char highadr) __sdcccall(1) //, int lowadr)
{
//	DI();
//	write_VDP(5, (lowadr >> (2 + 5)) & 0xf8 | 0x07);
//	write_VDP(11, ((highadr << 1) & 0x02) | ((lowadr >> 15) & 0x01));
//	write_VDP(5, (0xe8));
	write_VDP(11, ((highadr << 1) & 0x02));
//	EI();
}

unsigned char get_key(unsigned char matrix) __sdcccall(1)
{
	outp(0xaa, ((inp(0xaa) & 0xf0) | matrix));
	return inp(0xa9);
/*
__asm
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x0141	; SNSMAT(MAINROM)

	ld	 hl, #2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	#0x001c	; CALSLT

	ld	l,a
	ld	h,#0
__endasm;
*/
}

unsigned char get_stick(unsigned char trigno) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d5	; GTSTCK(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
__endasm;
}

unsigned char get_pad(unsigned char trigno) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d8	; GTTRIG(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
__endasm;
}

/* DISK BASIC only. */
/*volatile void Set_RAM_MODE(void){
__asm
	push	ix
	ld	a,(#0xf342)
	ld	hl,#0x4000
	call	#0x0024
	pop	ix
__endasm;
}

volatile void Set_ROM_MODE(void){
__asm
	push	ix
	ld	a,(#0xfcc1)
	ld	hl,#0x4000
	call	#0x0024
	pop	ix
__endasm;
}*/

unsigned char *jiffy = (unsigned char *)0xfc9e;
unsigned char jiffy_flag = 0;
unsigned char old_jiffy;
unsigned char old_jiffy2 = 0;

void wait_vsync(void)
{
/*
__asm
	ld	a,(#0xfc9e)
	ld	b,a
jiffyloop:
	ld	a,(#0xfc9e)
	cp	b
	jr	z,jiffyloop
__endasm;
*/
//	return;

	while((read_VDPstatus(2) & 0x40));
	while(!(read_VDPstatus(2) & 0x40)); /* WAIT VSYNC */

/*	++total_count;
	if(*jiffy >= 60){
		put_numd((long)(total_count), 2);
		put_strings(SCREEN2, 28, 22, str_temp, CHRPAL_NO);
		total_count = 0;
		*jiffy = 0;
	}*/


	++total_count;
#ifdef DEBUG3
	if((unsigned char)(*jiffy - old_jiffy2) >= 60){
//	if(*jiffy >= 60){
		old_jiffy2 = *jiffy;
		put_numd((long)(total_count), 2);
		put_strings(SCREEN2, 27, 22, str_temp, CHRPAL_NO);
		total_count = 0;
//		*jiffy = 0;
	}
#endif
}

void init_sys_wait(void)
{
__asm
	ld	a,(#0xfc9e)
	ld	(_old_jiffy),a
__endasm;
}

void sys_wait(unsigned char wait) __sdcccall(1)
{
__asm
	ld	l,a
	ld	a,(_old_jiffy)
;	add	a,l
	ld	b,a

jiffyloop2:
	ld	a,(#0xfc9e)
	sub	a,b
;	cp	b
	cp	l
	jr	c,jiffyloop2	; a<b
__endasm;

	++total_count;
#ifdef DEBUG
	if((unsigned char)(*jiffy - old_jiffy2) >= 60){
//	if(*jiffy >= 60){
		old_jiffy2 = *jiffy;
		put_numd((long)(total_count), 2);
		put_strings(SCREEN2, 27, 22, str_temp, CHRPAL_NO);
		total_count = 0;
//		*jiffy = 0;
	}
#endif
}

/*void sys_wait(unsigned char wait)
{
__asm
	ld	l,a
	ld	a,(#0xfc9e)
	add	a,l
	ld	b,a
jiffyloop2:
	ld	a,(#0xfc9e)
	cp	b
	jr	c,jiffyloop2	; a<b
__endasm;

	return;

	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}
*/
/*�p���b�g�E�Z�b�g*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	unsigned char port = VDP_writeport(VDP_WRITEPAL);
	write_VDP(16, color);
	outp(port, red * 16 | blue);
	outp(port, green);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0]/2, color[i][1]/2, color[i][2]/2);
}

//value < 0 ���ɋ߂Â���B
//value = 0 �ݒ肵���F
//value > 0 ���ɋ߂Â���B
void set_constrast(int value, unsigned char org_pal[MAXCOLOR][3], int pal_no)
{
	int j, k;
	int pal[3];


	for(j = 0; j < MAXCOLOR; j++){
		for(k = 0; k < 3; k++){
			if(value > 0)
				pal[k] = org_pal[j][k] + value;
			else if(value < 0)
				pal[k] = org_pal[j][k] * (15 + value) / 15;
			else
				pal[k] = org_pal[j][k];
			if(pal[k] < 0)
				pal[k] = 0;
			else if(pal[k] > 15)
				pal[k] = 15;
		}
		pal_set(pal_no, j, pal[0]/2, pal[1]/2, pal[2]/2);
	}
}

//wait�l�̑��x�ō�����t�F�[�h�C������B
/*void fadeinblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	init_sys_wait();
	for(j = -15; j <= 0; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
		init_sys_wait();
	}
}*/

//wait�l�̑��x�ō��Ƀt�F�[�h�A�E�g����B
/*void fadeoutblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	init_sys_wait();
	for(j = 0; j != -16; j--){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
		init_sys_wait();
	}
}*/

//wait�l�̑��x�Ŕ��Ƀt�F�[�h�A�E�g����B
void fadeoutwhite(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	init_sys_wait();
	for(j = 0; j < 16; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
		init_sys_wait();
	}
}

//�p���b�g���Ó]����B
void pal_allblack(int pal_no)
{
	unsigned char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

unsigned char spr_flag = 0, spr_next = 0;
unsigned char vdps0;


short test_h_f = TRUE;
short soundflag = FALSE;

char chr;

void put_strings(unsigned char scr, unsigned char x, unsigned char y,  char *str, unsigned char pal)
{
//	y = 28-y;

	XSIZE = 8;
	XSIZA = 0;
	YSIZE = 8;
	APAGE = 3; //map_page;
	VPAGE = 0;
	VDPcommand = HMMM;

	while(1){
		chr = *(str++);
		if(chr == '\0')
			break;
		if((chr < 0x30))
			chr = 0x40;
		chr -= '0';
		sx = (chr & 0x0f) * 8;
		sy = (chr / 16) * 8;
		dx = x * 8;
		dy = y * 8;
		VDPsetAREA2();
		++x;
	}
}


void put_numd(long j, unsigned char digit) __sdcccall(1)
{
	unsigned char i = digit;

	while(i--){
		str_temp[i] = j % 10 + 0x30;
		j /= 10;
	}
	str_temp[digit] = '\0';
}

void score_display(void)
{
	put_numd(score, 8);
	put_strings(SCREEN2, 15, 22 , str_temp, CHRPAL_NO);
	if(score >= hiscore){
		if((score % 10) == 0){
			hiscore = score;
			put_strings(SCREEN2, 8, 22, "HIGH ", CHRPAL_NO);
		}
	}else
		put_strings(SCREEN2, 8, 22, "SCORE", CHRPAL_NO);
}

void score_displayall(void)
{
//	put_strings(SCREEN2, 9, 22, "SCORE", CHRPAL_NO);
	score_display();
}

void hiscore_display(void)
{
	if(score > hiscore)
		if((score % 10) == 0)
			hiscore = score;

	put_numd(hiscore, 8);

	put_strings(SCREEN2, 9, 12, "HIGH", CHRPAL_NO);
	put_strings(SCREEN2, 9 + 5, 12, str_temp, CHRPAL_NO);
}

#include "inkey.h"

unsigned char st0, st1, pd0, pd1, pd2, k3, k5, k7, k9, k10;
unsigned char keycode = 0;

void keyscan1(void)
{
	keycode = 0;

	return;
}

void keyscan(void)
{
	DI();
	keycode = 0;

	k3 = get_key(3);

	k9 = get_key(9);
	k10 = get_key(10);
	k5 = get_key(5);

	st0 = get_stick(0);
	st1 = get_stick(1);

	pd0 = get_pad(0);
	pd1 = get_pad(1);
	pd2 = get_pad(3);
	EI();

	if((pd0) || (pd1) || !(k5 & 0x20)) /* X,SPACE */
		keycode |= KEY_A;
	if((pd2) || !(k3 & 0x01)) /* C */
		keycode |= KEY_B;
	if((st0 >= 1 && st0 <=2) || (st0 == 8) || (st1 >= 1 && st1 <=2) || (st1 ==8) || !(k10 & 0x08)) /* 8 */
		keycode |= KEY_UP1;
	if((st0 >= 4 && st0 <=6) || (st1 >= 4 && st1 <=6) || !(k9 & 0x20)) /* 2 */
		keycode |= KEY_DOWN1;

//	if(!(st & 0x0c)){ /* RL */
//		keycode |= KEY_START;
//	}else{
	if((st0 >= 6 && st0 <=8) || (st1 >= 6 && st1 <=8) || !(k9 & 0x80)) /* 4 */
		keycode |= KEY_LEFT1;
	if((st0 >= 2 && st0 <=4) || (st1 >= 2 && st1 <=4) || !(k10 & 0x02)) /* 6 */
		keycode |= KEY_RIGHT1;
//	}

	return; // keycode;
}


unsigned char x = 0;
int loopcounter = 0;
unsigned int soundtestno = 0;
int soundtest = FALSE;


/* �Q�|���̃��|�v */
unsigned char a = 0, b = 0;
//unsigned char i,j;
//int i, j,
int  xx, yy,*p_x,*p_y;
unsigned char pat_no;
short game_loop(void){
//	unsigned char i,j;
//	short xx,yy;
//	tmp_spr_count = 0;

/* �p�b�h���� & ���@�ړ� */
/*	for(i=0;i<1;i++)*/
	i = 0;
	a = 0;
	b = 0;
	{
		if(keycode & KEY_A) /* Z,SPACE */
			a=1;
		if(keycode & KEY_B) /* X */
			b=1;


/* 00  0=hit 1=Nohit */
/* AB */

		if (b){
			if (scrl_spd < SCRL_MAX)
				scrl_spd++;
		}else if (scrl_spd > SCRL_MIN)
			scrl_spd--;

		if (a & b)
			scrl_spd = 0;

/* ���@�ړ�(�΂ߕ����Ή�) */
		xx = yy = 0;
		pat_no = PAT_JIKI_C;

		if(keycode & KEY_LEFT1){ /* 4 */
			if(wake_count & 8)
				pat_no = PAT_JIKI_L;
			else
				pat_no = PAT_JIKI_R;
			xx = -1;
			muki = PAT_JIKI_L;
		}
		if(keycode & KEY_RIGHT1){ /* 6 */
			if(wake_count & 8)
				pat_no = PAT_JIKI_R;
			else
				pat_no = PAT_JIKI_L;
			xx = 1;
			muki = PAT_JIKI_R;
		}
		if(keycode & KEY_UP1){ /* 8 */
			pat_no = PAT_JIKI_J;
			yy = -1;
		}
		if(keycode & KEY_DOWN1){ /* 2 */
			pat_no = PAT_JIKI_J;
			yy = 1;
		}
//		if(!xx)
//			muki = pat_no;
		my_data_pat_num[i] = pat_no;
		++wake_count;

		/* �΂߂̎��̏���(�蔲����) */
		if ((xx == 0) || (yy == 0)){
			xx *= 3; yy *= 3;
		}
		else{
			xx *= 2; yy *= 2;
		}
		xx <<= SHIFT_NUM;
		yy <<= SHIFT_NUM_Y;

		p_x=&my_data_x[i];
		p_y=&my_data_y[i];

		*p_x+=xx;
		*p_y+=yy;

	/* ���@���ړ��ł���͈͂�ݒ� */
		if(*p_y <= JIKI_MIN_Y)
			*p_y = JIKI_MIN_Y;
			else if(*p_y >= JIKI_MAX_Y)
				*p_y = JIKI_MAX_Y;
			if(*p_x <= JIKI_MIN_X)
				*p_x = JIKI_MIN_X;
			else if(*p_x >= JIKI_MAX_X)
				*p_x = JIKI_MAX_X;

/*		DI();
		write_vram_adr(spr_page, 0x7600);
		EI();
*/

		X = *p_x;
		Y = *p_y;
		PAT = my_data_pat_num[i];
		PAL = mypal;
		DEF_SP_DOUBLE(); //*p_x, *p_y, my_data_pat_num[i], mypal);
//		EI();
//		DEF_SP_SINGLE(*p_x + (2 << SHIFT_NUM), *p_y, 2*4, mypal);
//		DEF_SP_SINGLE(*p_x + (2 << SHIFT_NUM), *p_y, 3*4, mypal);

		/* ���@�e���� */
		if(trgcount)
			trgcount--;
		if(renshaflag == FALSE)
			if(trgcount2)
				trgcount2--;

		if (a  //&& ((MAX_MYSHOT - trgnum) >= 1) 
			&& (muki != PAT_JIKI_C)){
			noshotdmg_flag = TRUE;
			if(trgcount2){
				if(!trgcount){
					trgcount = 5;

					if(myshot_free[MAX_MYSHOT] != END_LIST){
						ADD_LIST(MAX_MYSHOT, tmp, myshot_next, myshot_free);
						myshot_x[tmp] = my_data_x[i] + (1 << SHIFT_NUM);
						myshot_y[tmp] = my_data_y[i];

						myshot_yy[tmp] = 0;
						if(muki == PAT_JIKI_R)
							myshot_xx[tmp] = (6 << SHIFT_NUM);
						else if(muki == PAT_JIKI_L)
							myshot_xx[tmp] = -(6 << SHIFT_NUM);

						if(muki == PAT_JIKI_R)
							myshot_pat_num[tmp] = PAT_MYSHOT2;
						else
							myshot_pat_num[tmp] = PAT_MYSHOT1;

						trgnum++;
					}
/*					if(myshot_free[MAX_MYSHOT] != END_LIST){
						ADD_LIST(MAX_MYSHOT, tmp, myshot_next, myshot_free);
						myshot_x[tmp] = my_data_x[i] + (13 << SHIFT_NUM);
						myshot_y[tmp] = my_data_y[i];

						myshot_xx[tmp] = 0;
						myshot_yy[tmp] = -(6 << SHIFT_NUM);

						myshot_pat_num[tmp] = PAT_MYSHOT1;

						trgnum++;
					}
*/				}
			}
		}else
			trgcount2 = 60 / 3;
	}

	/** �X�P�W���[����͂����s **/
	do_schedule();

	/* ���@�e�ړ� */
	SEARCH_LIST2(MAX_MYSHOT, i, j, myshot_next){
		tmp_x = myshot_x[i];
		tmp_y = myshot_y[i];

		if(tmp_y < SPR_DEL_Y){
			tmp_x += myshot_xx[i];
			tmp_y += myshot_yy[i];
		}
//		/* ���@�e��ʊO���� */
		if((tmp_x < (((SCREEN_MIN_X + SPR_OFS_X) << SHIFT_NUM) + 16))
		|| (tmp_x > (((SCREEN_MAX_X - SPR_OFS_X) << SHIFT_NUM) + 16))){
			tmp_y = SPR_DEL_Y;
		}
		if(tmp_y == SPR_DEL_Y){
			trgnum--;

//			DEF_SP(tmp_x, tmp_y, myshot[i].pat_num,  CHRPAL_NO);
//			DEF_SP_SINGLE(tmp_x, tmp_y + (4 << SHIFT_NUM), 6*4, CHRPAL_NO);
			DEL_LIST(MAX_MYSHOT, i, j, myshot_next, myshot_free);

		}else{
			myshot_x[i] = tmp_x;
			myshot_y[i] = tmp_y;
//			DI();
//			write_vram_adr(spr_page, 0x7600 + tmp_spr_count * 4);


			X = tmp_x;
			Y = tmp_y + (4 << SHIFT_NUM_Y);
			PAT =  myshot_pat_num[i] * 8;
			PAL = CHRPAL_NO;
			DEF_SP_SINGLE(); //tmp_x, tmp_y + (4 << SHIFT_NUM_Y), myshot_pat_num[i] * 8, CHRPAL_NO);
//			EI();
//			DEF_SP_SINGLE(tmp_x, tmp_y + (4 << SHIFT_NUM), 0*4, CHRPAL_NO);
		}

	}

//	DI();
//	write_vram_adr(spr_page, 0x7600 + tmp_spr_count * 4);
//	EI();
	move_teki();
	move_tekishot();
//	EI();

#ifdef DEBUG
	/* �X�v���C�g���\�� */
	if(old_count[spr_page] != tmp_spr_count){
		put_numd((long)(tmp_spr_count), 2);
		put_strings(SCREEN2, 27, 24, str_temp, CHRPAL_NO);
	}
#endif
//	if(jiffy_flag){
//		jiffy_flag = 0;
/*		put_numd((long)(total_count), 2);
		put_strings(SCREEN2, 14, 14, str_temp, CHRPAL_NO);
		total_count = 0;*/
//	}
//	spr_count[spr_page] = tmp_spr_count;

	return NOERROR;
}

unsigned char color_flag[MAX_SPRITE];

/* �ϐ����������� */
void game_init(void){
	int i;

//	srand(time(NULL));	/* �����̏����� */
	for(i = 0; i < MAX_SPRITE * 2; ++i){
//		chr_data[i].x = 0;
//		chr_data[i].y = 212;
		chr_data[i].pat_num = 255;
		chr_data[i].atr = 0x80;
	}
	for(i = 0; i < MAX_SPRITE; ++i){
		color_flag[i] = 0;
		for(j = 0; j < 2; ++j){
			old_data[j][i].pat_num = 255;
			old_data[j][i].atr = 0;
//			old_data[j][i].pal = 0;
		}
	}

	stage = 0;
	waitcount = 0;
	schedule_ptr = 0;
	command_num = COM_DUMMY;
	command = (int *)stg1_data;
	uramode = 0;
//	renshaflag = FALSE;
	renshaflag = TRUE;

	trgcount = 0;	/* �V���b�g�Ԋu���~�b�^ */
	trgcount2 = 0;	/* �A�˃��~�b�^ */
	trgnum = 0;
	total_count = 0;

//	scrl = 0;
	scrl_spd = SCRL_MIN;

	mypal = CHRPAL_NO;
	mypal_dmgtime = 0;
	my_movecount = 0;

	/* �G�\����񏉊��� */
	INIT_LIST(MAX_MYSHOT, i, myshot_next, myshot_free);
	INIT_LIST(MAX_TKSHOT, i, tkshot_next, tkshot_free);
	INIT_LIST(MAX_TEKI, i, teki_next, teki_free);


	score = 0;
	tkshot_c = (6 << SHIFT_NUM);
	max_my_hp = 4; //7;

	my_hp = max_my_hp;

	for(i = 0; i < MAX_TKSHOT; i++){
		tkshot_xx[i] = 0;
		tkshot_yy[i] = 0;
	}
	tkshotnum = 0;
	scrdspflag = TRUE;
	noshotdmg_flag = FALSE;

	seflag = 0;

/* ���X�g������(�܂����g�p) */
/*	for(i=0;i<10;i++){
		start[i].next = &fin[i];
		start[i].prev = NULL;
		fin[i].next = NULL;
		fin[i].prev = &start[i];
	}
*/

	/* SPRITE ������ */
/*	for(i = 0; i < 256; i++)
		spr[i].y = SPR_DEL_Y;*/

/* ���@���W�̏����� */
	my_data_x[0] = (2 * 8 - SPR_OFS_X) << SHIFT_NUM;
	my_data_y[0] = (16 * 8 - SPR_OFS_Y) << SHIFT_NUM_Y;
	my_data_pat_num[0] = PAT_JIKI_C;
	my_data_x[1] = (2 * 8 - SPR_OFS_X) << SHIFT_NUM;		/* ��l�v���C��z�� */
	my_data_y[1] = (16 * 8 - SPR_OFS_Y) << SHIFT_NUM_Y;
	my_data_pat_num[1] = PAT_JIKI_C;

/* ���e���W�̏����� */
/*	for(i=0; i<10; i++){
		shot_data[i].x = 0 << SHIFT_NUM;
		shot_data[i].xx = 0;
		shot_data[i].y = SPR_DEL_Y;
		shot_data[i].yy = 0;
		shot_data[i].pat_num = PAT_MYSHOT1;
	}*/


/* �G���W�̏����� */
/* �S�X�v���C�g�\�����ő�180�Ƃ����4�������˂ł�������30���x */
/*	for(i=0; i<TEKI_NUM_MAX; i++){
		teki_data[i].x = (rand() % SCREEN_MAX_Y) << SHIFT_NUM;
		teki_data[i].xx = 0;
		teki_data[i].y = (rand() % SCREEN_MAX_X) << SHIFT_NUM;
		teki_data[i].yy = ((rand() & 7) + 2) << SHIFT_NUM;
		teki_data[i].pat_num = PAT_TEKI1;
	}
*/
}

void init_star(void)
{
	return;
}


void bg_roll(void)
{
	unsigned char i;

	return;
}

void clr_sp(void) //unsigned char num) __sdcccall(1)
{
__asm
;	or	a,a
;	ret	z
;	push	bc
;	ld	b,a
	ld	a,(_VDP_writeadr)
	ld	c,a
clrloop:
	ld	a,216 ;0xd4
	out	(c),a

;	xor	a, a
;	out	(c),a
;	out	(c),a
;	out	(c),a
;	djnz	clrloop
;	pop	bc
__endasm;
}

unsigned char pat_num, atr, atr2, *patr; //, pal;

inline void set_spr(void)
{
__asm
;	push	af
;	push	bc
;	push	de
;	push	hl

;	ld	hl, (_pchr_data)
	ld	hl,_chr_data
;	ld	de,2

	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,(_tmp_spr_count)
	or	a
	jr	z,sprend
;	ld	b,a
	ld	d,a
	xor	a,a
sprloop2:
;	ld	a,(hl)
;	out	(c),a
;	inc	hl
	outi
;	ld	a,(hl)
;	out	(c),a
;	inc	hl
	outi
;	inc	hl		;(*)
;	ld	a,(hl)
;	out	(c),a
	outi
;	xor	a,a
;	out	(c),a
	outi
;	inc	hl
;	add	hl,de
	dec	d
	jr	nz,sprloop2
;	djnz	sprloop2
sprend:
;	pop	hl
;	pop	de
;	pop	bc
;	pop	af
__endasm;
}

void set_sprite(void)
{
	unsigned char i, j;

//	spr_page ^= 0x01;

//	tmp_spr_count = spr_count[spr_page];

/* �X�v���C�g�\�� */
//	spr_count = 2;

/* �\�����Ԃ񏑂����� */
	if(tmp_spr_count > MAX_SPRITE){
/*		if(total_count & 1){
			for(i = tmp_spr_count - MAX_SPRITE, j = 0; j < MAX_SPRITE; i++, j++){
				chr_data2[j] = chr_data[i][spr_page];
			}
			for(i = 0; i < MAX_SPRITE; i++){
				chr_data[i][spr_page] = chr_data2[i];
			}
		}*/
		tmp_spr_count = MAX_SPRITE;
	}
/*	if(tmp_spr_count < MAX_SPRITE){
		clr_sp();
	}*/

/*	for(i = 0; i < tmp_spr_count; i++){
		CHR_PARA4 *pold_data = &old_data[spr_page][i];
		pchr_data = &chr_data[i];
		if((pold_data->pat_num != pchr_data->pat_num) || (pold_data->atr != pchr_data->atr) || (pold_data->pal != pchr_data->pal)){
			color_flag[i] = 1;
			pold_data->pat_num = pchr_data->pat_num;
			pold_data->atr = pchr_data->atr;
			pold_data->pal = pchr_data->pal;
		}
	}
*/

//	goto spr_end;

	/* �F���̏��� */
//	wait_vsync();
//	DI();

//	write_vram_adr(spr_page, 0x7600);

	for(i = 0; i < tmp_spr_count; i++){
		CHR_PARA4 *pold_data = &old_data[spr_page][i];
		pchr_data = &chr_data[i];
/*__asm
	DI
__endasm;
*/
		if((pold_data->pat_num != pchr_data->pat_num) || (pold_data->atr != pchr_data->atr)){
//		if((pold_data->pat_num == pchr_data->pat_num))
//			if((pold_data->atr == pchr_data->atr))
//				if((pold_data->pal == pchr_data->pal))
//					continue;
			pold_data->pat_num = pchr_data->pat_num;
			pold_data->atr = pchr_data->atr;
//			pold_data->pal = pchr_data->pal;
//		if(color_flag[i]){
//			pchr_data = &chr_data[i];
//			color_flag[i] = 0;
			pat_num = pchr_data->pat_num / 4;
			atr = pchr_data->atr & 0xf0;

			if(!(pchr_data->atr & 0x0f)){
//				for(j = 0; j < 16; ++j){
//					write_vram_data(spr_col[pat_num][j] | atr);
//					atr2 = spr_col[pat_num][j]; // | atr;
					patr = (unsigned char *)&spr_col[pat_num][0]; // | atr;
			DI();
			write_vram_adr(spr_page, 0x7600 - 512 + i * 16);
//			EI();
__asm
	push	bc
	push	de
	push	hl
;	ld	b,16
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	hl,(_patr)
	ld	a,(_atr)
	ld	d,a
palloop:
;	ld	a,(_atr2)
	ld	a,(hl)
;	ld	b,a
	or	d
	out	(c),a	;1
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;2
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;3
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;4
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;5
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;6
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;7
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;8
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;9
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;10
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;11
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;12
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;13
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;14
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;15
	inc	hl

	ld	a,(hl)
	or	d
	out	(c),a	;16
;	inc	hl

;	djnz	palloop
	pop	hl
	pop	de
	pop	bc
__endasm;
				EI();
//				}
			}else{
//				atr2 = 13 | atr;
//				for(j = 0; j < 16; ++j){
//					write_vram_data(13 | atr);
				DI();
				write_vram_adr(spr_page, 0x7600 - 512 + i * 16);
//				EI();
__asm
	push	bc
	ld	b,16
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,(_atr)
	or	13
c_loop:
	out	(c),a
	djnz	c_loop
	pop	bc
__endasm;
///				}
				EI();
			}
//			write_vram_adr(spr_page, 0x7600  + (i) * 4);
		}
//		PUT_SP(pchr_data->x, pchr_data->y, pchr_data->pat_num, 0);

	}

	DI();
	write_vram_adr(spr_page, 0x7600);
	set_spr();

	if(tmp_spr_count < MAX_SPRITE){
		clr_sp();
	}
	EI();

spr_end:

	old_count[spr_page] = tmp_spr_count;
	tmp_spr_count = 0;

//	wait_vsync();

	DI();
//	if((read_VDPstatus(2) & 0x40)){ /* WAIT VSYNC */
//		set_spr_atr_adr(spr_page); 
//		spr_flag = 0;
//	}else 
//	if(spr_flag == 2){		/* END */
		spr_flag = 1;
		spr_next = spr_page;
//	}else if(spr_flag == 0){	/* IDLE */
//		spr_flag = 1;
//		spr_next = spr_page;
		spr_page ^= 0x01;
//	}
	EI();
	sys_wait(1);
	init_sys_wait();
//	EI();
}

void set_se(void)
{
	if(seflag == 1){
		seflag = 0;
		DI();
		write_psg(6,127);
		write_psg(11,0);
		write_psg(12,15);
		write_psg(7,0x9c);  // 10011100
		write_psg(13,9);
		write_psg(10,0x10);
		EI();
//		if(soundflag == TRUE)
//			if(se_check())
//				se_stop();
//		S_IL_FUNC(se_play(sndtable[0], seflag - 1));	/* ���ʉ� */
//		if(mcd_status >= 0){
//			pcm_play(&SNDBUFF[seflag - 1][0], pcmsize[seflag - 1]);
//		}
	}
}

/* �X�v���C�g��S�ĉ�ʊO�Ɉڂ� */
void spr_clear(void){
	DI();
	write_vram_adr(spr_page, 0x7600);
//	for(i = 0; i < MAX_SPRITE; i++){
//		PUT_SP(0, 212, 255, 0);
		clr_sp(); //MAX_SPRITE);
//	}
	EI();
	DI();
	wait_vsync();
//	spr_flag = 0;
	set_spr_atr_adr(spr_page); //, SPR_ATR_ADR); /* color table : atr-512 (0x7400) */
//	spr_next = spr_page;
	spr_page ^= 0x01;
	EI();
}

void term(void)
{
__asm
	xor	a
	ld	c,0
	call	0005h
__endasm;
}

void game_put(void)
{
	unsigned char i, j;
	tmp_spr_count = 0;
	i = 0;

/*	DI();
	write_vram_adr(spr_page, 0x7600);
	EI();
*/
	X = my_data_x[i];
	Y = my_data_y[i] ;
	PAT = my_data_pat_num[i];
	PAL = mypal;
	DEF_SP_DOUBLE(); //my_data_x[i], my_data_y[i], my_data_pat_num[i], mypal);
	SEARCH_LIST2(MAX_MYSHOT, i, j, myshot_next){
		X = myshot_x[i];
		Y =  myshot_y[i] + (4 << SHIFT_NUM_Y);
		PAT = myshot_pat_num[i] * 8;
		PAL  = CHRPAL_NO;
		DEF_SP_SINGLE(); //myshot_x[i], myshot_y[i] + (4 << SHIFT_NUM_Y), myshot_pat_num[i] * 8, CHRPAL_NO);
	}
	SEARCH_LIST2(MAX_TEKI, i, j, teki_next){
		X = teki_x[i];
		Y =teki_y[i] ;
		PAT = teki_pat[i];
		PAL = teki_pal[i];
		DEF_SP_DOUBLE(); //teki_x[i], teki_y[i], teki_pat[i], teki_pal[i]);
	}
	SEARCH_LIST2(MAX_TKSHOT, i, j, tkshot_next){
		X = tkshot_x[i];
		Y =  tkshot_y[i] + (0 << SHIFT_NUM_Y);
		PAT =  tkshot_pat[i] * 8;
		PAL  = CHRPAL_NO;
		DEF_SP_SINGLE(); //tkshot_x[i], tkshot_y[i] + (0 << SHIFT_NUM_Y), tkshot_pat[i] * 8, CHRPAL_NO);
	}

//	spr_count[spr_page] = tmp_spr_count;

//	for(i = 0; i < (32 / 2); ++i){
//		DEF_SP(*p_x + ((i * 8) << SHIFT_NUM), *p_y + ((i * 8) << SHIFT_NUM_Y), my_data_pat_num[0], mypal);
/*		DEF_SP_SINGLE(*p_x + ((i * 8) << SHIFT_NUM), *p_y + ((i * 8) << SHIFT_NUM_Y), 2*4, mypal);
		DEF_SP_SINGLE(*p_x + ((i * 8) << SHIFT_NUM), *p_y + ((i * 8) << SHIFT_NUM_Y), 3*4, mypal);*/
/*		pchr_data = &chr_data[tmp_spr_count];
		pchr_data->x = i * 8;
		pchr_data->y = i * 8;
		pchr_data->pat_num = 2*4;
//		chr_data_pal[tmp_spr_count] = mypal;
		pchr_data->atr = mypal;
		tmp_spr_count++;
		pchr_data = &chr_data[tmp_spr_count];
		pchr_data->x = i * 8;
		pchr_data->y = i * 8;
		pchr_data->pat_num = 3*4;
//		chr_data_pal[tmp_spr_count] = mypal;
		pchr_data->atr = mypal;
		tmp_spr_count++;
	}*/
}


/* �Q�|���{�̂̏��� */
short errlv = 0;
unsigned char *vdp_value = 0xf3df;
unsigned char *forclr = 0xf3e9;
unsigned char *bakclr = 0xf3ea;
unsigned char *bdrclr = 0xf3eb;
unsigned char *clicksw = 0xf3db;
unsigned char *oldscr = 0xfcb0;

unsigned char forclr_old, bakclr_old, bdrclr_old, clicksw_old;

void main(void)
{
	VDP_readadr = read_mainrom(0x0006);
	VDP_writeadr = read_mainrom(0x0007);

	forclr_old = *forclr;
	bakclr_old = *bakclr;
	bdrclr_old = *bdrclr;

	*forclr = 15;
	*bakclr = 0;
	*bdrclr = 0;
	set_screencolor();

	clicksw_old = *clicksw;
	*clicksw = 0;

	checkbgm();

	set_screenmode(5);
	set_displaypage(0);
	DI();
	write_VDP(1, vdp_value[1] | 0x02);
	EI();
	spr_on();
	boxfill(0, 256, 256, 212, 0, 0, 0x00);

//	spr_page = 0;
//	spr_count[0] = spr_count[1] = 0;
	tmp_spr_count = 0;
	old_count[0] = old_count[1] = 0; //MAX_SPRITE;

	set_int();

	do{
		wait_vsync();
//		pal_allblack(CHRPAL_NO);
		boxfill(0, 0, 256, 212, 0, 0, 0x00);

/*		sx = 0;
		sy = 0; // + 512;
		dx = 64;
		dy = 32;
		XSIZE = 128;
		XSIZA = 0;
		YSIZE = 64;
		APAGE = 2; //map_page;
		VPAGE = 0;
		VDPcommand = HMMM;
		VDPsetAREA2();
		wait_VDP();*/
		EI();

//		init_star();
		wait_vsync();

		hiscore_display();
		put_strings(SCREEN2, 9, 14, "PUSH A BUTTON", CHRPAL_NO);
		put_strings(SCREEN2, 9, 17, "      ij k   ", CHRPAL_NO);
		put_strings(SCREEN2, 9, 18, " 2023 bcdefgh", CHRPAL_NO);
		DI();
		pal_all(CHRPAL_NO, org_pal);
		EI();
		do{
			keyscan();
		}while(keycode & (KEY_A | KEY_START));
		errlv = ERRLV1;
		do{
			keyscan();
			if(!(get_key(7) & 0x04) || (keycode & KEY_B)){
				errlv = SYSEXIT;
				break;
			}
		}while(!(keycode & (KEY_A | KEY_START)));
		put_strings(SCREEN2, 9, 14, "             ", CHRPAL_NO);
		put_strings(SCREEN2, 9, 12, "             ", CHRPAL_NO);
		put_strings(SCREEN2, 9, 17, "             ", CHRPAL_NO);
		put_strings(SCREEN2, 9, 18, "             ", CHRPAL_NO);

		if((errlv == SYSERR) || (errlv == SYSEXIT))
			break;
/*		else if(errlv == NOERROR){
			for(j = 0; j != -16 * 8; j--){
				wait_vsync();
				set_constrast(j / 8, org_pal, CHRPAL_NO);
				bg_roll();
			}
			boxfill(0, 0, 256, 212, 0, 0, 0x00);
			wait_vsync();
			opening_demo();

		}else if(errlv >= ERRLV1)
*/		{

			sx = 0;
			sy = 3 * 16;
			dy = 18 * 8;
			XSIZE = 16;
			XSIZA = 0;
			YSIZE = 16;
			APAGE = 2;
			VPAGE = 0;
			VDPcommand = HMMM;
			for(i = 0; i < 16; ++i){
				dx = i * 16;
				VDPsetAREA2();
			}
			wait_VDP();

			set_vol(0);
			playbgm();

			spr_clear();

			my_hp_flag = TRUE;
			game_init();	/* �e�ϐ��̏����� */
/*
			switch(errlv){
				case ERRLV2:
					stage = 3;
					max_my_hp = 10;
					my_hp = max_my_hp;
					tkshot_c /= 6;
					score = 1;
//					renshaflag = TRUE;
					break;
				case ERRLV3:
					score = 2;
					max_my_hp = 10;
					my_hp = max_my_hp;
//					renshaflag = TRUE;
//					renshaflag = FALSE;
					break;
			}
*/
			put_strings(SCREEN2, 8, 24, "LIFE", CHRPAL_NO);
#ifdef DEBUG
			put_strings(SCREEN2, 29, 22, "FPS", CHRPAL_NO);
			put_strings(SCREEN2, 29, 24, "SPR", CHRPAL_NO);
#endif
			score_displayall();
			put_my_hp_dmg();

			init_star();

//			spr_count[0] = spr_count[1] = 0;
//			old_count[0] = old_count[1] = MAX_SPRITE;
			init_sys_wait();
			set_sprite();
			set_se();

			DI();
			write_VDP(23, 0);
			EI();
/*__asm
	DI
__endasm;
			pal_all(CHRPAL_NO, org_pal);
__asm
	EI
__endasm;*/

/* �Q�[���̃��C�����[�v */
			do{
				if(mypal_dmgtime){
					--mypal_dmgtime;
					if(!mypal_dmgtime){
						mypal = CHRPAL_NO;
					}
				}
//					if(!mypal_dmgtime){
						if(!my_hp){
							score_display();
							put_my_hp_dmg();
/*							timeup = 60 * 10;
							scrlspd = 0;
							do{
							put_strings(SCREEN2, 11, 12, "CONTINUE A", CHRPAL_NO);
								keyscan();
								game_put();
								set_sprite();
								set_se();
							}while(keycode & (KEY_A | KEY_START));
							do{
								put_numd((long)(timeup / 60), 2);
								put_strings(SCREEN2, 15, 16, str_temp, CHRPAL_NO);

								if(!(--timeup)){
*/
//									put_strings(SCREEN2, 11, 12, "           ", CHRPAL_NO);
									put_strings(SCREEN2, 11, 12, " GAME OVER ", CHRPAL_NO);
									put_strings(SCREEN2, 15, 16, "  ", CHRPAL_NO);
									scrlspd = 0; //SPR_DIV / 4;
									for(k = 0; k < 60 ; k++){
										game_put();
										set_sprite();
										bg_roll();
										set_se();
									}
									errlv = ERRLV1;
									goto end;
/*								}
								keyscan();
								if(keycode & KEY_B){
									timeup -= 5;
									if((timeup) < 1)
										timeup = 1;
								}
								game_put();
								set_sprite();
								set_se();
							}while(!(keycode & (KEY_START | KEY_A)));

							put_strings(SCREEN2, 11, 12, "           ", CHRPAL_NO);
							put_strings(SCREEN2, 15, 16, "  ", CHRPAL_NO);
							scrlspd = 0; //SPR_DIV / 4;
							score %= 10;
							if(score != 9){
								++score;
							}
							scrdspflag =TRUE;
							my_hp = max_my_hp;
							put_my_hp();
							mypal = REVPAL_NO;
							mypal_dmgtime = DMGTIME * 4;
							noshotdmg_flag = TRUE;
*/
						}
//else{
//						}
//					}
//				}

				if(scrdspflag == TRUE){
					if(score > SCORE_MAX)
								score = SCORE_MAX;
					score_display();
					scrdspflag = FALSE;
				}
				if(my_hp_flag == TRUE){
					put_my_hp_dmg();
					my_hp_flag = FALSE;
				}

				keyscan();
				if((keycode & KEY_START) || (keycode & KEY_B)){
					if(scrl_spd)
						put_strings(SCREEN2, 14, 12, "PAUSE", CHRPAL_NO);

					do{
						game_put();
						set_sprite();
						set_se();
						keyscan();
					}while((keycode & (KEY_START | KEY_B)));
					do {
						game_put();
						set_sprite();
						set_se();
						keyscan();
						if(keycode & KEY_A){
//						if(keycode & KEY_B){
//							bg_roll();
//							set_sprite();
//							return SYSEXIT;		/* ��C�ɔ����� */
//							return NOERROR;		/* ��C�ɔ����� */
							errlv = NOERROR;
							goto end;
						}
//						if(keycode & KEY_A){
//						if(keycode & KEY_B){
//							scrl_spd = SCRL_MIN;
//							bg_roll();
//							set_sprite();
//							put_strings(SCREEN2, 14, 12, "     ", CHRPAL_NO);
//							return ERRLV2;
//						}
					}while((!(keycode & (KEY_START | KEY_B))));
					do{
						game_put();
						set_sprite();
						set_se();
						keyscan();
					}while(keycode & (KEY_START | KEY_B));
					scrl_spd = SCRL_MIN;
					put_strings(SCREEN2, 14, 12, "     ", CHRPAL_NO);

//					return ERRLV1;
//					tmp_spr_count = 0;
					continue;
				}

				switch(game_loop()){
					case SYSEXIT:
						errlv = SYSEXIT;
						goto end;
					case NOERROR:
						set_sprite();
						set_se();
						bg_roll();
						break;
					default:
//						tmp_spr_count = 0;
						continue;
				}

//				if(read_VDPstatus(0) & 0x20){
//					hit_check = TRUE;
//				}else{
//					hit_check = FALSE;
//				}
			}while((scrl_spd != 0) && (get_key(7) & 0x04)); /* ESC */
			errlv = SYSEXIT;

end:
			if(errlv == SYSEXIT)
				break;
			if(errlv != NOERROR){
				switch(checkbgm()){
					case 1:
__asm
	call #0xbc06
__endasm;
					break;

					case 2:
						play_bgm(-2);
						break;
				}
				for(k = 0; k < 16 * 8; k++){
					game_put();
					set_sprite();
					set_se();
//					wait_vsync();

					DI();
					set_constrast(k / 8, org_pal, CHRPAL_NO);
					EI();

					bg_roll();
//					if(checkbgm() == 1){
						set_vol(k / 8);
//					}
				}
			}
			stopbgm();
		}
		wait_vsync();
		spr_clear();
		wait_vsync();
	}while((get_key(7) & 0x04));


/* �I������ */
	stopbgm();
	set_vol(0);

	reset_int();

	DI();
	pal_all(CHRPAL_NO, org_pal);
	EI();

	*forclr = forclr_old;
	*bakclr = bakclr_old;
	*bdrclr = bdrclr_old;
	set_screencolor();

	set_screenmode(*oldscr);

	*clicksw = clicksw_old;

	key_flush();
//	term();
//	exit(0);
}


void intvsync(void)
{
__asm
;intvsync:
	ld	(_vdps0),a
	push	af
;	push	ix
__endasm;

	if(spr_flag == 1){
		spr_flag = 0;
//		set_spr_atr_adr(spr_next); 
//		write_VDP(11, ((spr_next << 1) & 0x02));
__asm
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	ld	a,(_spr_next)
	add	a,a
	and	a,#0x02
	out	(c),a
	ld	a,11 | 0x80
	out	(c),a
__endasm;
	}
__asm
;	pop	ix
	pop	af
INTWORK:
	DB	0,0,0,0,0
__endasm;
}

void set_int(void)
{
__asm
	DI
;	PUSH	IY
;	PUSH	HL
;	PUSH	DE
;	PUSH	BC
	LD	IY,-609
	PUSH	IY
	POP	HL
	LD	DE,INTWORK
	LD	BC,5
	LDIR
	LD	HL,_intvsync
	LD	(IY+2),L
	LD	(IY+3),H
	LD	(IY+0),0F7H
	LD	(IY+4),0C9H

	ld	a,h
	ld	hl,#0xf341	;slot0
	cp	#0x40
	jr	c,slotset
	inc	hl			;slot1
	cp	#0x80
	jr	c,slotset
	inc	hl			;slot2
	cp	#0xc0
	jr	c,slotset
	inc	hl			;slot3
slotset:
	LD	A,(HL)
	LD	(IY+1),A

;	POP	BC
;	POP	DE
;	POP	HL
;	POP	IY
	EI
__endasm;
}

void reset_int(void)
{
__asm
	DI
	LD	HL,INTWORK
	LD	DE,-609
	LD	BC,5
	LDIR
	EI
__endasm;
}
