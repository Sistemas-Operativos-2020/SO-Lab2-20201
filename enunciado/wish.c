#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//  {}  \n  []
// gcc -o wish wish.c -Wall -Werror

int numItems, numPaths = 1;
char  ** items;
char *shellPaths[] =  {"/bin/"};

void procesarItems();
void salir();
void ejecutarComando();
int parser(char *string, char ***items, ssize_t lineSize);

int main(int argc, char ** argv){
	
	// INTERACTIVE MODE
	if(argc == 1){
		int seguir = 1;
		char *linea;
		size_t len = 0;
		ssize_t lineSize = 0;
		
		// Ciclo principal
		while(seguir == 1){
			printf("wish> ");
			lineSize = getline(&linea, &len, stdin);
			numItems = parser(linea, &items, lineSize);

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
			printf("No se pudo abrir el archivo\n");
			exit(1);
		}

		lineSize = getline(&linea, &len, file);

		while (lineSize >= 0){

			numItems = parser(linea, &items, lineSize);

			procesarItems();

			lineSize = getline(&linea, &len, file);
		}

	}
	// ERROR
	else{
		printf("Demasiados argumentos\n");
		exit(1);
	}
	exit(0);
}

// Método para procesar los comandos luego de leerlos
void procesarItems(){
	if(strcmp(items[0], "exit") == 0){
		salir();
	}
	else{
		ejecutarComando();
	}
}

// Método para salir del sistema
void salir(){
	free(items);
	exit(0);
}

// Método para ejecutar comandos externos
void ejecutarComando(){

	char *path, *com =  (char *) malloc(strlen(items[0]) + 1);
	
	strcpy(com, items[0]);
	
	// Buscar en cual directorio se puede ejecutar el comando
	for(int i = 0; i < numPaths; i++){
		path = (char *) malloc(strlen(shellPaths[i]) + 1);
		strcpy(path, shellPaths[i]);

		strcat(path, com);	

		if(access(path, X_OK) == 0)
			break;

		// Si no se encuentra directorio se termina la ejecución
		if(i == (numPaths - 1)){
			free(com);
			free(path);
			printf("Comando no reconocido o no se pudo ejecutar\n");
			return;
		}
	}
	// Se puede ejecutar el comando
	int pid = fork();

	if(pid == 0){
		// Hijo
		if(execvp(com, items)){
			// Error
			printf("Ocurrió un error al ejecutar el comando\n");
		}
	}
	else if(pid > 0){
		// Padre
		wait(NULL);
	}
	else{
		// Error
		printf("No se pudo crear un proceso hijo\n");
	}
	
	free(com);
	free(path);
}

// Método para obtener los argumentos
int parser(char *string, char ***items, ssize_t lineSize){
	char *found, **words;
	int num = 1;

	string[lineSize - 1] = '\0';

	// Convertir tabulaciones y saltos de linea en espacio
	for(int i = 0; i < lineSize; i++){
		if(string[i] == '\t' || string[i] == '\n')
			 string[i] = ' ';
	}	

	// Eliminar espacios del principio
	while(*string == ' ' )
		string ++;

	// Contar número de palabras
	for(int i = 1; string[i] != '\0'; i++){
		if( string[i] != ' ' &&  string[i-1] == ' ')
			num++;
	}

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
	words[i] = NULL;
	
	*items = words;
	return num;
}