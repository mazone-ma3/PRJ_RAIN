	.section .text
	.global r_start
	.global HSYNC_handler
	.global RASTER_handler
	.global VSYNC_handler
	.extern sin_table


	/* HSYNC���荞�݃n���h�� */
HSYNC_handler:
#	move.w	raster_line,0xe80018 /* �����I�t�Z�b�g�������� */
	rte

RASTER_handler:

#	movem.l %d0-%d1/%a0, -(%sp)	/* ���W�X�^�ޔ� */
	movem.l %d1/%a0,-(%sp)		/* ���W�X�^�ޔ� */

	move.w line_counter,%d1


	move.l	(psintable),%a0
#	move.w  (%a0, %d1.w), 0xe80018	/* sin�e�[�u������I�t�Z�b�g�擾(GRP0) */
	move.w  (%a0, %d1.w), 0xe8001c	/* sin�e�[�u������I�t�Z�b�g�擾(GRP1) */

	move.w	%d1,0xe80012		/* ���̃��X�^ */
	addq.w	#4,%d1				/* ���C���J�E���^�i�߂� */
	and.w	#0x1ff,%d1			/* 512�܂� */
	move.w  %d1,line_counter

	movem.l (%sp)+,%d1/%a0		/* ���W�X�^���A */
#	movem.l (%sp)+, %d0-%d1/%a0	/* ���W�X�^���A */
	rte

	/* VSYNC���荞�݃n���h�� */
VSYNC_handler:
#	rts

	/* ������ */
r_start:
	movem.l %d0/%a0, -(%sp)
	lea	 sin_table, %a0
	move.l  table_offset, %d0
	addq.l  #2, %d0			/* �e�[�u����2�o�C�g�i�߂�i�A�j���[�V�����j */
	and.l	#0x1ff,%d0		/* 512�܂� */
	move.l  %d0, table_offset

	add.l	%d0,%a0
	move.l	%a0,psintable
	movem.l (%sp)+, %d0/%a0
	rts
#	rte

	/* �f�[�^�Z�N�V���� */
	.section .data
line_counter:
	.word 0					/* ���݂̃��C�� */
table_offset:
	.long 0					/* sin�e�[�u���̃I�t�Z�b�g */
psintable:
	.long 0					/* sin�e�[�u���ւ̃|�C���^ */
#raster_line:
#	.word 0
