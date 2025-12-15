# rainamg makefile for AMIGA OS GCC

LIBPATH = c:\amiga-gcc/m68k-amigaos/lib/

#É}ÉNÉç
CC = m68k-amigaos-gcc

CFLAGS = -c
LINKOPTS = -Os -noixemul

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c $<

.SUFFIXES: .o .s
.s.o:
	m68k-amigaos-as -o $*.o $*.s

#ê∂ê¨éËèá

all:rainamg

OBJFILES = rainamg.o

rainamg : $(OBJFILES)
	$(CC) $(LINKOPTS) $(OBJFILES) -o rainamg -lamiga
	adf.bat rainamg

rainamg.o : rainamg.c
