/* C64 Sample rain64.c by m@3 */
/* llvm-mos または cc65でコンパイルする */

#include <stdio.h>

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))

/* BITセット */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

enum {
	COLOR_BLACK       = 0,
	COLOR_WHITE       = 1,
	COLOR_RED         = 2,
	COLOR_CYAN        = 3,
	COLOR_PURPLE      = 4,
	COLOR_GREEN       = 5,
	COLOR_BLUE        = 6,
	COLOR_YELLOW      = 7,
	COLOR_ORANGE      = 8,
	COLOR_BROWN       = 9,
	COLOR_LIGHT_RED   = 10,
	COLOR_GRAY_ONE    = 11,
	COLOR_DARK_GREY   = 11,
	COLOR_GRAY_TWO    = 12,
	COLOR_MEDIUM_GREY = 12,
	COLOR_LIGHT_GREEN = 13,
	COLOR_LIGHT_BLUE  = 14,
	COLOR_GRAY_THREE  = 15,
	COLOR_LIGHT_GREY  = 15,
};

unsigned char *SPRITE_POINTERS = (unsigned char *)0x07f8;
unsigned char *SPRITE_ENABLE_REGISTER = (unsigned char *)0xd015;
unsigned char *SPRITE_0_COLOR_REGISTER = (unsigned char *)0xd027;
unsigned char *SPRITE_MULTICOLOR_REGISTERS = (unsigned char *)0xd01c;
unsigned char *SPMC0 = (unsigned char *)0xd025;
unsigned char *SPMC1 = (unsigned char *)0xd026;

unsigned char *MSGIGX = (unsigned char *)0xd010;
unsigned char *SP0X = (unsigned char *)0xd000;
unsigned char *SP0Y = (unsigned char *)0xd001;
unsigned char *SPXX = (unsigned char *)0xd010;

volatile unsigned char *RASTER = (unsigned char *)0xd012;

unsigned char *spr0ShapeData = (unsigned char *)0x340; /* 13 * 64 */

unsigned char *porta = (unsigned char *)0xdc00;
unsigned char *portb = (unsigned char *)0xdc01;
unsigned char *portadir = (unsigned char *)0xdc02;
unsigned char *portbdir = (unsigned char *)0xdc03;

unsigned char *BoaderColorRegister = (unsigned char *)0xd020;
unsigned char *BackgroundColor = (unsigned char *)0xd021;

unsigned char *screenarea0 = (unsigned char *)0x0400;
unsigned char *screenarea1 = (unsigned char *)0x0500;
unsigned char *screenarea2 = (unsigned char *)0x0600;
unsigned char *screenarea3 = (unsigned char *)0x06e8;

unsigned char *foregroundarea0 = (unsigned char *)0xd800;
unsigned char *foregroundarea1 = (unsigned char *)0xd900;
unsigned char *foregroundarea2 = (unsigned char *)0xda00;
unsigned char *foregroundarea3 = (unsigned char *)0xdae8;

unsigned char *interrupt = (unsigned char *)0xdc0e;
unsigned char *charset = (unsigned char *)0x001L;
unsigned char *charsetram = (unsigned char *)0xd018;
unsigned char *chardata, *chardata2;

unsigned char *SCREENCONTROL1 = (unsigned char *)0xd011;
unsigned char *SCREENCONTROL2 = (unsigned char *)0xd016;

unsigned char *EXTBGCOOLR1 = (unsigned char *)0xd022;
unsigned char *EXTBGCOOLR2 = (unsigned char *)0xd023;

unsigned char spr0[64] = {
	 0x1a, 0x90, 0x00,
	 0x65, 0x64, 0x00,
	 0x9a, 0x98, 0x00,
	 0xaa, 0xa8, 0x00,
	 0x99, 0x98, 0x00,
	 0xb7, 0x78, 0x00,
	 0xb7, 0x78, 0x00,
	 0x2f, 0xe0, 0x00,
	 0x3f, 0xf0, 0x00,
	 0xe6, 0x6c, 0x00,
	 0xe5, 0x6c, 0x00,
	 0x35, 0x90, 0x00,
	 0x1d, 0xb0, 0x00,
	 0x1b, 0xe0, 0x00,
	 0x55, 0x54, 0x00,
	 0x69, 0xa4, 0x00,
	 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x00,
	 0,
};

unsigned char font[] = {
	 0x1f,
	 0x3e,
	 0x3a,
	 0x00,
	 0xf1,
	 0xe3,
	 0xa3,
	 0x00,
};

char *bgram;
unsigned char *cram;

void put_strings(unsigned char x, unsigned char y,  char *str, unsigned char pal)
{
	char chr;
	unsigned short i = 0;
	bgram = (char *)0x0400 + (x + y * 40);
	cram = (unsigned char *)0xd800 + (x + y * 40);

//	if(cram > (char *)0x0dbe7)
//		return;

	while((chr = *(str++)) != '\0'){
//		if((chr < 0x30)) //|| (chr > 0x5f))
//			chr = 0x20;
//		else
//			chr -= '0';
		chr &= 0x3f;
//		if((chr >= 0x40))
//			chr -= 0x40;
		*(bgram++) = chr;
		*(cram++) = pal;
	}
}

char str_temp[9];

void put_numd(long j, char digit)
{
	char i = digit;

	while(i--){
		str_temp[i] = j % 10 + 0x30;
		j /= 10;
	}
	str_temp[digit] = '\0';
}

void copy(void)
{
//	register unsigned char ra asm("a");

//	asm volatile(
//		"txa\n"
//		"pha\n"
//		"tya\n"
//		"pha\n"
/*		"lda #0\n"
		"ldy #$d0\n"
		"sta 95\n"
		"sty 96\n"
		"lda #0\n"
		"ldy #$e0\n"
		"sta 90\n"
		"sty 91\n"
		"lda #0\n"
		"ldy #$40\n"
		"sta 88\n"
		"sty 89\n"
		"jsr $a3bf\n"*/
//		"pla\n"
//		"tay\n"
//		"pla\n"
//		"tax\n"
//		://"=a"(ra)	/* 値が返るレジスタ変数 */
//		://"a"(ra)	/* 引数として使われるレジスタ変数 */
//		:"%a","%x","%y"		/* 破壊されるレジスタ */
//	);
}

short x; // = 100;

unsigned char i = 0, keycode, oldcharset, data = 0;
//unsigned char j = 0;
short j;

int main(void)
{
	//Copy font ROM -> RAM
	*interrupt = *interrupt & 0xfe;
	oldcharset = *charset;
	*charset = *charset & 0xfb;
	*charset |= 0x03;	// BASIC ROM

//	copy();
	for(j = 0, chardata = (unsigned char *)0x3800, chardata2 = (unsigned char *)0xd000; j < 4096; ++j, ++chardata, ++chardata2){
		*chardata = *chardata2;
	}
/*	for(chardata = (unsigned char *)53248; chardata <= (unsigned char *)57343; ++chardata){
			*(chardata - 40960) = *chardata;
	}*/

//	*charset = *charset | 0x04;
	*charset = oldcharset;
	*interrupt = *interrupt | 0x01;
	*charsetram = *charsetram & 0xf0 | 0x0e;

//	register unsigned char j;
	for(chardata = (unsigned char *)0x38d8, j = 0; j < 8; ++chardata, ++j){
		data = font[j];
		*chardata = data;
		*(chardata + 1024) = 255 - data;
	}

/*	asm(
		"txa\n"
		"pha\n"
		"tya\n"
		"pha\n"
		"jsr $e544\n"
		"pla\n"
		"tay\n"
		"pla\n"
		"tax\n"
	);*/

	// Clear Screen Black
	*BoaderColorRegister = 0;
	*BackgroundColor = 0;

	*SCREENCONTROL1 = *SCREENCONTROL1 & 0x9f;
	*SCREENCONTROL2 = *SCREENCONTROL2 | 0x10;

	*EXTBGCOOLR1 = COLOR_WHITE;
	*EXTBGCOOLR2 = COLOR_YELLOW;

	for(;;){
		screenarea0[i] = screenarea1[i] = screenarea2[i] = screenarea3[i] = 0x20;

		foregroundarea0[i] = foregroundarea1[i] = foregroundarea2[i] = foregroundarea3[i] = 0x01;
		++i;
		if(i == 0)
			break;
	}

	// Put Strings
//	char *bgram, *cram;
//	bgram = (char *)0x400;
//	cram = (char *)0xd800;
/*	for(i = 0x0; i < 0xff; ++i){
//		__putchar(i);
		*bgram++ = 0x1f;
		*cram++ = COLOR_LIGHT_RED;
	}*/
//	put_strings(0, 0, "01234567890", COLOR_WHITE );
//	put_strings(0, 1, "HELLO WORLD\x5f", COLOR_WHITE );


	for(i = 0; i < 40; ++i){
		put_strings(i, 18, "\x5b", COLOR_LIGHT_RED);
		put_strings(i, 19, "\x5b", COLOR_LIGHT_RED);
	}
	put_strings(12, 22, "SCORE 000000000", COLOR_WHITE );
	put_strings(12, 24, "LIFE", COLOR_WHITE );


	// Set SPRITE Data
	*SPRITE_POINTERS = 0x0d;	/* 13 */
	*SPRITE_ENABLE_REGISTER = 0x01;
	*SPRITE_MULTICOLOR_REGISTERS = 0x01;
	*SPRITE_0_COLOR_REGISTER = COLOR_YELLOW; //LIGHT_RED; //COLOR_YELLOW;
	*SPMC0 = COLOR_LIGHT_RED; //YELLOW; //WHITE;
	*SPMC1 = COLOR_WHITE; //LIGHT_RED;

	for(i = 0; i < 63; i++){
		spr0ShapeData[i] = spr0[i];
	}

	x = 176;
	*MSGIGX = 0;
	*SP0X = x;
	*SP0Y = 178;

	// Main Loop
	for(;;){
		keycode = 0;

		/* PAD */
		*portadir = 224;
		*porta = 0xff;
		if(!BITTST(0, *portb)){	/* UP */
			BITSET(0, keycode);
		}
		if(!BITTST(1, *portb)){	/* DOWN */
			BITSET(1, keycode);
		}
		if(!BITTST(2, *portb)){	/* LEFT */
			BITSET(2, keycode);
		}
		if(!BITTST(3, *portb)){	/* RIGHT */
			BITSET(3, keycode);
		}
		if(!BITTST(4, *portb)){	/* SHOT */
			BITSET(4, keycode);
		}

		if(keycode)
			goto skip;

		/* KEYBOARD */
		*portadir = 255;
		*porta = 0xef;

		if(!BITTST(1, *portb)){	/* I */
			BITSET(0, keycode);
		}
		if(!BITTST(4, *portb)){	/* M */
			BITSET(1, keycode);
		}
		if(!BITTST(2, *portb)){	/* J */
			BITSET(2, keycode);
		}
		if(!BITTST(5, *portb)){	/* K */
			BITSET(3, keycode);
		}
		if(!BITTST(7, *portb)){	/* N */
			BITSET(4, keycode);
		}
		if(!BITTST(3, *portb)){	/* 0 */
			BITSET(5, keycode);
		}

		*porta = 0x7f;
		if(!BITTST(4, *portb)){	/* ' ' */
			BITSET(4, keycode);
		}
		if(!BITTST(1, *portb)){	/* <- */
			BITSET(5, keycode);
		}
		if(!BITTST(7, *portb)){	/* STOP */
			BITSET(5, keycode);
		}

		*porta = 0xfd;
		if(!BITTST(4, *portb)){	/* Z */
			BITSET(4, keycode);
		}


skip:
		if(BITTST(0, keycode)){	/* UP */
			--(*SP0Y);
		}
		if(BITTST(1, keycode)){	/* DOWN */
			++(*SP0Y);
		}
		if(BITTST(2, keycode)){	/* LEFT */
			--x; //(*SP0X);
		}
		if(BITTST(3, keycode)){	/* RIGHT */
			++x; //(*SP0X);
		}
		if(BITTST(4, keycode)){	/* SHOT */
			++(*SPRITE_0_COLOR_REGISTER);
		}
		if(BITTST(5, keycode)){	/* END */
			break;
		}

		*SP0X = x & 0xff;
		*SPXX = (x >> 8)  & 0x01;
		while(*RASTER != 249);

	}

	*SCREENCONTROL2 = *SCREENCONTROL2 & 0xef;

	return 0;
}
