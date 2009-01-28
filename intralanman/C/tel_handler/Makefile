CFLAGS=-I../../../../../libs/esl/src/include -g -ggdb -ffast-math -Wall -Werror -Wunused-variable -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS=-L../../../../../libs/esl/ -lpthread -lesl

all: tel_handler

tel_handler: tel_handler.c
	$(CC) $(CFLAGS) tel_handler.c -o tel_handler $(LDFLAGS) $(LIBS)

clean:
	rm -f tel_handler 

install: all
	cp tel_handler /usr/bin/tel_handler
