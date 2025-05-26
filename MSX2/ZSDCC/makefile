# RAIN.COM makefile for MSX z88dk

#マクロ
CC= zcc

CFLAGS = +msx -compiler sdcc --opt-code-speed
# --opt-code-size
# --sdcccall0
# -DNODLAY -lm

#LINKOPTS = +msx -create-app -subtype=msxdos
LINKOPTS = +cpm -create-app
LINKOPTS2 = +msx -create-app -subtype=disk -zorg=0x83c0

# Cソ－スからobj作成

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) $< -c

#生成手順

all:RAIN.COM

OBJFILES = RAIN.o

RAIN.COM : $(OBJFILES)
	$(CC) $(LINKOPTS) $(OBJFILES) -o RAIN.COM
	$(CC) $(LINKOPTS2) $(OBJFILES) -o RAIN.BIN

RAIN.o : RAIN.c sp.h spr_def.h sp_com.h inkey.h list.h spr_col.h sp_ene.h RAMAIN.h sp_sht.h
