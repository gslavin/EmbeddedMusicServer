# Copyright (c) 2014 Cesanta Software
# All rights reserved

PROG = music_daemon
CFLAGS = -W -Wall -I../SoundRequestServer -pthread -g -O0 -lm $(CFLAGS_EXTRA)
SOURCES = $(PROG).c 

all: $(PROG)

run: $(PROG)
	./$(PROG)

$(PROG): $(SOURCES) Makefile
	$(CC) -o $(PROG) $(SOURCES) $(CFLAGS)

win:
	wine cl $(SOURCES) /MD /nologo /DNDEBUG /O1 /I../.. /Fe$(PROG).exe
	wine $(PROG).exe

clean:
	rm -rf $(PROG) *.exe *.dSYM *.obj *.exp .*o *.lib *.gc*
