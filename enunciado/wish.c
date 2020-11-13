#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

//  {}  \n  []    ||
// gcc -o wish wish.c -Wall -Werror
// ./wish
// ./test-wish.sh

int numItems, numPaths, redir;
char  ** items;
char **shellPaths;
char error_message[30] = "An error has occurred\n";

void procesarItems();
void salir();
void cambiarDir();
void addPath();
void ejecutarComando();
int parser(char *string, ssize_t lineSize);

int main(int argc, char ** argv){
	numPaths = 1;
	shellPaths = malloc((numPaths) * sizeof(char*));
	shellPaths[0] = "/bin/";
	
	// INTERACTIVE MODE
	if(argc == 1){
		char *linea;
		size_t len = 0;
		ssize_t lineSize = 0;
		
		// Ciclo principal
		while(1){
			printf("wish> ");
			lineSize = getline(&linea, &len, stdin);

			if(lineSize <= 1){
				continue;
			}

			numItems = parser(linea, lineSize);

			if(numItems == -1){
				write(STDERR_FILENO, error_message, strlen(error_message));
				continue;
			}
			
			if(numItems == 0)
				continue;

			procesarItems();
		}
	}
	// BATCH MODE
	else if(argc == 2){

		FILE *file;
		char *linea;
		size_t len = 0;
		ssize_t lineSize = 0;

		file = fopen(argv[1], "r");

		// Si no se abre el archivo
		if(!file){
			// Error
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}

		lineSize = getline(&linea, &len, file);

		while (lineSize >= 0){		
			numItems = parser(linea, lineSize);

			if(numItems == -1){
				write(STDERR_FILENO, error_message, strlen(error_message));
				lineSize = getline(&linea, &len, file);
				continue;
			}

			if(numItems == 0){
				lineSize = getline(&linea, &len, file);
				continue;
			}

			procesarItems();

			lineSize = getline(&linea, &len, file);
		}

	}
	// ERROR
	else{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	exit(0);
}

// Método para procesar los comandos luego de leerlos
void procesarItems(){
	if(strcmp(items[0], "exit") == 0){
		salir();
	}
	else if(strcmp(items[0], "cd") == 0){
		cambiarDir();
	}
	else if(strcmp(items[0], "path") == 0){
		addPath();
	}
	else{
		ejecutarComando();
	}

	free(items);
	numItems = 0;
}

// Método para salir del sistema
void salir(){
	if(numItems > 1){
		// Error
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}

	exit(0);
}

// Método para cambiar el directorio de trabajo
void cambiarDir(){
	if(numItems > 2 || numItems == 1){
		// Error
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}

	if(chdir(items[1]) != 0){
		// Error 
		write(STDERR_FILENO, error_message, strlen(error_message));
	}		
}

// Método para sobreescribir el shell path
void addPath(){
	numPaths = numItems - 1;

	char **newPaths = malloc((numPaths) * sizeof(char*));
	char *path;

	for(int i = 0; i < numPaths; i++){
		path = (char *) malloc(strlen(items[i+1]) + 1);
		strcat(path, items[i+1]);
		strcat(path, "/");
		newPaths[i] = path;
	}
	
	shellPaths = newPaths;
}

// Método para ejecutar comandos externos
void ejecutarComando(){

	// No hay shell path de búsqueda
	if(numPaths == 0){
		// Error 
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}

	char *path, *com =  (char *) malloc(strlen(items[0]) + 1);
	
	strcpy(com, items[0]);
	
	// Buscar en cual directorio se puede ejecutar el comando
	for(int i = 0; i < numPaths; i++){
		path = (char *) malloc(strlen(shellPaths[i]) + 1);
		strcpy(path, shellPaths[i]);
		strcat(path, com);

		if(access(path, X_OK) == 0)
			break;

		// Si no se encuentra directorio no se ejecuta el comando
		if(i == (numPaths - 1)){
			write(STDERR_FILENO, error_message, strlen(error_message));
			return;
		}
	}
	// Se puede ejecutar el comando
	int pid = fork();

	// Sin redirección
	if(redir == 0){
		if(pid == 0){
			// Hijo
			if(execvp(path, items)){
				// Error
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
		else if(pid > 0){
			// Padre
			wait(NULL);
		}
		else{
			// Error
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
	}
	// Con redirección
	else{
		if(pid == 0){
			// Hijo

			int fp;

			if((fp = open(items[numItems], O_WRONLY | O_CREAT |O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1){
				// Error
				write(STDERR_FILENO, error_message, strlen(error_message));
				return;
			}

			if(dup2(fp, STDOUT_FILENO) == -1){
				// Error
				write(STDERR_FILENO, error_message, strlen(error_message));
				return;
			}

			close(fp);

			items[numItems] = NULL;

			if(execvp(path, items)){
				// Error
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
		else if(pid > 0){
			// Padre
			wait(NULL);
		}
		else{
			// Error
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
	}
	
	free(com);
	free(path);
}

// Método para obtener los argumentos
int parser(char *string, ssize_t lineSize){
	char *found, **words;

	int num = 1, redirFiles = 0, len = lineSize;
	redir = 0;

	string[lineSize - 1] = '\0';

	// Convertir tabulaciones y saltos de linea en espacio
	for(int i = 0; i < lineSize; i++){
		if(string[i] == '\t' || string[i] == '\n')
			 string[i] = ' ';
	}	

	// Eliminar espacios del principio
	while(*string == ' ' ){
		string++;
		len--;
	}

	if(len <= 1)
		return  0;

	// Contar número de palabras
	for(int i = 1; string[i] != '\0'; i++){
		// Error no más de un operador
		if(string[i] == '>' && redir == 1)
			return -1;

		if(string[i] == '>'){
			redir = 1;
			string[i] = ' ';
		}

		if( string[i] != ' ' &&  string[i-1] == ' ' && redir == 0)
			num++;

		if(string[i] != ' ' &&  string[i-1] == ' ' && redir == 1)
			redirFiles++;
	}

	// Error más de 1 archivo o ninguno para redirigir
	if(redir == 1 && (redirFiles > 1 || redirFiles == 0))
		return -1;

	// Reservar espacio en memoria
	words = malloc((num + 1) * sizeof(char*));

	// Llenar memoria con palabras
	int i = 0;
	while((found = strsep(&string, " ")) != NULL){
		if(*found != '\0'){			
			words[i++] = found;
		}			
	}

	// Último argumento como nulo
	if(redir == 0)
		words[i] = NULL;
	
	items = words;
	return num;
}