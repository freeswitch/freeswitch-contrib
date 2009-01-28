INCS=-I../../../../../libs/esl/src/include
LIBEDIT_DIR=../../libs/libedit
DEBUG=-g -ggdb
PICKY=-O2 -ffast-math -Wall -Werror -Wunused-variable -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes
CFLAGS=$(INCS) -DHAVE_EDITLINE $(DEBUG) -I$(LIBEDIT_DIR)/src/ $(PICKY)
MYLIB=libesl.a
LIBS=-lncurses -lpthread -lesl
LDFLAGS=-L../../../../../libs/esl/
OBJS=src/esl.o src/esl_event.o src/esl_threadmutex.o src/esl_config.o
SRC=src/esl.c src/esl_event.c src/esl_threadmutex.c src/esl_config.c
HEADERS=src/include/esl_config.h src/include/esl_event.h src/include/esl.h src/include/esl_threadmutex.h

all: tel_handler

tel_handler: tel_handler.c
	$(CC) $(CC_CFLAGS) $(CFLAGS) tel_handler.c -o tel_handler $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CC_CFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o tel_handler *~ 

install: all
	cp tel_handler /usr/bin/tel_handler
