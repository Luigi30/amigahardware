IDIR=../include

CC=vc -DAMIGA -c99
AS=vc -DAMIGA -c99
CFLAGS=-g -I$(IDIR)
LDFLAGS=
ASFLAGS=-c -g -I$(IDIR)

OBJDIR=obj
LDLIBS=-lamiga -lauto -lvc
OUTDIR=../bin

OBJECTS = blitter.o packbits.o sprite_ship.o interleaved_tiles.o video.o blit.o globals.o \
			game/game.o font.o sprites/sprites.o image/loader.o input.o serial.o amiga.o blit.o iff.o \
			main.o
			
$(OBJDIR)/%.o: %.c
	$(CC) $@ $< $(CFLAGS)
	
$(OBJDIR)/%.o: %.s
	$(CC) $< $(ASFLAGS)
			
hello : $(OBJECTS)
	vlink -o $(OUTDIR)/$@ vlibos3:startup.o $^ -Cvbcc -bamigahunk -Bstatic $(LDFLAGS) $(LDLIBS)
	
clean:
	rm -f $(ODIR)/*.o