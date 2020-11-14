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
	char *string;
	size_t len = 0;
	ssize_t lineSize = 0;		
		
	printf("wish> ");
	lineSize = getline(&string, &len, stdin);

	string[lineSize - 1] = '\0';

	// Convertir tabulaciones y saltos de linea en espacio
	for(int i = 0; i < lineSize; i++){
		if(string[i] == '\t' || string[i] == '\n')
			 string[i] = ' ';
	}	

	char *found;
	char *temp = malloc(strlen(string) + 1);
	strcpy(temp, string);
	int nComs = 0;
	// Contar cantidad de comandos a ejecutar
	while((found = strsep(&temp, "&")) != NULL){
		if(*found != '\0'){	
			// Eliminar espacios del principio
			while(*found == ' ' ){
				found++;
			}
			if(strlen(found) > 0)
				nComs++;
		}			
	}

	if(nComs == 0){
		return;
	}

	char **comsPar = malloc(nComs * sizeof(char*));

	int i = 0;
	// Separar comandos
	while((found = strsep(&string, "&")) != NULL){
		if(*found != '\0'){	
			// Eliminar espacios del principio
			while(*found == ' ' ){
				found++;
			}

			if(strlen(found) > 0){				
				comsPar[i] = found;
				i++;
			}	
		}			
	}
	
}