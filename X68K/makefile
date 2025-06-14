# SP_RUN.X makefile for X680x0 GCC (elf2x68k) with XSP

#É}ÉNÉç
CC = m68k-xelf-gcc

#CFLAGS =  -O -Wall -g
CFLAGS = -m68000 -O2 -finput-charset=utf-8 -fexec-charset=cp932

#LINKOPTS = -liocs -ldos
# -lsignal
LINKOPTS = -Ttext=0x6800 -s
# -specs=x68knodos.specs

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c $<

#ê∂ê¨éËèá

all:RAIN.X

OBJFILES = SP_INIT.o SUBFUNC.o SP_MAIN.o SP_LD.o PLAY.o FONT68.o FMD68.o

#XSPLIBFILES = xsp2lib.o pcm8afnc.o

RAIN.X : $(OBJFILES)
	$(CC) $(LINKOPTS) -o RAIN.X $(OBJFILES) $(XSPLIBFILES)
# $(CFLAGS) $(LINKOPTS)

SP_INIT.o : SP_INIT.c SP.h SP_LD.h PLAY.h common.h FMD68.h

SP_LD.o : SP_LD.c SP_LD.h sp.h common.h

PLAY.o : PLAY.c PLAY.h

SUBFUNC.o : SUBFUNC.c SP.h common.h

SP_MAIN.o : SP_MAIN.c SP.h SPR_DEF.h SP_COM.h FONT68.h common.h SP_ENE.h SP_SHT.h

FONT68.o : FONT68.c FONT68.h

FMD68,o : FMD68.c FMD68.h key.h keyx68.h tone.h
