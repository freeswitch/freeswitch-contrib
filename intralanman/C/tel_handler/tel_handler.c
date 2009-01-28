#include <stdio.h>
#include <stdlib.h>
#include <esl.h>

int main(int argc, char *argv[])
{
	esl_handle_t handle = {{0}};
	if(argc == 2) {
		char buf[1024];
		esl_connect(&handle, "localhost", 8021, "ClueCon");
		snprintf(buf, sizeof(buf), "api pa call %s", argv[1]);
		esl_send_recv(&handle, buf);
		esl_disconnect(&handle);
		return 0;
	} else {
		printf("Not enough args!!");
		return 1;
	}
}
