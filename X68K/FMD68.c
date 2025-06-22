/* FM MUSIC SAMPLE for X68K */
/* GCC(elf2x68k) */
/* �Q�l PC-9801�}�V����T�E���h�v���O���~���O�AInside X68000 */
/* �v���O���}�[�̂��߂�X68000���n���h�u�b�N */

#include "fmd68.h"

#include <stdio.h>
#include <stdlib.h>
#include <x68k/iocs.h>

#include "key.h"
#include "keyx68.h"
#include "tone.h"

#define IOCS

FILE *fmdstream[2];

#define ERROR 1
#define NOERROR 0

#define PARTSUU 8

volatile unsigned char COUNT[PARTSUU] = {1,1,1,1,1,1,1,1}; // �����J�E���^
volatile unsigned char STOPFRG[PARTSUU] = {0,0,0,0,0,0,0,0}; // WAIT&SYNC&OFF
volatile unsigned char MAXVOL[PARTSUU] = {15,15,15,15,15,15,15,15}; // �{�����[��
volatile unsigned char LENGTH[PARTSUU] = {5,5,5,5,5,5,5,5}; // ��{����

volatile unsigned short OFFSET[PARTSUU] = {0,0,0,0,0,0,0,0}; // ���t�f�[�^���s���A�h���X
volatile unsigned short STARTADR[PARTSUU] = {0,0,0,0,0,0,0,0}; // ���t�f�[�^�J�n�A�h���X
volatile unsigned char FEEDVOL = 0; // �t�F�[�h�A�E�g���x��
volatile unsigned char FCOUNT = 1; //�t�F�[�h�A�E�g�J�E���^
volatile unsigned char LOOPTIME = 0; // ���t�񐔁i�O�͖������[�v�j
volatile unsigned char STOPPARTS = 0; //
volatile unsigned char ENDFRG = 0; //
volatile unsigned char NSAVE = 0; //


unsigned char mem[65536];

short bload2(char *loadfil, unsigned short offset)
{
	unsigned short size;
	unsigned short address;
	unsigned char buffer[3];

	if ((fmdstream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, 2, fmdstream[0]);
	address = buffer[0] + buffer[1] * 256;
	fread( buffer, 1, 2, fmdstream[0]);
	size = (unsigned short)(buffer[0] + buffer[1] * 256) - (unsigned short)address;
	address -= offset;
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address , size, (unsigned short)address + size);

	fread( &mem[address] , 1, size, fmdstream[0]);
	fclose(fmdstream[0]);
	return NOERROR;
}

#define port1 ((volatile unsigned char *)0xe90001)
#define port2 ((volatile unsigned char *)0xe90003)

/* FM���W�X�^�ݒ� */
void set_fm(unsigned char reg, unsigned char data)
{
#ifdef IOCS
	_iocs_opmset(reg, data);
#else
	unsigned char a;

	while(*port2 & 0x80);
dummy:
	*port1 = reg;
	while (*port2 & 0x80);
dummy2:
	*port2 = data;
#endif
}

/*  AR  DR  SR  RR  SL  OL  KS  ML DT1 DT2 AME */
enum { AR, DR, SR, RR, SL, OL, KS, MUL,DT1,DT2,AME };
enum { D1R = DR , TL=OL, D1L = SL, D2R=SR};
enum {CON = 4 * 11, FL};

char op[4] = {0, 2, 1, 3};

/* ���F�ݒ� */
void set_tone(int no, int ch)
{
	unsigned char i, j, k;

	for(i = 0; i < 4; ++i){
		j = ch + op[i] * 8;
		k = i * 11;
		set_fm(0x40 + j, tone_table[no][MUL + k] | tone_table[no][DT1 + k] * 16);
		set_fm(0x60 + j, tone_table[no][TL + k]);
		set_fm(0x80 + j, tone_table[no][AR + k] | tone_table[no][KS + k] * 64);
		set_fm(0xa0 + j, tone_table[no][D1R + k] | tone_table[no][AME + k] * 128);
		set_fm(0xc0 + j,tone_table[no][D2R + k] | tone_table[no][DT2 + k] * 64);
		set_fm(0xe0 + j, tone_table[no][RR + k] | tone_table[no][D1L + k] * 16);
	}
	j = 0x20 + ch;
	set_fm(j, tone_table[no][CON] | tone_table[no][FL] * 8 | 0xc0);

/*
	for(i = 0; i < 4; ++i){
	j = 0x40 + ch;
	for(i = 0; i < 24; ++i){
		set_fm(j, tone_table[no][i]);
		j+=8;
	}

	j += 0x20;
	i += 4;
	set_fm(j, tone_table[no][i] | 0xc0);
*/
}

/* ���K�ݒ� */
void set_key(int no, int ch)
{
	set_fm(0x28 + ch, key_table[no]);
	set_fm(0x30 + ch, 5);
}

char key[8] = {0, 1, 2, 3, 4, 5, 6, 7};


void stop(void);

unsigned char count = 0;

//void  __attribute__((interrupt))int_fm(void)
//{
//}

void  __attribute__((interrupt))int_fm(void)
{
	unsigned char i, j, no, ch;
	unsigned char data;

	/* ���荞�� on */
	asm volatile("andi.w	#0x0f8ff,%sr\n");

	set_fm(0x14, 0x2a);

playloop:
playloop2:
//	if(FEEDVOL == 15)
//		stop();
//	if(FEEDVOL)
//		feedsub();
	for(i = 0; i < PARTSUU; ++i){
		if(STOPFRG[i] >= 254){
			/* �����҂��E���t�I�� ���t�X�L�b�v */
			continue;
		}
		--COUNT[i];
		if(!(COUNT[i])){
			/* ���t���� */
			for(;;){
				data = mem[OFFSET[i]];
				switch(data){
					case 226:	/* ���ʕύX V */
						data = mem[--OFFSET[i]];
						MAXVOL[i] = data;
						break;
					case 227:	/* �W�������ύX T */
						data = mem[--OFFSET[i]];
						LENGTH[i] = data;
						break;
					case 228:	/* ���F�ύX */
						no = mem[--OFFSET[i]];
						set_tone(no, i);
						break;
					case 225:	/* ���ڏo�� Y */
						ch = mem[--OFFSET[i]];
						no = mem[--OFFSET[i]];
						set_fm(ch, no);
						break;
					case 255:	/* ���[�v */
						OFFSET[i] = STARTADR[i] + 1;
						ENDFRG = 1;
						break;
					case 254:	/* ���� */
						STOPFRG[i] = 254;
						--OFFSET[i];
						++STOPPARTS;
						goto playend;
						break;
					default:
						/* ���t */
						STOPFRG[i] = data & 0x7f;
						set_fm(0x08, 0x00 | key[i]);	/* off */
						if((data & 0x7f) != 0){
							set_key((data & 0x7f) - 1, i);	/* key */
							set_fm(0x08, 0xf0 | key[i]);	/* on */
						}
						if(data & 0x80){	/* �������ݒ肳��Ă��� */
							data = LENGTH[i];
						}else{
							data = mem[--OFFSET[i]];
						}
						COUNT[i] = data;
						--OFFSET[i];
						goto playend;
						break;
				}
				--OFFSET[i];
			}
		}
playend:
		continue;
	}
	if(PARTSUU == STOPPARTS){	/* ���t�p�[�g��=��~�p�[�g�� */
		j = PARTSUU;
		for(i = 0; i < PARTSUU; ++i){
			if(STOPFRG[i] != 255){	/* ���t��~�łȂ� */
				STOPFRG[i] = 0;	/* ���t�� */
				COUNT[i] = 1;	/* �����J�E���^��1�ɂ��� */

				--j;		/* ���t��~�p�[�g���̌v�Z */
			}
		}
		STOPPARTS = j;	/* ���t��~�p�[�g�� */
		goto playloop2;
	}

	/* ���荞�ݏI�� */
	if((--ENDFRG) == 0){	/* �I���J�E���^��0�Ȃ�I�� */

		ENDFRG = 0;
		if(!LOOPTIME)
			goto playend2;
//			return;			/* �������[�v */
		if(--LOOPTIME){
			goto playloop;	/* ���[�v�񐔂�0�ȊO�Ȃ烋�[�v */
		}
		/* ���t���Ȓ�~ */
		stop();
	}
	ENDFRG = 0;
playend2:
//	asm volatile("andi.w	#0x0f8ff,%sr\n");
}

static volatile uint8_t s_mfpBackup[0x18] = {
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
};
static volatile uint32_t s_vector118Backup = 0;
static volatile uint32_t s_uspBackup = 0;

int init_sndint(void)
{
	int ret = 0;
	/* ���荞�� off */
	asm volatile("ori.w	#0x0700,%sr\n");

#ifdef DEBUG
	ret = _iocs_opmintst(int_fm);

	asm volatile (
		"AER		= 0x003\n"
		"IERA		= 0x007\n"
		"IERB		= 0x009\n"
		"ISRA		= 0x00F\n"
		"ISRB		= 0x011\n"
		"IMRA		= 0x013\n"

		"IMRB		= 0x015\n"

//		"	lea.l	int_vsync,%a2\n"

		/* MFP �̃o�b�N�A�b�v����� */
		"	movea.l	#0x0e88000,%a0\n"			/* a0.l = MFP�A�h���X */
//		"	lea.l	s_mfpBackup(%pc),%a1\n"		/* a1.l = MFP�ۑ���A�h���X */
		"	lea.l	s_mfpBackup,%a1\n"		/* a1.l = MFP�ۑ���A�h���X */
		"	move.b	AER(%a0),AER(%a1)\n"		/*  AER �ۑ� */
//		"	move.b	IERB(%a0),IERB(%a1)\n"		/* IERB �ۑ� */
		"	move.b	IMRB(%a0),IMRB(%a1)\n"		/* IMRB �ۑ� */
//		"	move.b	IMRA(%a0),IMRA(%a1)\n"		/* IMRA �ۑ� */

//		"	move.l	#0x118,s_vector118Backup\n"	/* �ύX�O�� V-disp �x�N�^ */

		/* ���荞�ݐݒ� */
//		"	move.l	%a2,0x118\n"				/* �x�N�^������ */
//		"	bclr.b	#4,AER(%a0)\n"				/* �A�����ԂƓ����Ɋ��荞�� */
//		"	bset.b	#6,IMRB(%a0)\n"				/* �}�X�N���͂��� */
//		"	bset.b	#6,IERB(%a0)\n"				/* ���荞�݋��� */

		"	bclr.b	#3,IMRB(%a0)\n"				/* �}�X�N���͂��� */
//		"	bset.b	#3,IMRA(%a0)\n"				/* �}�X�N���͂��� */

//		:"=d"(rd0)
//		:"d"(rd0),"d"(rd1),"a"(ra0),"a"(ra1),"a"(ra2) //,"r"(rpc)
	);

#else
//	ret = _iocs_vdispst (int_vsync, 0, 0*256+1);
	ret = _iocs_opmintst(int_fm);
#endif


	/* ���荞�� on */
	asm volatile("andi.w	#0x0f8ff,%sr\n");
	return ret;
}

void reset_sndint(void)
{
	asm volatile("ori.w	#0x0700,%sr\n");

#ifdef DEBUG
	_iocs_opmintst (0);

	asm volatile(
		"AER		= 0x003\n"
		"IERA		= 0x007\n"
		"IERB		= 0x009\n"
		"ISRA		= 0x00F\n"
		"ISRB		= 0x011\n"
		"IMRA		= 0x013\n"
		"IMRB		= 0x015\n"

		/* MFP �̐ݒ�𕜋A */
		"	movea.l	#0x0e88000,%a0\n"					/* a0.l = MFP�A�h���X */
//		"	lea.l	s_mfpBackup(%pc),%a1\n"			/* a1.l = MFP��ۑ����Ă������A�h���X */
		"	lea.l	s_mfpBackup,%a1\n"			/* a1.l = MFP��ۑ����Ă������A�h���X */

//		"	move.b	AER(%a1),%d0\n"
//		"	andi.b	#%%0101_0000,%d0\n"
//		"	andi.b	#0x50,%d0\n"
//		"	andi.b	#%%1010_1111,AER(%a0)\n"
//		"	andi.b	#0xaf,AER(%a0)\n"
//		"	or.b	%d0,AER(%a0)\n"					/* AER bit4&6 ���A */

//		"	move.b	IERB(%a1),%d0\n"
//		"	andi.b	#%%0100_0000,%d0\n"
//		"	andi.b	#0x40,%d0\n"
//		"	andi.b	#%%1011_1111,IERB(%a0)\n"
//		"	andi.b	#0xbf,IERB(%a0)\n"
//		"	or.b	%d0,IERB(%a0)\n"					/* IERB bit6 ���A */

//		"	move.b	IMRA(%a1),%d0\n"
//		"	andi.b	#0x08,%d0\n"
//		"	andi.b	#0xf7,IMRA(%a0)\n"
//		"	or.b	%d0,IMRA(%a0)\n"					/* IMRA bit3 ���A */

		"	move.b	IMRB(%a1),%d0\n"
		"	andi.b	#0x08,%d0\n"
		"	andi.b	#0xf7,IMRB(%a0)\n"
		"	or.b	%d0,IMRB(%a0)\n"					/* IMRB bit3 ���A */

//		"	move.b	IMRB(%a1),%d0\n"
//		"	andi.b	#%%0100_0000,%d0\n"
//		"	andi.b	#0x40,%d0\n"
//		"	andi.b	#%%1011_1111,IMRB(%a0)\n"
//		"	andi.b	#0xbf,IMRB(%a0)\n"
//		"	or.b	%d0,IMRB(%a0)\n"					/* IMRB bit6 ���A */

		/* V-DISP ���荞�ݕ��A */
//		"	move.l	s_vector118Backup(%pc),0x118\n"
//		"	move.l	s_vector118Backup,0x118\n"
//		:"=d"(rd0)
//		:"d"(rd0),"d"(rd1),"a"(ra0),"a"(ra1)
	);
#else
	_iocs_opmintst (0);
#endif
	asm volatile("andi.w	#0x0f8ff,%sr\n");
}

void stop(void)
{
	unsigned char i;
	set_fm(0x14, 0x0);
	for(i = 0; i < PARTSUU; ++i){
		set_fm(0x08, 0x00 | key[i]);	/* off */
	}
}
/*
int	main(int argc,char **argv)
{
	unsigned short no = 0;
	unsigned char i, ch = 0;
	unsigned char noise = 0;

dum:	_iocs_b_super(0);		// �X�[�p�[�o�C�U���[�h �œK���h�~�Ƀ��x����t����

	if (argc < 2){
		exit(1);
	}


	if (argc >= 3){
		no = atoi(argv[2]);
		if((no % 256) > 9)
			no = 0;
	}

	if (argc >= 4){
		noise = atoi(argv[3]);
	}
}
*/
void load_fmdbgm(char *filename)
{
//	if(bload2(argv[1], 0x1000*0) == ERROR)
	if(bload2(filename, 0x1000*0) == ERROR)
		exit(1);
}

void play_fmdbgm(void)
{
	unsigned short no = 0;
	unsigned char i, ch = 0;
	unsigned char noise = 0;

	FEEDVOL = 0;
	FCOUNT = 1;
	STOPPARTS = 0;
	ENDFRG = 0;
	NSAVE = 0;
	for(i = 0; i < PARTSUU; ++i){
		int j = 0xdb00-0x1100 + i * 2 + (no % 256) * 12 * 2;
		COUNT[i] = 1;
		STARTADR[i] = mem[j] + mem[j+1] * 256;
		if(!(STARTADR[i])){
			STOPFRG[i] = 255;
			MAXVOL[i] = 255;
			STOPPARTS++;
		}else{
			MAXVOL[i] = 0;
			STOPFRG[i] = 0;
		}
		STARTADR[i] -= 0x1000;
		OFFSET[i] = STARTADR[i];
		LENGTH[i] = 5;
		set_tone(noise, i);
	}
	LOOPTIME = 0; //no / 256;

	set_fm(0x12, 191);

	if(init_sndint())
		exit(1);
	set_fm(0x14, 0x2a);
}
//	getchar();

void stop_fmdbgm(void)
{
	stop();
	reset_sndint();

//	exit(0);
}
