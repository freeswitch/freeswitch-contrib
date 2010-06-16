/*
* Copyright (c) 2010, Anthony Minessale II
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
*
* Contributed by João Mesquita <jmesquita@gmail.com>
*
*
* Sponsored by Gabpark
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <esl.h>
#include <getopt.h>

typedef struct {
    char e_name[128];
    char e_header[128];
    char action[128];
} inbound_event_profile_t;

static inbound_event_profile_t profiles[128] = {{{0}}};
static int pcount = 0;

static void mycallback (esl_event_t *event, char * path)
{
    FILE *output;
    char *serialized = NULL;
    
    if (fork() == 0) {
        output = popen (path, "w");
        if (!output) {
          fprintf (stderr, "Could not execute script.\n");
          exit (EXIT_FAILURE);
        }
        /* Pass information to new process */
        esl_event_serialize(event, &serialized, ESL_FALSE);
        fprintf (output, "%s", serialized);
        if (ferror (output)) {
            fprintf (stderr, "Output to stream failed.\n");
            exit (EXIT_FAILURE);
        }
        pclose (output); /* Might be already closed, but might as well try */
        /* Free memory */
        esl_safe_free(serialized);
        exit(0);
    }
}

static int usage(char *name){
	printf("Usage: %s [-H <host>] [-P <port>] [-p <secret>] [-d <level>] [-c configuration]\n\n", name);
	printf("  -?,-h --help                    Usage Information\n");
	printf("  -H, --host=hostname             Host to connect\n");
	printf("  -P, --port=port                 Port to connect (1 - 65535)\n");
	printf("  -u, --user=user@domain          user@domain\n");
	printf("  -p, --password=password         Password\n");
	printf("  -c, --conf=command              Config file to use.\n");
	printf("  -d, --debug=level               Debug Level (0 - 7)\n\n");
	return 1;
}

int main(int argc, char *argv[])
{
    char host[128] = "127.0.0.1", conf[128] = "/etc/fs_eventsd.conf", pass[128] = "ClueCon";
    int port = 8021;
    esl_handle_t handle = {{0}};
    esl_config_t cfg;
    int rv = 0;

    /* Vars for optargs */
	int opt;
	static struct option options[] = {
		{"help", 0, 0, 'h'},
		{"host", 1, 0, 'H'},
		{"port", 1, 0, 'P'},
		{"user", 1, 0, 'u'},
		{"password", 1, 0, 'p'},
		{"debug", 1, 0, 'd'},
		{"conf", 1, 0, 'c'},
		{0, 0, 0, 0}
	};

	char temp_host[128];
    char temp_conf[128];
	int argv_host = 0;
	char temp_pass[128];
	int argv_pass = 0 ;
	int temp_port = 0;
	int argv_port = 0;
	int temp_log = -1;
	int argv_error = 0;
    int argv_conf = 0;
	
    esl_global_set_default_logger(6);
    signal(SIGCHLD, SIG_IGN);

	for(;;) {
		int option_index = 0;
		opt = getopt_long(argc, argv, "H:U:P:S:p:d:c:h?", options, &option_index);
		if (opt == -1) break;
		switch (opt)
		{
			case 'H':
				esl_set_string(temp_host, optarg);
				argv_host = 1;
				break;
			case 'c':
                esl_set_string(temp_conf, optarg);
                argv_conf = 1;
                break;
			case 'P':
				temp_port= atoi(optarg);
				if (temp_port > 0 && temp_port < 65536){
					argv_port = 1;
				} else {
					printf("ERROR: Port must be in range 1 - 65535\n");
					argv_error = 1;
				}
				break;
			case 'p':
				esl_set_string(temp_pass, optarg);
				argv_pass = 1;
				break;
			case 'd':
				temp_log=atoi(optarg);
				if (temp_log < 0 || temp_log > 7){
					printf("ERROR: Debug level should be 0 - 7.\n");
					argv_error = 1;
				} else {
					esl_global_set_default_logger(temp_log);
				}
				break;
				
			case 'h':
			case '?':
				usage(argv[0]);
				return 0;
			default:
				opt = 0;
		}
	}

    if (argv_host){
        esl_set_string(host, temp_host);
    }
    if (argv_pass){
        esl_set_string(pass, temp_pass);
    }
    if (argv_port){
        port = temp_port;
    }
    if (argv_conf){
        esl_set_string(conf, temp_conf);
    }

	if (argv_error){
		printf("\n");
		return usage(argv[0]);
	}

    if (!(rv = esl_config_open_file(&cfg, conf)))
    {
        esl_log(ESL_LOG_ERROR, "No config file, nothing to be done then.\n");
        return -1;
    }

    if (rv)
    {
        char *var, *val;
        char cur_cat[128] = "";
        while (esl_config_next_pair(&cfg, &var, &val))
        {
            if (strcmp(cur_cat, cfg.category))
            {
                esl_set_string(cur_cat, cfg.category);
                esl_set_string(profiles[pcount].e_name, cur_cat);
                pcount++;
            }

            if (!strcasecmp(var, "action"))
            {
                esl_set_string(profiles[pcount-1].action, val);
            }

            if (!strcasecmp(var, "header"))
            {
                esl_set_string(profiles[pcount-1].e_header, val);
            }
        }
    }

    esl_log(ESL_LOG_INFO, "Connecting...\n");
    if (esl_connect(&handle, host, port, NULL, pass) != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_ERROR, "Error Connecting [%s]\n", handle.err);
        return -1;
    }

    esl_log(ESL_LOG_INFO, "OK!\n");

    esl_events(&handle, ESL_EVENT_TYPE_PLAIN ,"all");

    while (handle.connected)
    {
        if(esl_recv_event(&handle, 1, NULL) == ESL_SUCCESS)
        {
            int i;
            for(i = 0; i < pcount; i++)
            {
                if (!strcasecmp(profiles[i].e_name,
                    esl_event_get_header(handle.last_ievent,
                    profiles[i].e_header)))
                {
                    mycallback(handle.last_ievent,
                        profiles[i].action);
                }
            }
        }
    }

    esl_log(ESL_LOG_INFO, "Disconnecting...\n");

    esl_disconnect(&handle);

    return 0;
}
