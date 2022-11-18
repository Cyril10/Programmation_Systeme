#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <stdbool.h>

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
	if(argc == 2){
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
				bool stop = false;
				while(stop == false){
					read(F[i][0], &intervalle, 2*sizeof(int));
					if(intervalle[0] != 0){
						//on cherche et renvoies les nombre premiers au pere
						for(int temp = intervalle[0]; temp <= intervalle[1]; temp++){
							if(premier(temp) == 1){
								write(P[i][1], &temp, sizeof(int));
							}
						}
						//valeur d'arret de lecture pour le pere
						int arret = 0;
						write(P[i][1], &arret, sizeof(int));
					}else{
						stop = true;
					}
				}
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
		
		//on envoie les sousintervallesAAfficher et on récupères et affichesles valeurs par les fils
		bool FINI = false;
		int compteurBoucleFils = 0; //permet de savoir combien de fois on a envoyer aux fils les sousIntervalles, si on a p fils et qu'on a envoyer p intervalle, alors on fait compteurBoucleFils++
		
		//on ferme cotes du tube qu'on n'utilise pas
		for(int i=0; i<10; i++){
			close(F[i][0]);
			close(P[i][1]);
		}
		
		while(FINI == false){
			//sert a envoyer les intervalles au fils
			i=0;
			while( (i < 10) && (FINI == false)){
				//on créer et envoie les sousIntervalleAEnvoyer
				int sousIntervalleAEnvoyer[2];
				sousIntervalleAEnvoyer[0] = sousIntervalle[(10*compteurBoucleFils) + i];
				sousIntervalleAEnvoyer[1] = sousIntervalle[(10*compteurBoucleFils) + i+1]-1;
				
				write(F[i][1], &sousIntervalleAEnvoyer, 2*sizeof(int));
				//on verifie si tous les intervalles ont été envoyé
				
				if(10*compteurBoucleFils + i+1 == n){
					FINI = true;
				}
				i++;
			}
			
			//on récupère les valeurs
			int j = 0;
			while(( j < i)){ //j inf a i, car si on arrete la boucle avant un tour complet des fils, il faut savoir a partir de quel fils on arrete de lire, si on a envoyer le dernier intervalle au fils 3, il faut arreter de lire au tube 3, on s'arrete donc à i-1 (i-1 car on fait i++)
				int tampon = 1;
				while(tampon != 0){
					read(P[j][0], &tampon, sizeof(int));
					if(tampon != 0){
						printf("%d ", tampon);
					}
				}
				printf("\n");
				j++;
			}
		compteurBoucleFils++;
		}
		
		//condition d'arret pour les fils
		int intervalleArret[2];
		intervalleArret[0] = 0;
		intervalleArret[1] = 0;
		for(i=0; i<10; i++){
			write(F[i][1], &intervalleArret, 2*sizeof(int));
		}
		
		//on ferme les cotes du tube qu'on vient d'utiliser
		for(int i=0; i<10; i++){
			close(F[i][1]);
			close(P[i][0]);
		}
		
		for(i=0; i<10; i++){
			wait(NULL);
		}
	}else{
		printf("Il faut entrer un parametre (10)");
	}
}
