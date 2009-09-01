CC=gcc
ESL_PATH=../../../libs/esl
INCLUDES=-I$(ESL_PATH)/../../libs/apr/include -I$(ESL_PATH)/src/include
CFLAGS=-ggdb3 $(INCLUDES) -Wall -Werror
LIBS=-L$(ESL_PATH) -L$(ESL_PATH)/../../.libs -L$(ESL_PATH)/../../libs/apr/.libs -lpthread -lesl -lapr-1

all: esl2agi

esl2agi: esl2agi.c
	$(CC) -o esl2agi esl2agi.c $(CFLAGS) $(LIBS)

clean:
	rm -rf esl2agi esl2agi.dSYM
