#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/stat.h>
#include <fcntl.h>

//  {}  \n  []    ||
// gcc -o prueba prueba.c -Wall -Werror

int main(int argc, char ** argv){
	char str[] = "     	 ";

	while(str == ' ' ){
		str++;
		lineSize--;
	}
	
	printf("%d", strlen(str));
}