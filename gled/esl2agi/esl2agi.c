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
 * 
 * Contrib:
 *  main: Anthony Minessale II
 *
 */

#include "esl2agi.h"

/* 
 * Translator thread entry point
 */
static void *esl2agi_thread(void *data) {
        esl_accept_t esl_req;
	esl_accept_t *tmp;
	esl2agi_pipes_t pipes;
        esl_handle_t eslC = {{0}};

        char script_path[1024] = "\0";
        const char *buf;
	int pid;
	int thread_running;

	tmp = VOID_TO_ESL_A(data);
	esl_req.client_sock = tmp->client_sock;
	esl_req.addr = tmp->addr;
	free(tmp);

        esl_attach_handle(&eslC, esl_req.client_sock, esl_req.addr);
        if (!(buf = esl_event_get_header(eslC.info_event, "variable_ivr_path"))) {
                esl_disconnect(&eslC);
                perror("No ivr_path variable set, check your dialplan!");
                return NULL;
        }

        strncpy(script_path, buf, sizeof(script_path) - 1);

	if ( pipe(pipes.script) < 0) {
                esl_disconnect(&eslC);
                perror("script pipes failed...");
                return NULL;
	}
	if ( pipe(pipes.socket) < 0 ) {
		close(pipes.script[0]);
		close(pipes.script[1]);
                esl_disconnect(&eslC);
                perror("socket pipes failed...");
                return NULL;
	}

	thread_running = 1;
	pid = fork();
	if (pid < 0) {
		close(pipes.script[0]);
		close(pipes.script[1]);
		close(pipes.socket[0]);
		close(pipes.socket[1]);
                esl_disconnect(&eslC);
                perror("Fork failed, aie aie aie!");
		return NULL;
	}
	else if (pid == 0) {
		/* Child */
		close(pipes.socket[1]);
		close(pipes.script[0]);
		close(esl_req.client_sock);

	        dup2(pipes.socket[0], STDIN_FILENO);
        	dup2(pipes.script[1], STDOUT_FILENO);

	        execl(script_path, script_path, NULL);
		thread_running=0;
		perror("Error running script");
		close(pipes.socket[0]);
		close(pipes.script[1]);
		exit(0);
	}
	else if (pid > 0 ) { 
		esl_event_t *event;
		esl_status_t status;
		char sbuf[2048];

		close(pipes.socket[0]);
		close(pipes.script[1]);

		signal(SIGCHLD, SIG_IGN);
		signal(SIGPIPE, SIG_IGN);

		handle_setup_env(pipes.socket[1],&eslC);

		/* TODO: check status */
		status = esl_send(&eslC,"myevents");

		while ( eslC.connected && thread_running) {
			int r , nfds = 0;
			fd_set rd;

			FD_ZERO(&rd);
			FD_SET(esl_req.client_sock, &rd);
			nfds = MAX(nfds,esl_req.client_sock);
			FD_SET(pipes.script[0], &rd);
			nfds = MAX(nfds,pipes.script[0]);

			r = select(nfds + 1, &rd, NULL, NULL, NULL);
			if (r == -1 )
				break;
			if (FD_ISSET(esl_req.client_sock, &rd)) { /* ESL SIDE have something */
				status = esl_recv_event(&eslC, 0, &event);
				if (status == ESL_FAIL)
					goto end;
		                else if (status == ESL_SUCCESS) {
					/* Translation ESL TO AGI
					 * - DTMF events.
					 * - what else ?
					 */
				}
			}
			if (FD_ISSET(pipes.script[0],&rd) ) { /* AGI has something to say */
				bzero(&sbuf,2047);
				r = read(pipes.script[0],&sbuf,2047);
				if (r<0)
					goto end;
				else {
					/*TODO: correct ugly hack to remove final \n */
					sbuf[r-1] = '\0';
					if ( find_and_exec_command(&eslC,pipes.socket[1],(char *) &sbuf) < 0 )
						goto end;
				}
			}
		}
end:
		kill(pid,SIGHUP);
		close(pipes.socket[1]);
		close(pipes.script[0]);
		if (eslC.connected)
			esl_execute(&eslC,"hangup",NULL,NULL);
	        esl_disconnect(&eslC);
	}
        return NULL;
}

/*
 * Callback on accept, launch a detached thread then return.
 */
static void esl2agi(esl_socket_t server_sock, esl_socket_t client_sock, struct sockaddr_in *addr) {
	esl_accept_t *esl_req;

	/* Will be freed in the thread after read */
	esl_req = malloc(sizeof(esl_accept_t) + 1);
	esl_req->client_sock = client_sock;
	esl_req->addr = addr;

	pthread_t translator;

	pthread_create(&translator,NULL,esl2agi_thread,ESL_A_TO_VOID(esl_req));
	pthread_detach(translator);

	return;
}

/* Main entry
 * Esl listen and that's all.
 * Copyright (c) 2009, Anthony Minessale II
 * TODO:	- add mod_dialplan_asterisk lookup and load if not present.
 */
int main(int argc, char *argv[])
{
	int i;
	char *ip = NULL;
	int port = 0;
	
	for (i = 1; i + 1 < argc; ) {
		if (!strcasecmp(argv[i], "-h")) {
			ip = argv[++i];
		} else if (!strcasecmp(argv[i], "-p")) {
			port = atoi(argv[++i]);
		} else {
			i++;
		}
	}

	if (!(ip && port)) {
		fprintf(stderr, "Usage %s -h <host> -p <port>\n", argv[0]);
		return -1;
	}

	signal(SIGCHLD, SIG_IGN);

	esl_listen(ip, port, esl2agi);
	perror("We should never come here");	
	return 0;
}

/* Setup env 
 * TODO:	- correct parameters
 *		- add a generic routine instead of blindly repeating code
 */
static int handle_setup_env(int fd,esl_handle_t *eslC) {
	char buf[1024];
	char *sbuf;

	/* Setup Env for AGI scripts */
	if ( (sbuf = esl_event_get_header(eslC->info_event, "variable_ivr_path")) ) {
		sprintf(buf,"agi_request: %s\n",sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else
		return -1;

	if ( (sbuf = esl_event_get_header(eslC->info_event, "variable_channel_name")) ) {
		sprintf(buf,"agi_channel: %s\n",sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else
		return -1;

	if ( write(fd, "agi_language: en\n",18) < 0 )
		return -1;

	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-Source")) ) {
		sprintf(buf, "agi_type: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else
		return -1;

	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-Unique-ID")) ) {
		sprintf(buf, "agi_uniqueid: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else
		return -1;

	/* ANI/DNIS */
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-Caller-ID-Number")) ) {
		sprintf(buf, "agi_callerid: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_callerid: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-Caller-ID-Name")) ) {
		sprintf(buf, "agi_calleridname: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_calleridname: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}

	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-Screen-Bit")) ) {
		sprintf(buf, "agi_callingpres: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_callingpres: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	/* TODO */
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Caller-Caller-ID-Number")) ) {
		sprintf(buf, "agi_callingani2: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_callingani2: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	/* TODO */
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Caller-Caller-ID-Number")) ) {
		sprintf(buf, "agi_callington: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_callington: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Caller-Destination-Number")) ) {
		sprintf(buf, "agi_dnid: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_dnid: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	/* TODO */
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Caller-Destination-Number")) ) {
		sprintf(buf, "agi_callingtns: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_callingtns: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}

	/* TODO: correct info */
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Caller-Destination-Number")) ) {
		sprintf(buf, "agi_rdnis: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_rdnis: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-Context")) ) {
		sprintf(buf, "agi_context: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_context: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}

	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-Destination-Number")) ) {
		sprintf(buf, "agi_extension: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_extension: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}

	/* TODO: correct info */
	if ( (sbuf = esl_event_get_header(eslC->info_event, "Channel-State-Number")) ) {
		sprintf(buf, "agi_priority: %s\n", sbuf);
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}
	else {
		sprintf(buf, "agi_priority: %s\n", "unknown");
		if ( write(fd, buf,strlen(buf) ) < 0)
			return -1;
	}

	if ( write(fd, "agi_enhanced: 0.0\n", 19) < 0 )
		return -1;
	if ( write(fd, "agi_accountcode: \n\n", 21) < 0 )
		return -1;
	return 0;
}

/*
 * Hangup AGI cmd
 * TODO:	- Add arg support to know which channel we want to hangup.
 */
static int handle_hangup(esl_handle_t *eslC,int fd,int *argc, char *argv[]) {
	esl_status_t status;

	status = esl_execute(eslC,"hangup",NULL,NULL);
	if (status == ESL_FAIL) {
#ifdef _DEBUG_STDERR
		perror("ESL FAILED TO HANGUP");
#endif
		if ( write(fd,"200 result=failure\n\n",128) < 0 )
			return -1;
	}
	else if (status == ESL_SUCCESS) {
		if ( write(fd,"200 result=success\n\n",128) < 0 )
			return -1;
	}
	fprintf(stderr,"Call hungup\n");
	return 0;
}

/*
 * Answer AGI cmd
 */
static int handle_answer(esl_handle_t *eslC,int fd,int *argc, char *argv[]) {
	esl_status_t status;

	status = esl_execute(eslC,"answer",NULL,NULL);
	if (status == ESL_FAIL) {
#ifdef _DEBUG_STDERR
		perror("ESL FAILED TO HANGUP");
#endif
		if ( write(fd,"200 result=failure\n\n",128) < 0 )
			return -1;
	}
	else if (status == ESL_SUCCESS) {
		if ( write(fd,"200 result=success\n\n",128) < 0 )
			return -1;
	}
	fprintf(stderr,"Answered call\n");
	return 0;
}

static int handle_streamfile(esl_handle_t *eslC,int fd,int *argc, char *argv[]) {
	esl_status_t status;
	char buf[1024];

	if (*argc < 3 || *argc > 5)
		return -1;

	if (argv[3]) {
		/* We should set playback_terminators var there */
		sprintf(buf,"playback_terminators=%s",argv[3]);
		fprintf(stderr,"dtmf terminator i don't care at all '%s'",buf);
/*
		status = esl_execute(eslC,"set",buf,NULL);
		if (status == ESL_FAIL)
			fprintf(stderr,"unable to set playback terminators...\n");
*/
	}

	if (argv[4]) {
		fprintf(stderr,"We don't care about offset at the moment... sorry :)\n");
	}

	fprintf(stderr,"Executing playback of '%s'\n",argv[2]);

	status = esl_execute(eslC,"playback",argv[2],NULL);

	/* We should check  playback_samples var to return offset...*/

	if (status == ESL_FAIL) {
#ifdef _DEBUG_STDERR
		perror("ESL FAILED TO HANGUP");
#endif
		if ( write(fd,"200 result=-1\n\n",128) < 0 )
			return -1;
	}
	else if (status == ESL_SUCCESS) {
		if ( write(fd,"200 result=1\n\n",128) < 0 )
			return -1;
	}
	fprintf(stderr,"Playback ended\n");

	return 0;
}


static void parse_args(char *buf,int *argc,char *argv[_MAX_CMD_ARGS]) {
	char *cur;
	int x = 0;
	int quote = 0;
	int escaped = 0;
	int space=1;

	cur = buf;
	while (*buf) {
		switch (*buf) {
			case '"':
				if (escaped)
					goto ok;
				else
					quote = !quote;
				if (quote && space) {
					argv[x++] = cur;
					space=0;
				}
				escaped = 0;
			break;
			case ' ':
			case '\t':
				if (!quote && !escaped) {
					space = 1;
					*(cur++) = '\0';
				} else
					goto ok;
			break;
			case '\\':
				if (escaped)
					goto ok;
				else
					escaped = 1;
			break;
			default:
ok:
				if (space) {
					argv[x++] = cur;
					space=0;
				}
				*(cur++) = *buf;
				escaped=0;
		}
		buf++;
	}
	*(cur++) = '\0';
	argv[x]= NULL;
	*argc = x;
	return;
}

static command_binding_t bindings[_MAX_CMD] = {
	{ {"ANSWER" , NULL}, handle_answer },
	{ {"HANGUP" , NULL}, handle_hangup },
	{ {"STREAM", "FILE" , NULL}, handle_streamfile },
};


static command_binding_t *find_binding(char *cmd[]) {
	int x,y,match=0;

	for (x=0; x < sizeof(bindings); x++) {
		if ( ! bindings[x].cmd[0] ) {
			break;
		}
		match = 1;

		for (y=0; match && cmd[y]; y++) {
			if (!bindings[x].cmd[y]) /* Is the next part existing ?*/
				break;
			if ( strncasecmp(bindings[x].cmd[y],cmd[y], MAX(strlen(bindings[x].cmd[y]),strlen(cmd[y]) ) ) )
				match=0;
		}
		if (bindings[x].cmd[y]) /* There's a word missing in cmd */
			match=0;
		if (match)
			return &bindings[x];
	}
	return NULL;
}

static int find_and_exec_command(esl_handle_t *eslC,int fd,char *buf) {
	char *argv[1024];
	int argc = 0;
	int r = -1;
	command_binding_t *bind;

	parse_args(buf,&argc,argv);

	bind = find_binding(argv);
	if (bind) {
		r = bind->handler(eslC,fd,&argc,argv);
	}
	else {
		fprintf(stderr,"Binding not found: '%s'",buf);
	}
	return r;
}

