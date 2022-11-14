#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

//return 1 si le chiffre est premier
int premier(int n){
	int i;
	if(n<2) return 0;
	if(n==2) return 1;
	for(i=2; i<n; i++){
		if(n%i == 0) return 0;
	}
	return 1;
}

int main(int argc, char **argv){
	int i, F[10][2], P[10][2];
	
	//on créer les 20 pipes, 10 pour commu pere->fils (F) et 10 pour commu fils->pere (P)
	for(i=0; i<10; i++){
		if(pipe(F[i])==-1){
			perror("erreur tube F");
			exit(1);
		}
		if(pipe(P[i])==-1){
			perror("eereur tube P");
			exit(1);
		}
	}
	
	//on passe dans la zone des 10 fils
	for(i=0; i<10; i++){
		int pid;
		//création des fils
		pid=fork();
		if(pid<0){
			perror("Erreur de fork");
			exit(1);
		}
		if(pid==0){
			int tampon, compteur = 0;
			int intervalle[2];
			//on ferme les tubes inutiles
			for(int j=0; j<10; j++){
				if(j!=i){
					//les tubes F
					close(F[j][0]);
					close(F[j][1]);
					//les tubes P
					close(P[j][0]);
					close(P[j][1]);
				}
			}
			//on ferme le coté du tube inutile (ecriture dans F et lecture dans P)
			close(F[i][1]);
			close(P[i][0]);
			
			//on lit ce qu'il y a dans le tube (les deux valeurs de l'intervalle a chercher)
			while(read(F[i][0], &tampon, sizeof(int)) != 0){
				intervalle[compteur] = tampon;
				compteur++;
			}
			
			//on cherche et renvoies les nombre premiers au pere
			for(int temp = intervalle[0]; temp <= intervalle[1]; temp++){
				if(premier(temp) == 1){
					write(P[i][1], &temp, sizeof(int));
				}
			}
			//valeur d'arret de lecture pour le pere
			int arret = 0;
			write(P[i][1], &arret, sizeof(int));
			
			//on ferme le côté du tube qui vient d'être utilisé	
			close(F[i][0]);
			close(P[i][1]);
			
			exit(0);
		}
	}
	
	//on est dans le pere
	
	//on créer l'intervalle
	int intervalle[2];
	intervalle[0] = 2;
	intervalle[1] = 1000;
	
	//on initialise la taille des sous intervalles en fonction de l'entrée utilisateur(pour l'instant 10)
	int n;
	n = (int)strtol(argv[1], NULL, 10);
	
	int tailleSousIntervalle = intervalle[1] - intervalle[0];
	tailleSousIntervalle = (int)tailleSousIntervalle / n;
	
	//on initialise et rempli le tableau de sous intervalle
	int sousIntervalle[n+1]; //n+1 car on veut la premiere et la derniere valeurs de l'intervalle principale
	
	for(int i = 0; i <= n; i++){
		int temp = 0;
		temp = intervalle[0] + (i*tailleSousIntervalle);
		if(i==n){
			temp = temp + (intervalle[1] - intervalle[0]) % n; //on rajoute les valeurs manquante car on arrondi
		}
		sousIntervalle[i] = temp;
	}
	
	//on envoie les sousIntervalles
	for(int i = 0; i<10; i++){
		int sousIntervalleAEnvoyer[2];
		sousIntervalleAEnvoyer[0] = sousIntervalle[i];
		sousIntervalleAEnvoyer[1] = sousIntervalle[i+1] - 1;
		
		close(F[i][0]);
		write(F[i][1], &sousIntervalleAEnvoyer, 2*sizeof(int));
		close(F[i][1]);
	}
	
	for(int i =0; i<10; i++){
		close(P[i][1]);
	}
	
	int tampon = 1; //1 car la valeur d'arret est 0
	for(int i = 0; i<10; i++){
		while(tampon != 0){
			read(P[i][0], &tampon, sizeof(int));
			if(tampon != 0){
				printf("%d ", tampon);
			}
		}
		tampon = 1;
		printf("\n");
	}
	
	for(int i = 0; i<10; i++){
		close(P[i][0]);
	}
	
	for(i=0; i<10; i++){
		wait(NULL);
	}
}
