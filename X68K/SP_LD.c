/* �ȈՃX�v���C�g���|�_ with XSP */
/* SP�`�������|�h����B */

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
//#include <sys\dos.h>
//#include <conio.h>

#ifdef XSP
#include "XSP2lib.H"
//#include "PCM8Afnc.H"
#endif

#include "sp.h"
#include "sp_ld.h"

#define FILE_MAX 1 /* 10 */

FILE *sp_stream[FILE_MAX];

#ifdef XSP
/* �X�v���C�g PCG �p�^�[���ő�g�p�� */
#define	PCG_MAX		256
/*
	XSP �p PCG �z�u�Ǘ��e�[�u��
	�X�v���C�g PCG �p�^�[���ő�g�p�� + 1 �o�C�g�̃T�C�Y���K�v�B
*/
char pcg_alt[PCG_MAX + 1];
/* PCG �f�[�^�t�@�C���ǂݍ��݃o�b�t�@ */
char pcg_dat[PCG_MAX * 128];
#endif

int sp68_load(char *fil, short offset, short sprparts)
{
	long i, j, k;
	unsigned short data;
	unsigned char pattern[128];

#ifndef XSP
	unsigned short *spram;

	spram  = (unsigned short *)0xeb8000;
	spram += ((64 * offset));
#else
	short *ppcg_data = (short *)pcg_dat;

	xsp_pcgmask_on(0, 64-1); //short start_no, short end_no);
#endif
	if ((sp_stream[0] = fopen( fil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", fil);

		fclose(sp_stream[0]);
		return 1;
	}
/*	printf("Loading file:%s\n" ,fil);*/
/* 512 = 128 * 4 */
	for (i = 0; i < sprparts; i++){
		fread(pattern, 1, 128, sp_stream[0]);

		for (j = 0; j < 64; j++){
			data = pattern[j * 2] * 256 + pattern[j * 2 + 1];
#ifndef XSP
			*(spram++) = data;
#else
			*ppcg_data++ = data;
#endif
		}
	}
	fclose(sp_stream[0]);
#ifdef XSP
	/* PCG �f�[�^�� PCG �z�u�Ǘ����e�[�u�����w�� */
	xsp_pcgdat_set(pcg_dat, pcg_alt, sizeof(pcg_alt));
#endif
	return 0;
}

int pal68_load(char *fil)
{
	long i, j, k;
	unsigned short data;
	unsigned char pattern[2];

	unsigned short *palram_char;
	palram_char  = (unsigned short *)0xe82200;

	if ((sp_stream[0] = fopen( fil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", fil);

		fclose(sp_stream[0]);
		return 1;
	}
/*	printf("Loading file:%s\n" ,fil);*/

	for (i = 0; i < 240; i++){
		k = fread(pattern, 1, 2, sp_stream[0]);
		if(k < 1)
			break;
		data = pattern[0] * 256 + pattern[1];
		*(palram_char++) = data;
	}
	fclose(sp_stream[0]);

	return 0;
}


#define MSXWIDTH 256
#define MSXLINE 212
#define PCGSIZEX 4
#define PCGSIZEY 8
//#define PCGMSXPARTS 256
#define MAXSPRITE 128

int msxspconv(char *loadfil, short offset, short sprparts)
{
	FILE *stream[2];
	unsigned char pattern[10];
	unsigned char msxcolor[MSXWIDTH / 2][MSXLINE];

	long i, j,k,y, x, xx, yy, no, max_xx;

	unsigned short data;

//#ifndef XSP
	unsigned short *spram;
	spram  = (unsigned short *)0xeb8000;
	spram += ((64 * offset));
//#endif
#ifdef XSP
	short *ppcg_data = (short *)pcg_dat;
	xsp_pcgmask_on(0, 128-1); //short start_no, short end_no);
#endif


	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}

	fread(pattern, 1, 1, stream[0]);	/* MSX�擪��ǂݎ̂Ă� */
	fread(pattern, 1, 4, stream[0]);	/* MSX�w�b�_���ǂݎ̂Ă� */
	fread(pattern, 1, 2, stream[0]);	/* MSX�w�b�_��ǂݎ̂Ă� */

	for(y = 0; y < MSXLINE; ++y){
		for(x = 0; x < MSXWIDTH / 2 ; ++x){
			msxcolor[x][y] = 0;
		}
	}
	for(y = 0; y < MSXLINE; ++y){
		for(x = 0; x < MSXWIDTH / 8; ++x){
			i = fread(pattern, 1, 4, stream[0]);	/* 8dot�� */
			if(i < 1)
				break;

			/* �F���� */
			msxcolor[0 + x * 4][y] = pattern[0]; 
			msxcolor[1 + x * 4][y] = pattern[1]; 
			msxcolor[2 + x * 4][y] = pattern[2];
			msxcolor[3 + x * 4][y] = pattern[3];
		}
	}
	fclose(stream[0]);
	max_xx = 128;


	j = 0;
	xx=0;
	yy=0;
	x=0;
	for(no = 0; no < sprparts; ++no){
		for(i = 0; i < 2; ++i){
			for(j = 0; j < 2; ++j){
//				printf("\nno =%d ",no);
				for(y = 0; y < PCGSIZEY; ++y){
					for(x = 0; x < PCGSIZEX; x+=2){

						if((x+xx) >= max_xx) {
							xx=0;
							yy+=PCGSIZEY*2;
						}

						data = msxcolor[x + xx][y + yy] * 256 + msxcolor[x + xx + 1][y + yy];
//#ifndef XSP
						*(spram++) = data;
#ifdef XSP
						*ppcg_data++ = data;
#endif
					}
				}
				yy+=PCGSIZEY;
			}
			yy-=PCGSIZEY*2;
			xx+=PCGSIZEX;
		}
	}
#ifdef XSP
	/* PCG �f�[�^�� PCG �z�u�Ǘ����e�[�u�����w�� */
	xsp_pcgdat_set(pcg_dat, pcg_alt, sizeof(pcg_alt));
#endif

	return 0;
}