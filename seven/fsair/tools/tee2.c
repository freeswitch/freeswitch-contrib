/* tee2 - read from standard input and write to standard output, 
   standard error and a file.
   Useful when you redirect the stdout but still want to keep an eye on
   the output. Like a B2BSocket:                                             

   $ echo > /tmp/a
   $ echo > /tmp/b
   $ tail -f /tmp/a | nc localhost 8021 | tee2 /tmp/b | nc -l -k 8022 | tee /tmp/a

   Want to write to more than one file? Just using the UNIX art
   $ echo blah | tee2 /tmp/1 | tee /tmp/{2,3,4,5,6}

/* Author: Seven Du  */
 
#include <stdio.h>
#include <stdlib.h>
 
int main (int argc, char **argv)
{
	char c;
	char *file_name;
	FILE *file;
	if (argc < 2) {
		fprintf(stderr, "Usage: tee2 <file_name>\n");
		exit(1);
	}           
	    
	file_name = argv[1];
	
	if(! (file = fopen(file_name, "w")) ){
		fprintf(stderr, "Error write file %s !\n", file_name);
		exit(2);
	}           
	
	while((c = fgetc(stdin)) != EOF) {
		putc(c, file);
		putc(c, stdout);
		putc(c, stderr);
		
		if(c == '\n') {
			fflush(file);
			fflush(stdout);
			fflush(stderr);
		}
		
	}                      
	
	fclose(file);
	exit(0);
} 
