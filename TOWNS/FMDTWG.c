/* FM MUSIC SAMPLE for FM TOWNS BIOS�Ŋ܂� */
/* GCC */
/* �Q�l PC-9801�}�V����T�E���h�v���O���~���O�AFM TOWNS�e�N�j�J���f�[�^�u�b�N */
/* Free386�̃h�L�������g(SND.C) */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "fmdtwg.h"

#include "key.h"
#include "keytow.h"
#include "tone.h"

#define BIOS
//#define BIOSVOL

#define ERROR 1
#define NOERROR 0

#define _disable() asm("cli\n")
#define _enable() asm("sti\n")


#define PARTSUU 6

#ifdef BIOS
#define BIOSVOL
#include "snd.h"
char sndwork[SndWorkSize];
#endif

FILE *stream[2];

//extern void *ms_timer_sub_int;
//extern void *ms_timer_a_int;
//extern void *ms_timer_b_int;
//void int_fm(void);

volatile unsigned char COUNT[PARTSUU] = {1,1,1,1,1,1}; // �����J�E���^
volatile unsigned char STOPFRG[PARTSUU] = {0,0,0,0,0,0}; // WAIT&SYNC&OFF
volatile unsigned char MAXVOL[PARTSUU] = {15,15,15,15,15,15}; // �{�����[��
volatile unsigned char LENGTH[PARTSUU] = {5,5,5,5,5,5}; // ��{����

volatile unsigned short OFFSET[PARTSUU] = {0,0,0,0,0,0}; // ���t�f�[�^���s���A�h���X
volatile unsigned short STARTADR[PARTSUU] = {0,0,0,0,0,0}; // ���t�f�[�^�J�n�A�h���X
volatile unsigned char FEEDVOL = 0; // �t�F�[�h�A�E�g���x��
volatile unsigned char FCOUNT = 1; //�t�F�[�h�A�E�g�J�E���^
volatile unsigned char LOOPTIME = 1; // ���t�񐔁i�O�͖������[�v�j
volatile unsigned char STOPPARTS = 0; //
volatile unsigned char ENDFRG = 0; //
volatile unsigned char NSAVE = 0; //

unsigned char mem[65536];


void SND_int(void)
{
//	return;

	SND_fm_timer_a_set(0,0);
	SND_fm_timer_b_set(0,0);

	_disable();
	asm volatile (
		"push	%eax\n"
		"leal _int_fm,%eax\n"
		"mov %eax,(ms_timer_b_int)\n"
		"pop	%eax\n"
	);

	SND_fm_timer_b_set(1,199);

	_enable();
}

int SND_fm_read_status(void){
	return inportb(0x4d8);
}

short bload2(char *loadfil, unsigned short offset)
{
	unsigned short size;
	unsigned short address;
	unsigned char buffer[3];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
//		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
	fread( buffer, 1, 2, stream[0]);
	address = buffer[0] + buffer[1] * 256;
	fread( buffer, 1, 2, stream[0]);
	size = (unsigned short)(buffer[0] + buffer[1] * 256) - (unsigned short)address;
	address -= offset;
//	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address , size, (unsigned short)address + size);

	fread( &mem[address] , 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

/* FM���W�X�^�ݒ� ch.1-3 */
#ifdef BIOS

int bank1,regNo1,data1;

int SND_fm_write_data2(void) //int bank, int regNo, int data )
{
	asm(
	"enter	$0,$0\n"
	"pushl	%ebx\n"
	"pushl	%edx\n"

//	"movb	8(%ebp),%bh\n"
//	"movb	12(%ebp),%dh\n"
//	"movb	16(%ebp),%dl\n"

	"movb	_bank1,%bh\n"
	"movb	_regNo1,%dh\n"
	"movb	_data1,%dl\n"

	"movb	$0x11,%ah\n"
	"call	sound_bios1\n"

	"movzbl	%al,%eax\n"

	"popl	%edx\n"
	"popl	%ebx\n"
	"leave\n"
	);
}


void set_fm(char bank, char reg, char data)
{
	while(SND_fm_read_status() & 0x80);
	bank1 = bank;
	regNo1 = reg;
	data1 = data;
	SND_fm_write_data2(); //bank, reg, data);
//	SND_fm_write_save_data(bank, reg, data);
}
#else
void set_fm(unsigned char bank, unsigned char reg, unsigned char data)
{
	int port;
	if(bank == 1)
		port = 0x4dc;
	else if(bank == 0)
		port = 0x4d8;

	while(inp(0x4d8) & 0x80);
	outportb(port,reg);
//__asm
//	ld	a,(ix+0)
//__endasm;
//	while(inp(0x4d8) & 0x80);
	outportb(port+2,data);
}
#endif

/*  AR  DR  SR  RR  SL  OL  KS  ML DT1 DT2 AME */
enum { AR, DR, SR, RR, SL, OL, KS, MUL,DT1,DT2,AME };
enum { D1R = DR , TL=OL, D1L = SL, D2R=SR};
enum {CON = 4 * 11, FL};

char op[4] = {0, 2, 1, 3};

/* ���F�ݒ� */
void set_tone(unsigned char no, unsigned char ch)
{
	unsigned char i, j, k, bank = 0;
	if(ch >= 3){
		ch -= 3;
		bank = 1;
	}

	for(i = 0; i < 4; ++i){
		j = ch + op[i] * 4;
		k = i * 11;
		set_fm(bank, 0x30 + j, tone_table[no][MUL + k] | tone_table[no][DT1 + k] * 16);
		set_fm(bank, 0x40 + j, tone_table[no][TL + k]);
		set_fm(bank, 0x50 + j, tone_table[no][AR + k] | tone_table[no][KS + k] * 64);
		set_fm(bank, 0x60 + j, tone_table[no][D1R + k] | tone_table[no][AME + k] * 128);
		set_fm(bank, 0x70 + j,tone_table[no][SR + k]); // | tone_table[no][DT2 + k] * 64);
		set_fm(bank, 0x80 + j, tone_table[no][RR + k] | tone_table[no][SL + k] * 16);
//		set_fm(bank, 0x90 + j, 0x0f);
	}
//	j = 0xb0 + ch;
	set_fm(bank, 0xb0 + ch, tone_table[no][CON] | tone_table[no][FL] * 8);// | 0xc0);
//	set_fm(bank, 0xb4 + ch, 0xc0);


/*	j = 0x30 + ch;
	for(i = 0; i < 28; ++i){
		set_fm(bank, j, tone_table[no][i]);
		j += 4;
	}
	j += 0x10;
		set_fm(bank, j, tone_table[no][i]);
*/
}

/* �����ݒ� */
void set_key(unsigned char no, unsigned char ch)
{
	char i, j, bank = 0;
	if(ch >= 3){
		ch -= 3;
		bank = 1;
	}
	set_fm(bank, 0xa4 + ch, key_table[no][0]);
	set_fm(bank, 0xa0 + ch, key_table[no][1]);
}


char key[6] = {0, 1, 2, 4, 5, 6};

void stop(void)
{
	unsigned char i;
	SND_fm_timer_b_set(0,0);
	for(i = 0; i < PARTSUU; ++i){
		set_fm(0, 0x28, 0x00 | key[i]);	/* off */
	}
//	SND_end();
}

unsigned char count = 0;

void int_fm(void)
{
	unsigned char i, j, no, ch;
	unsigned char data;

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
						set_fm(0, ch, no);
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
						set_fm(0, 0x28, 0x00 | key[i]);	/* off */
						if((data & 0x7f) != 0){
							set_key((data & 0x7f) - 1, i);	/* key */
							set_fm(0, 0x28, 0xf0 | key[i]);	/* on */
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
			return;			/* �������[�v */
		if(--LOOPTIME){
			goto playloop;	/* ���[�v�񐔂�0�ȊO�Ȃ烋�[�v */
		}
		/* ���t���Ȓ�~ */
		stop();
	}
	ENDFRG = 0;
}

/*
int	main(int argc,char **argv)
{
	unsigned short no = 0;
	unsigned char i, ch = 0;
	unsigned char noise = 0;

//	SND_init(sndwork);

	if (argc < 2){
		return ERROR;
	}

	if (argc >= 3){
		no = atoi(argv[2]);
		if((no % 256) > 9)
			no = 0;
	}

	if (argc >= 4){
		noise = atoi(argv[3]);
	}

//	printf("Hello,world.\n");

//	getchar();
}
*/

int load_fmdbgm(char *filename)
{
//	if(bload2(argv[1], 0x1000*0) == ERROR)
	if(bload2(filename, 0x1000*0) == ERROR)
		return ERROR;
//		exit(1);
	return NOERROR;
}

void play_fmdbgm(void)
{
	unsigned short no = 0;
	unsigned char i, ch = 0;
	unsigned char noise = 0;

#ifdef BIOSVOL
//	printf("BIOS��������");
//	SND_init(sndwork);
//	printf("BIOS�������I��");

#endif
	FEEDVOL = 0;
	FCOUNT = 1;
	STOPPARTS = 0;
	ENDFRG = 0;
	NSAVE = 0;

	for(i = 0; i < PARTSUU; ++i){
		int j = 0xdb00-0x1100 + i * 2 + (no % 256) * 12 * 2;
		COUNT[i] = 1;
		STARTADR[i] = mem[j] + mem[j+1] * 256;
		if(!STARTADR[i]){
			STOPFRG[i] = 255;
			MAXVOL[i] = 255;
			STOPPARTS++;
		}else{
			MAXVOL[i] = 0;
			STOPFRG[i] = 0;
		}
		STARTADR[i] -= 0x1000;
		OFFSET[i] = STARTADR[i];
		set_tone(noise, i);
	}
	LOOPTIME = 0; //no / 256;

	/* �~���[�g���� */
#ifdef BIOSVOL
//	SND_elevol_all_mute(-1);
//	SND_elevol_mute(0x33);
#else
//	outp(0x4e0,0x3f);
//	outp(0x4e1, (inp(0x4e1) | 0x0c) & 0x8f);
//	outp(0x4e2,0x3f);
//	outp(0x4e3, (inp(0x4e3) | 0x0c) & 0x8f);
	outp(0x4ec, 0x40); //(inp(0x4ec) | 0x40) & 0xc0);
	outp(0x4d5, 0x02);
//	outp(0x60,0x4);
#endif

	SND_int();
//	getchar();
}

void stop_fmdbgm(void)
{
	stop();
//	return 0;
}
