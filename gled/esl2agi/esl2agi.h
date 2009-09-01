/*
 * Copyright (c) 2009, Tristan Mahe for TELEMAQUE
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* 
 * Define Section
 */

#define _GNU_SOURCE

/* 
 * Inc Section
 */

#include <stdio.h>
#include <stdlib.h>
#include <esl.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
/* 
 * Macros Section
 */

#define PIPES_TO_VOID(v)	((void*)(v))
#define VOID_TO_PIPES(p)	((esl2agi_pipes_t*)(p))

#define ESL_A_TO_VOID(v)	((void*)(v))
#define VOID_TO_ESL_A(p)	((esl_accept_t*)(p))

#define MAX(x,y)		((x) > (y) ? (x) : (y))

/* 
 * Struct Section
 */

typedef struct {
	esl_socket_t client_sock;
	struct sockaddr_in *addr;
} esl_accept_t;

typedef struct {
	int script[2];
	int socket[2];
} esl2agi_pipes_t;

/* 
 * Func Section
 */

static void esl2agi(esl_socket_t server_sock, esl_socket_t client_sock, struct sockaddr_in *addr);

static void *esl2agi_thread(void *data);

/*
 * - EXEC application OPTIONS
 * - GET DATA filetoplay timeout maxdigits
 * - GET VARIABLE variablename
 * - HANGUP 'channelname'
 * - RECORD FILE filename format escapedigits timeout offsetsamples BEEP s=silence
 * - SAY DIGITS
 * - set autohangup time
 * - set music  on|off class
 * - set variable
 * - stream file filename <escape digits> [sample offset]
 * - wait for digit timeout
 */


static int handle_setup_env(int fd,esl_handle_t *eslC);

static int handle_hangup(esl_handle_t *eslC,int fd);

static int handle_answer(esl_handle_t *eslC,int fd);
