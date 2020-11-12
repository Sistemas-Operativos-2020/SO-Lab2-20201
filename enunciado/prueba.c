#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char ** argv){
	char path[] = "tests/p1.sh";

	printf("%s\n", path);

	if(access(path, X_OK) == 0){
		printf("Se puede ejecutar\n");
		
		int pid = fork();

		if(pid == 0){
			// Hijo
			if(execvp(com, items)){
				// Error
				printf("Comando no ejecutado\n");
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
	else{
		printf("No se puede ejecutar\n");
	}
}