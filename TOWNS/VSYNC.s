.ascii	"VSYNC"
.data
//.extern _int_vsync
//.extern _datasegment
//.extern _VECTOR_ADRV
//.extern _VECTOR_SEGV
//.extern _VECTOR_REAV
.extern _vsync_flag
//.extern _saveIMR_S

.align 2
_datasegment:
	.long	0
_VECTOR_ADRV:
	.long	0
_VECTOR_SEGV:
	.long	0
_VECTOR_REAV:
	.long	0
//_vsync_flag:
//	.long	0
_saveIMR_S:
	.long	0

.text
.global _int_vsync_ent
.global _init_vsync_ent
.global _reset_vsync_ent

_int_vsync_ent:
	cli

	push	%ds
	push	%eax
	push	%ebx

//	mov	$0x40,%al
//	out %al,$0x22

	mov		$0x05ca,%dx
	outb	%al,%dx		// VSYNC FLAG Clear

	movw	$0x14,%ax
	movw	%ax,%ds

	movl	$1,_vsync_flag

	movb	$0x20,%al
	outb	%al,$0x0010	;/* EOI(Slave) */

	outb	%al,$0x06c // 1�ʕb�E�F�C�g
	cmc
	cmc

	movb	$0x20,%al
	outb	%al,$0	;/* EOI(Master) */

	pop	%ebx
	pop	%eax
	pop	%ds
	iretl

///////////////////////////////////////////////////////////////////////////////

_init_vsync_ent:
	push	%es
	pushal
	pushfl
/*	popfl
	popal
	pop	%es
	ret
*/
//	jmp	__end

	cli

	mov	%ds,%ax
	mov	%ax,_datasegment

	mov	$0x04b,%cl
	mov	$0x2502,%ax
	int	$0x21
	movw	%es,%ax
	movl	%ebx,_VECTOR_ADRV
	movw	%ax,_VECTOR_SEGV

	movb	$0x04b,%cl
	movw	$0x2503,%ax
	int	$0x21
	movl	%ebx,_VECTOR_REAV

	push	%ds

	mov	%cs,%ax
	mov %ax,%ds

	mov	$0x02506,%ax
//	mov	$0x02504,%ax
	mov	$0x4b,%cl
	leal	_int_vsync_ent,%edx
	int	$0x21

	pop	%ds

//	jmp	__noerror1
	jnb	__noerror1

//	outp(0x22,0x40);
	mov	$0x40,%al
	out %al,$0x22
	hlt

__noerror1:
	out	%al,$0x06c // 1�ʕb�E�F�C�g
	cmc
	cmc

	inb	$0x0010+2,%al

//	mov	_saveIMR_S,%dx
//	mov	%al,%dx
	movb	%al,_saveIMR_S

	out	%al,$0x06c // 1�ʕb�E�F�C�g
	cmc
	cmc

//	mov	00100000b,%dl
//	mov	00001000b,%dl
	movb	$0x08,%dl
	xorb	$0xff,%dl
	andb	%dl,%al
	outb	%al,$0x0010+2	// ���荞�݋���
	sti

__end:
	popfl
	popal
	pop	%es
	xor	%eax,%eax

	ret

///////////////////////////////////////////////////////////////////////////////

_reset_vsync_ent:
	cli

	push	%es
	pushal
	pushfl

//	jmp	_resetskip

	out	%al,$0x06c	// 1�ʕb�E�F�C�g
	cmc
	cmc

//
	inb	$0x0010+2,%al

	out	%al,$0x06c	// 1�ʕb�E�F�C�g
	cmc
	cmc

//	orb	00001000b,%al
	orb	$0x80,%al
//
	movb	_saveIMR_S,%al
	outb	%al,$0x0010+2	// ���荞�ݕ��A


	out	%al,$0x06c	// 1�ʕb�E�F�C�g
	cmc
	cmc

//_resetskip:
	push	%ds
//	mov	%cs,%ax
//	mov %ax,%ds

	mov	$0x4b,%cl
	movl	_VECTOR_ADRV,%edx
	movw	_VECTOR_SEGV,%ax
	movl	_VECTOR_REAV,%ebx

	movw	%ax,%ds
	movw	$0x2507,%ax
	int	$0x21

	pop	%ds
	jnb	__noerror

//	outp(0x22,0x40);
	mov	$0x40,%al
	out %al,$0x22

	hlt

//	mov	al,00100101b
//	out	$0010h,al	/* EOI */

__noerror:
	popfl
	popal
	pop	%es
	xor	%eax,%eax

	sti

	ret
