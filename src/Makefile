OS=$(shell uname)

IDIR=../include

CC=vc -DAMIGA -c99
AS=vc -DAMIGA -c99
CFLAGS=-I$(IDIR)
LDFLAGS=
ASFLAGS=-c -I$(IDIR)

OBJDIR=../obj
LDLIBS=-lamiga -lauto -lvc
OUTDIR=../bin

ifeq ($(OS),MINGW32_NT-6.2)
$(info * Build environment is MinGW 6.2)
LIBDIR := C:/amiga-dev/targets/m68k-amigaos/lib/
else ifeq ($(OS),AmigaOS)
$(info * Build environment is AmigaOS)
LIBDIR := vlibos3:
BINDIR := GG:/bin
else
	$(error Unknown environment. If Windows, check that MinGW 6.2 is installed. If Amiga, check that the GG: assign exists.)
endif

C_SRC	=	globals.c serial.c amiga.c \
			game/game.c game/input.c \
			gfx/blit.c gfx/bob.c gfx/font.c \
			image/iff.c image/loader.c \
			sprites/sprites.c \
			main.c
		
ASM_SRC	= 	gfx/blitter.s gfx/palette.s gfx/video.s \
			image/packbits.s \
			sprites/hwsprites/explode0.s sprites/hwsprites/explode1.s

OBJECTS = $(patsubst %.c, $(OBJDIR)/%.o, $(C_SRC)) $(patsubst %.s, $(OBJDIR)/%.o, $(ASM_SRC))
			
$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $< $(CFLAGS) -o $@
	
$(OBJDIR)/%.o: %.s
	@mkdir -p $(@D)
	$(CC) -c $< $(ASFLAGS) -o $@
	
all : raid resource
	
raid : $(OBJECTS)
	vlink -o $(OUTDIR)/$@ $(LIBDIR)startup.o $^ -Cvbcc -bamigahunk -Bstatic -L$(LIBDIR) $(LDFLAGS) $(LDLIBS)
	
resource :
	cp -r ../resource/. ../bin/
		