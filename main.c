#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int cmpt=0;

void signal_handler1(int sig){
	printf("pong%d ", cmpt);
	fflush(stdout);
	cmpt++;
}

void signal_handler2(int sig){
	printf("ping%d ", cmpt);
	fflush(stdout);
	cmpt++;
}

int main(int argc, char **argv){
	pid_t pid;
	int nb_boucle;
	nb_boucle = (int)strtol(argv[1], NULL, 10);
	if(argc == 2){
		signal(SIGUSR2, signal_handler1);
		signal(SIGUSR1, signal_handler2);
		
		pid=fork();
		
		if(pid < 0){
			perror("erreur de fork");
			exit(1);
		}
		if(pid==0){
			//sleep(1);
			pid_t parent = getppid();
			for(int i = 0; i < nb_boucle; i++){
				kill(parent, SIGUSR2);
				pause();
				//sleep(1);
			}
			return 0;
		}else{
			for(int i = 0; i < nb_boucle; i++){
				kill(pid, SIGUSR1);
				pause();
				//sleep(1);
			}
		}
		printf("\n");
	}else{
		printf("Il faut entrer le nombre de ping pong a afficher");
	}
	return 0;
}
