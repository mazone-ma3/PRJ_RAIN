/* rainamg.c */

#include <stdio.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>

#include <hardware/cia.h>

#include <proto/exec.h>
#include <proto/lowlevel.h>

#define CUSTOM_BASE 0xDFF000
#define CUSTOM (*(volatile struct Custom *)CUSTOM_BASE)

#define bplpt 0x0e0
#define sprpt 0x120


enum {
	BPL1PTH = bplpt + 0x00,
	BPL1PTL = bplpt + 0x02,

	SPR0PT  = sprpt  + 0x00,
	SPR0PTH = SPR0PT + 0x00,
	SPR0PTL = SPR0PT + 0x02,
	SPR1PT  = sprpt  + 0x04,
	SPR1PTH = SPR1PT + 0x00,
	SPR1PTL = SPR1PT + 0x02,
	SPR2PT  = sprpt  + 0x08,
	SPR2PTH = SPR2PT + 0x00,
	SPR2PTL = SPR2PT + 0x02,
	SPR3PT  = sprpt  + 0x0C,
	SPR3PTH = SPR3PT + 0x00,
	SPR3PTL = SPR3PT + 0x02,
	SPR4PT  = sprpt  + 0x10,
	SPR4PTH = SPR4PT + 0x00,
	SPR4PTL = SPR4PT + 0x02,
	SPR5PT  = sprpt  + 0x14,
	SPR5PTH = SPR5PT + 0x00,
	SPR5PTL = SPR5PT + 0x02,
	SPR6PT  = sprpt  + 0x18,
	SPR6PTH = SPR6PT + 0x00,
	SPR6PTL = SPR6PT + 0x02,
	SPR7PT  = sprpt  + 0x1C,
	SPR7PTH = SPR7PT + 0x00,
	SPR7PTL = SPR7PT + 0x02,
};

unsigned short COPPERL[] = {
	BPL1PTH,0x0002, //;Bitplane 1 pointer = $21000
	BPL1PTL,0x1000,
	SPR0PTH,0x0002, //;Sprite 0 pointer = $25000
	SPR0PTL,0x5000,
	SPR1PTH,0x0003, //;Sprite 1 pointer = $30000
	SPR1PTL,0x0000,
	SPR2PTH,0x0003, //;Sprite 2 pointer = $30000
	SPR2PTL,0x0000,
	SPR3PTH,0x0003, //;Sprite 3 pointer = $30000
	SPR3PTL,0x0000,
	SPR4PTH,0x0003, //;Sprite 4 pointer = $30000
	SPR4PTL,0x0000,
	SPR5PTH,0x0003, //;Sprite 5 pointer = $30000
	SPR5PTL,0x0000,
	SPR6PTH,0x0003, //;Sprite 6 pointer = $30000
	SPR6PTL,0x0000,
	SPR7PTH,0x0003, //;Sprite 7 pointer = $30000
	SPR7PTL,0x0000,
	0xFFFF,0xFFFE, //End of Copper list
};
//	;;
//	;;  Sprite data
//	;;
unsigned short SPRITE[] = {
	0x6D60,0x7200, //VSTART, HSTART, VSTOP

	 0x07f0, 0x0808,
	 0x0808, 0x17f4,
	 0x13e4, 0x2c1a,
	 0x1ffc, 0x2002,
	 0x16ec, 0x2912,
	 0x16ec, 0x2912,
	 0x1ddc, 0x2ffa,
	 0x1ddc, 0x0ff8,
	 0x0ff8, 0x03e0,
	 0x1ffc, 0x1ffc,
	 0x300f, 0x6ff3,
	 0x3013, 0x6fef,
	 0x1024, 0x1fde,
	 0x0ff8, 0x01e0,
	 0x0000, 0x0ff8,
	 0x0630, 0x19cc,

//	0x0990,0x07E0, //First pair of descriptor words
//	0x13C8,0x0FF0,
//	0x23C4,0x1FF8,
//	0x13C8,0x0FF0,
//	0x0990,0x07E0,

	0x8080,0x8D00,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,

	0x0000,0x0000, //End of sprite data
};


int main(void)
{
	short i = 0;
	unsigned long *pointer, *pointer2;

	unsigned char h_count = 151, v_count = 194, h_pos = 64, v_pos = 44, h_inc = 1, v_inc = 1;

	struct Library *LowLevelBase = OpenLibrary("lowlevel.library", 39L);
	if (!LowLevelBase) {
//		printf("cannot open lowlevel.library\n");
		return 10;
	}

//	CUSTOM.bltddat = 0;
	CUSTOM.bplcon0 = 0x1200;
	CUSTOM.bpl1mod = 0x0000; //MOVE.W  #$0000,BPL1MOD(a0) ;Modulo = 0
	CUSTOM.bplcon1 = 0x0000; //MOVE.W  #$0000,BPLCON1(a0) ;Horizontal scroll value = 0
	CUSTOM.bplcon2 = 0x0024; //	MOVE.W  #$0024,BPLCON2(a0) ;Sprites have priority over playfields
	CUSTOM.ddfstrt = 0x0038; //	MOVE.W  #$0038,DDFSTRT(a0) ;Set data-fetch start
	CUSTOM.ddfstop = 0x00d0; //	MOVE.W  #$00D0,DDFSTOP(a0) ;Set data-fetch stop

	CUSTOM.diwstrt = 0x2c81;
	CUSTOM.diwstop = 0xf4c1;

	/* RGB */
	CUSTOM.color[0] = 0x0008;
	CUSTOM.color[1] = 0x0000;
	CUSTOM.color[17] = 0xff0;//0x0ff0;
	CUSTOM.color[18] = 0xf00;//0x00ff;
	CUSTOM.color[19] = 0xfff;//0x0f0f;

	pointer = (unsigned long *)0x20000L;
	pointer2 = (unsigned long *)COPPERL;

	do{
		*(pointer++) = *(pointer2++);
	}while(*pointer2 != 0xfffffffe);

	pointer = (unsigned long *)0x25000L;
	pointer2 = (unsigned long *)SPRITE;

	do{
		*(pointer++) = *(pointer2++);
	}while(*pointer2 != 0x0);

	*((unsigned long *)0x30000L) = 0;

	CUSTOM.cop1lc = 0x20000L; //MOVE.L  #$20000,COP1LC(a0)

	pointer = (unsigned long *)0x21000L;
	i = 1999+1;
	do{
		*(pointer++) = 0xffffffff;
	}while(--i);

	CUSTOM.copjmp1 = 0;
	CUSTOM.dmacon = 0x83a0;
/*
	CUSTOM.color[0] = 0x0008;
	CUSTOM.color[1] = 0x0000;
	CUSTOM.color[7] = 0x0ff0;
	CUSTOM.color[8] = 0x00ff;
	CUSTOM.color[9] = 0x0f0f;
*/

	CUSTOM.dmacon = DMAF_SETCLR | DMAF_SPRITE;

	for(;;){
		ULONG joy =ReadJoyPort(1);

		// ビットチェック
		if ((joy & JPF_JOY_UP)){    /* 上 */;
			v_pos -= v_inc;
		}
		if ((joy & JPF_JOY_DOWN)){  /* 下 */;
			v_pos += v_inc;
		}
		if ((joy & JPF_JOY_LEFT)){  /* 左 */;
			h_pos -= h_inc;
		}
		if ((joy & JPF_JOY_RIGHT)){ /* 右 */;
			h_pos += h_inc;
		}

		if ((joy & JPF_BTN1)){  /* 火ボタン（メイン） */;
			 h_pos = 64;
			 v_pos = 44;
		}

		while(((CUSTOM.vhposr / 256))); // == 0x20));

		*((unsigned char *)0x25001) = h_pos;	// H_START

		*((unsigned char *)0x25000) = v_pos;		// V_START
		*((unsigned char *)0x25002) = v_pos + 16;	// V_STOP
	}

	return(0);
}

