# RAIN.EXP makefile for FM TOWNS-gcc クロスコンパイル環境

#マクロ

GCCROOT=C:/tgcc
LIBRARY_PATH=$(GCCROOT)/lib
C_INCLUDE_PATH=$(GCCROOT)/include

LINK = $(GCCROOT)/bin/ld_towns -N -S -o $@ $(LIBRARY_PATH)/crt0.o
LINKOPTS = -L$(LIBRARY_PATH) -lTN -lce -lme -lpc 

# Cソ−スからobj作成
.SUFFIXES: .o .c
.c.o:
	$(GCCROOT)/bin/cpp  -D__GNUC__ -DTOWNS -I. -I $(C_INCLUDE_PATH) $*.c $*.i
	$(GCCROOT)/bin/cc1 -O -quiet -msoft-float $*.i
	$(GCCROOT)/bin/as -o $*.o $*.s

.SUFFIXES: .o .s
.s.o:
	$(GCCROOT)/bin/as -o $*.o $*.s

#生成手順

all:RAIN.EXP

OBJFILES = SP_INIT.o SUBFUNC.o SP_MAIN.o SP68_LD.o FONTTW.o FMDTWG.o VSYNC.o

RAIN.EXP : RAIN.bin
	$(GCCROOT)/bin/genexp3 RAIN.bin RAIN.EXP

RAIN.bin : $(OBJFILES)
	$(LINK) $(OBJFILES) $(LINKOPTS)

SP_INIT.o : SP_INIT.c SP.H

#SPMAKE.o : SPMAKE.c SPMAKE.H

#PLAY.o : PLAY.c PLAY.H

SUBFUNC.o : SUBFUNC.c SUBFUNC.H SP.H

SP68_LD.o : SP68_LD.c SP68_LD.H SP.H

SP_MAIN.o : SP_MAIN.c SP_MAIN.H SP.H SPR_DEF.H SP_COM.H  FONTTW.H FMDTWG.h

FONTTW.o : FONTTW.c FONTTW.H

FMDTWG,o : FMDTWG.c FMDTWG.h key.h keyxtow.h tone.h

VSYNC.o : VSYNC.s
