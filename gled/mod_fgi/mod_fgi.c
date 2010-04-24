/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2010, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Tristan Mahé <t.mahe@telemaque.fr>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Anthony Minessale II <anthm@freeswitch.org>
 * 
 * mod_fgi.c -- Launch a background script to control a call via basic langage
 *
 */
#include <switch.h>

/* Be compiler Happy */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_fgi_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_fgi_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_fgi_load);
SWITCH_MODULE_DEFINITION(mod_fgi, mod_fgi_load, mod_fgi_shutdown, NULL);


/* here we go */
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
  
#define FG_READ 0
#define FG_WRITE 1
#define _MAX_CMD_ARGS 128

#define MAX(x,y)                ((x) > (y) ? (x) : (y))
#define MIN(x,y)                ((x) < (y) ? (x) : (y))

static void parse_fgi_args(char *buf,int *argc,char *argv[_MAX_CMD_ARGS]);

pid_t fgi_open_process( const char *cmd, int *in, int *out,int *thread_running) {
	int p_in[2], p_out[2];
	pid_t pid;

	if (pipe(p_in) != 0 || pipe(p_out) != 0) {
		return -1;
	}

	*thread_running=1;
	pid = fork();
 
	if (pid < 0) {
		return pid;
	}
	else if (pid == 0) {
		close(p_in[FG_WRITE]);
		dup2(p_in[FG_READ], FG_READ);
		close(p_out[FG_READ]);
		dup2(p_out[FG_WRITE], FG_WRITE);
		execl(cmd, cmd,NULL);
		*thread_running=0;
		perror("execl");
		close(p_in[FG_READ]);
		close(p_out[FG_WRITE]);
		exit(0);
	}
  
	if (in == NULL) {
		close(p_in[FG_WRITE]);
	}
	else {
		*in = p_in[FG_WRITE];
	}

	if (out == NULL) {
		close(p_out[FG_READ]);
	}
	else {
		*out = p_out[FG_READ];
	}
	close(p_in[FG_READ]); //  We just write there
	close(p_out[FG_WRITE]); // and we only read there

	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	return pid;
}

static switch_status_t find_and_exec_command(char *cmd, int fd_reply) {
	char *argv[1024];
	int argc = 0;

	parse_fgi_args(cmd,&argc,argv);
	/* */
	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t exec_fgi_run(char *argv) {
	int in,out;
	pid_t pid;
	int res=0;
	int nfds=0;
	fd_set rd;
	int thread_running;
	char cmdbuf[_MAX_CMD_ARGS*128];

	pid = fgi_open_process(argv,&in,&out,&thread_running);
	if ( pid <= 0)
		return SWITCH_STATUS_SUCCESS;

	while (thread_running) {
		FD_ZERO(&rd);
		FD_SET(in, &rd);
		nfds = MAX(nfds,out);

		res = select(nfds + 1, &rd, NULL, NULL, NULL);
		if (res == -1 )
			break;
		else if (FD_ISSET(out,&rd) ) { /* They have something to say */
 			res = read(out,&cmdbuf,2047);
			if (res<0)
				goto end;
			else {
				if ( find_and_exec_command((char *) &cmdbuf,in) != SWITCH_STATUS_SUCCESS )
					goto end;
				}
                        }
                }
end:
	kill(pid,SIGHUP);
	close(in);
	close(out);
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_STANDARD_APP(launch_fgi_thread) {
	char *argv[6] = { 0 };
	int argc;
	char *mydata = NULL;
//	switch_channel_t *channel = switch_core_session_get_channel(session);

	if (zstr(data) || !(mydata = switch_core_session_strdup(session, data))) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "INVALID ARGS!\n");
		return;
	}

	if ((argc = switch_separate_string(mydata, ' ', argv, sizeof(argv) / sizeof(argv[0]))) < 2) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "INVALID ARGS!\n");
		return;
	}
	exec_fgi_run(mydata);
	/* Free up things ? */
}

SWITCH_MODULE_LOAD_FUNCTION(mod_fgi_load) {
	switch_application_interface_t *app_interface;

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Hello !\n");
        SWITCH_ADD_APP(app_interface, "fgi", "Fgi", "launch and give control to an external entity with STDIN/STDOUT", launch_fgi_thread, "fgi <path_to_executable> [<args>]", SAF_NONE);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_fgi_shutdown) {
	/* Should kill remaining AGI process on shutdown ?? 
		Should del cmd too
	*/
	return SWITCH_STATUS_SUCCESS;
}


static void parse_fgi_args(char *buf,int *argc,char *argv[_MAX_CMD_ARGS]) {
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

