/*
 *   FreeSWITCH Console
 */

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <winsock2.h>
#else
#define closesocket close
#endif

#ifdef _MSC_VER
#include <io.h>
#define write _write
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <unistd.h>
#endif


#define buflen 4096
#define NUL '\0'

char *app = "fsconsole";

/* Defaults according to current (12-06-06) freeswitch.xml */
char *hostname = "localhost";
char *secret = "ClueCon";
u_short port = 8021;

/* Debug Level */
int debug = 0;

char *intro = "Welcome to the FreeSWITCH Console.\n";

char    buffer[buflen], *command;
#ifdef WIN32
SOCKET  sd;
#else
int     sd;
#endif
int		flag, prompt, display;

char *trim(char *str);
void print_tokens(char *line);


#ifdef WIN32
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

typedef enum {
	STATUS_SUCCESS
} status_t;

static int socket_set_nonblock(socket_t sd)
{
#ifdef WIN32
	u_long argp = 1;
	if (ioctlsocket(sd, FIONBIO, &argp) == SOCKET_ERROR) {
		return ((WSAGetLastError() == 0) ? STATUS_SUCCESS : WSAGetLastError());
	}
#else
	flag = fcntl(sd, F_GETFL, 0);
	flag |= O_NONBLOCK;
	if (fcntl(sd, F_SETFL, flag) == -1) {
		return errno;
	}
#endif
	return STATUS_SUCCESS;
}

unsigned long getaddr(char *host_name)
{
	struct hostent *host;
	unsigned long addr = 0;
	struct in_addr address;

	if (isalpha(host_name[0])) {   /* host address is a name */
		host = gethostbyname(host_name);
		if(!host) {
			return 0;
		} else {
			address = *(struct in_addr*)host->h_addr_list[0];
			addr = address.s_addr;
		}
	}
	else  { 
	  addr = inet_addr(host_name);
	}

	return addr;
}

int main(int argc, char *argv[])
{

	int len;
	char    contents[4096];

	char whitespace[] = "\r";
	char *token, *token_split, *subtoken_split;
	char token_key[32], token_val[32], token_contents[4096], subtoken_contents[4096];
	char subtoken_key[32], subtoken_val[32];

	struct sockaddr_in sin = {0};

	char cmd[256];

	if (argv[1] && !strcmp(argv[1], "--help")) {
		fprintf(stderr,"usage %s hostname port secret\n", argv[0]);
		exit(0);
	}

	/* Hostname, Port and Secret defined on command line */
	if (argc > 1) {
		hostname = argv[1];
	} else if (argc > 2) {
		port = (u_short)atoi(argv[2]);
	} else if (argc > 3) {
		secret = argv[3];
	}

	/* Init socket descriptor */
	sd = socket(AF_INET, SOCK_STREAM, 0);  

	/* Put Data intoN sockaddr_in struct */
	sin.sin_family = AF_INET;
	sin.sin_port =  htons(port);
	sin.sin_addr.s_addr = getaddr(hostname);

	/* Connect Socket */
	if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("connecting");
		exit(1);
	}

	/* Set Socket Descriptor to Non-Blocking */
	socket_set_nonblock(sd);

	/* Set the prompt to non-blocking */
	socket_set_nonblock(0);

	/* Print the Introduction */
	printf(intro);

	/* First Prompt = No, would show before authentication.... */
	prompt = 0;

	/* While Program is Running... */
	for(;;)
     	{
		/* If Authenticated -- print the current version */
		if (command)
		{
			if(!strcmp(command,"authenticated"))
			{
				/* Print the version */
				printf("\nRunning Version:\n");
				sprintf(buffer, "api version\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));

				/* Authentication Complete -- set current command to blank */
				command = "";
			}
		}

		/* Prompt */
		if (prompt)
		{
			printf("\n%s@%s> ", app, hostname);
			prompt = 0;
		}

                memset(contents, 0, sizeof(buffer));
                memset(buffer, 0, sizeof(buffer));
                memset(cmd, 0, sizeof(cmd));

		len = atoi(buffer);
		if (len <= 0) len += buflen;
		len = recv(sd, buffer, len, 0);
  
		if (len > 0)
		{
			if (debug > 1)
				printf("Received a response: len %d.\n", len);

			if (debug > 2)
				display = 1;

			memset(contents, 0, sizeof(contents));
			strncpy(contents, buffer, len);

			/* Print Incoming input to screen if debug enabled or display is set */
			if (debug > 0 || display == 1) 
			{
				write(1,contents,len);

				/* If not debugging, Prevent Display of Response */
				if (debug < 2)
					display = 0;

				/* Response received -- Set the prompt */
				prompt = 1;


			}

			/* print_tokens(contents); */
			for (token=strtok(contents,whitespace); token !=NULL; token=strtok(NULL,whitespace))
			{

				/* Multiline requires \r, set back to \r
					to properly execute the display of responses! UNF */
				strcpy(whitespace, "\r");

				sprintf(token_contents, "%s", token);
	
				token_split = strtok(token_contents, ":");
					sprintf(token_key, "%s", token_split);
				token_split = strtok(NULL, "\n");
					sprintf(token_val, "%s", token_split);
				token_split = strtok(NULL, "\n");

				token = strtok(NULL, "\n\n");

				trim(token_val);

				if (debug > 2)
					printf("\nkey: '%s', val: '%s'\n", token_key, token_val);

				if (strcmp(token_key, "Content-Type") == 0)
				{
					display = 1;

					if (strcmp(token_val, "auth/request") == 0)
					{

						printf("\nAuthenticating...\n");
			
						command = "auth";
						sprintf(buffer, "auth %s\n\n", secret);
						write((int)sd, buffer, (unsigned int)strlen(buffer));

						/* If not debugging, Prevent Display of Response */
						if (debug < 2)
							display = 0;

				     	} else if (strcmp(token_val, "command/reply") == 0) {

						/* Parse the reply */		
						sprintf(subtoken_contents, "%s", token_split);
	
						subtoken_split = strtok(subtoken_contents, ":");
							sprintf(subtoken_key, "%s", subtoken_split);
						subtoken_split = strtok(NULL, "\n");
							sprintf(subtoken_val, "%s", subtoken_split);
						subtoken_split = strtok(NULL, "\n");

						trim(subtoken_val);

						if (debug > 2)
							printf("subkey: '%s', subval: '%s'\n", subtoken_key, subtoken_val);

						if (strcmp(subtoken_key, "Reply-Text") == 0)
						{
							if (strcmp(subtoken_val, "+OK accepted") == 0)
							{
								if (strcmp(command, "auth") == 0)
								{
									printf("Authentication Successful\n");
									command = "authenticated";

									/* Auth Command received -- Prompt and Display handled elsewhere */
									prompt = 0;
									display = 0;

									/* Authentication required a \r, set back to \n 
										to properly execute the display of responses! */
									strcpy(whitespace, "\n");

								}
							} else if (strncmp(subtoken_val, "-", 1) == 0) {
								/* Received an error, print */

								printf("\n%s\n", subtoken_val);
								/* Since we received a response, set the prompt */
								prompt = 1;

							} else if (strncmp(subtoken_val, "+", 1) == 0) {
								/* Received an uncoded OK, print */

								printf("\n%s\n", subtoken_val);
								/* Since we received a response, set the prompt */
								prompt = 1;

							}

											/* || "ts" is a hack -- why? */
						} else if (strcmp(subtoken_key, "") == 0 || strcmp(subtoken_key, "ts") == 0) {
							printf("\n%s\n", subtoken_val);
							/* Since we received a response, set the prompt */
							prompt = 1;
						}

					} else if (strcmp(token_val, "api/response") == 0) {

						/* Since we received an api response, display results */
						if (command && strcmp(command,"authenticated"))
						{
							display = 1;
						}

					}
				} 

			}

		}

		/* Get the command */
		fgets(cmd, 254, stdin);

		/* Received a Command */
		if (strlen(cmd))
		{

			/* Received an Enter Only - Reset Prompt*/
			if (strcmp(cmd, "\n") == 0) {
				prompt = 1;
			}

			/* Drop the \n */
			cmd[strlen(cmd) - 1] = 0;

			/* If not debugging, Prevent Display of Response */
			if (debug < 2)
				display = 0;
			
			if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
				printf("Exiting.. goodbye.\n");
				break;
			} else if (strncmp(cmd, "event", 5) == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strcmp(cmd, "noevents") == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strncmp(cmd, "log", 3) == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strcmp(cmd, "nolog") == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strncmp(cmd, "api", 3) == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strncmp(cmd, "bgapi", 5) == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strncmp(cmd, "sendevent", 9) == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strncmp(cmd, "sendmsg", 7) == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				sprintf(buffer, "%s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else if (strncmp(cmd, "help", 4) == 0 || strncmp(cmd, "?", 1) == 0) {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);

				printf("\n-  Console  -\n");
				printf("Valid Commands:\n\n");
				printf("event [xml|plain] <list of events to log or all for all> - enable or disable events by class or all\n");
				printf("noevents - disable all events that were previously enabled w/ event\n");
				printf("nixevent <some_event> - command to allow 'events all' followed by 'nixevent <some_event>' to do all but <some_event>\n");
				printf("log <level> - enable log output.  Levels same as the console.conf values\n");
				printf("nolog - disable log output previously enabled by the log command\n");
				printf("api <command> <arg> - send an api command (blocking mode)\n");
				printf("bgapi <command> <arg> - send an api command (non-blocking mode) this will let you execute a job in the background\n");
				printf("sendevent <event-name> - send an event into the event system (multi line input for headers)\n");
				printf("exit - close the fsconsole\n");

				printf("\n-    API    - ");

				/* Print the additional api commands available from console... */
				sprintf(buffer, "api help\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));
			} else {
				if (debug > 2)
					printf("Received Command: %s\n", cmd);


				sprintf(buffer, "api %s\n\n", cmd);
				write((int)sd, buffer, (unsigned int)strlen(buffer));

				printf("\n");

			}

		}

		/* 100% CPU fix */
		usleep(1);

	}

   closesocket(sd);
}

char *trim(char *str)
{
	char *ibuf = str, *obuf = str;
	int i = 0, cnt = 0;

	/*
	**  Trap NULL
	*/

	if (str)
	{
		/* Remove leading spaces */
		for (ibuf = str; *ibuf && isspace(*ibuf); ++ibuf)
		;
		if (str != ibuf)
			memmove(str, ibuf, ibuf - str);

		/*  Collapse embedded spaces */

		while (*ibuf)
		{
			if (isspace(*ibuf) && cnt) {
				ibuf++;
			} else {

				if (!isspace(*ibuf)) {
					cnt = 0;
				} else {
					*ibuf = ' ';
					cnt = 1;
				}

				obuf[i++] = *ibuf++;
			}
		}

		obuf[i] = NUL;

		/*  Remove trailing spaces */
		while (--i >= 0)
		{
			if (!isspace(obuf[i]))
				break;
		}

		obuf[++i] = NUL;
	}

	return str;

}


