/* ���ʃ}�N�� for MSX2 */

#ifndef SP_H_INCLUDE
#define SP_H_INCLUDE

#define ABS(X) ((X < 0) ? -X : X)

//#define NOERROR		0			/* �����܂�̂��� *
#define ERROR		-1
enum { NOERROR = 0, SYSERR, SYSEXIT, ERRLV1, ERRLV2, ERRLV3};

#define SCREEN_MAX_X 256

#define SCREEN_MAX_Y 212
#define CHR_TOP 0

#define SCREEN_MIN_X 0
#define SCREEN_MIN_Y 0
#define WAIT1S 60

#define MAX_SPRITE 32			/* �ő勖�e�X�v���C�g�\���� */

#define BACKCOLOR	0x00		/* �w�i��h��Ԃ��F */

#define SPBACKCOLOR	0x00	/* �X�v���C�g�̖��\��������h��Ԃ��F */

//#define TEKI_NUM_MAX 8

#define STAR_NUM	16						/* �X�^�|�̐� */

#define SHIFT_NUM_Y	3
#define SHIFT_NUM	3		/* ���W�n���V�t�g�����(�Œ菬���_���Z�p) */
//#define SHIFT_NUM	1

#define SCRL_SFT 4			/* �X�N���|���̃e�X�g�p */
#define SCRL_MIN 16
#define SCRL_MAX (SCRL_MIN*SCRL_MIN) / 2

/* �֐������C�����ł̂�extern�����ɂ���}�N�� */
#ifndef MAIN
#define EXTERN extern
#else
#define EXTERN
#endif

/* ���ʕϐ� */

typedef struct chr_para3{
	unsigned char y, x;
//	short x;
	unsigned char pat_num,atr;
//	unsigned char pal;
} CHR_PARA3;

typedef struct chr_para4{
	unsigned char pat_num,atr;
//	unsigned char pal;
} CHR_PARA4;

EXTERN CHR_PARA3 chr_data[MAX_SPRITE * 2];
//EXTERN CHR_PARA3 chr_data2[MAX_SPRITE];
EXTERN CHR_PARA4 old_data[2][MAX_SPRITE];

//EXTERN unsigned char spr_num[2];						/* �X�v���C�g�ő�\���� */

EXTERN short scrl, scrl_spd;				/* �X�N���|���Ǘ��p */
//EXTERN unsigned char star[5][STAR_NUM];		/* �X�^�|�Ǘ��p */


/**********************************************************************/

#define CONST
// const

enum {
	SPR_OFS_X = -16, //16,
	SPR_OFS_Y = -16, //-16,
};


/* �^ */
typedef struct {
	unsigned char patno;
	signed char x;
	signed char y;
	unsigned char atr;
} SPR_COMB;

typedef struct {
	CONST unsigned char patmax;
	CONST SPR_COMB *data;
} SPR_INFO;

/* �}�N�� */
//#define SYS_WAIT sys_wait

#define SPR_SET(NO, X, Y) {\
	spr[NO].x = X;\
	spr[NO].y = Y;\
}

EXTERN CHR_PARA3 *pchr_data;

/* �X�v���C�g�ʒu���`����}�N�� */

/*
#define DEF_SP_SINGLE_MACRO(X, Y, PAT, PAL) {\
	pchr_data = &chr_data[tmp_spr_count++];\
	pchr_data->pat_num = PAT; \
		pchr_data->atr = 0x00 | PAL;\
}*/
/*
#define DEF_SP_SINGLE_MACRO(X, Y, PAT, PAL) {\
	chr_data[tmp_spr_count].x = ((X >> SHIFT_NUM) + SPR_OFS_X); \
	chr_data[tmp_spr_count].y = ((Y >> SHIFT_NUM_Y) + SPR_OFS_Y - 1); \
	chr_data[tmp_spr_count].pat_num = PAT; \
		chr_data[tmp_spr_count++].atr = 0x00 | PAL;\
}
*/

#define DEF_SP_SINGLE_MACRO(X, Y, PAT, PAL) {\
	pchr_data = &chr_data[tmp_spr_count++];\
	pchr_data->x = (((X) >> SHIFT_NUM) + SPR_OFS_X); \
	pchr_data->y = (((Y) >> SHIFT_NUM_Y) + SPR_OFS_Y - 1); \
	pchr_data->pat_num = (PAT); \
	if((pchr_data->x >= 256) || (pchr_data->x <= -16)){\
		pchr_data->y = 212;\
	}\
	else if(pchr_data->x >= 0){\
		pchr_data->atr = 0x00 | PAL;\
	}\
	else {\
		pchr_data->x += 32;\
		pchr_data->atr = 0x80 | PAL;\
	}\
}

/* �}�N���� �����X�v���C�g���� */
/*
#define DEF_SP_MACRO(X, Y, pat, pal_no) {\
	chrnum = spr_info[pat].patmax;\
	sprcomb = spr_info[pat].data;\
	tmp_xx = X;\
	tmp_yy = Y;\
	while(chrnum--){\
		DEF_SP_SINGLE_MACRO(tmp_xx + (sprcomb->x << SHIFT_NUM),\
					 tmp_yy + (sprcomb->y << SHIFT_NUM_Y),\
					sprcomb->patno, pal_no\
					);\
		sprcomb++;\
	}\
}
*/
/*#define DEF_SP_DOUBLE(X, Y, PAT, PAL) {\
	DEF_SP_SINGLE(X, Y, PAT * 8, PAL);\
	DEF_SP_SINGLE(X, Y, PAT * 8 + 4, PAL);\
}*/
//	printf("<%d %x>" , pat, spr_info[pat].adr);\

#endif
