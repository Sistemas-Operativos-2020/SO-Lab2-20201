#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

//  {}  \n  []
int main(int argc, char ** argv){
	
	if(argc < 2){
		printf("No se ha ingresado ningún comando \n");
	} 
	else{
		char *args[argc];
		struct timeval start_time, end_time;
		double tiempo;

		for(int i = 0; i < (argc - 1); i++){
			args[i] = argv[i + 1];
		}
		args[argc - 1]  = NULL;
		
		int rc = fork();

		if (rc < 0){
			printf("Fork failed\n");
			return 1;
		}
		else if (rc == 0){			
			execvp(args[0], args);
		}
		else{
			gettimeofday(&start_time, NULL);
			wait(NULL);
			gettimeofday(&end_time, NULL);
			tiempo = (end_time.tv_sec - start_time.tv_sec)  * 1000000 + 
					(end_time.tv_usec - start_time.tv_usec);
			tiempo = tiempo / 1000000;
			printf("Tardó: %f segundos\n", tiempo);	
		}

	}
	return 0;
}