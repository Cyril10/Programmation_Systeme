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

int main(){
	int i, T[10][2];
	
	//on créer les 10 pipes, 1 pour chaque fils
	for(i=0; i<10; i++){
		if(pipe(T[i])==-1){
			perror("erreur tube");
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
					close(T[j][0]);
					close(T[j][1]);
				}
			}
			//on ferme le coté du tube inutile
			close(T[i][1]);
			//on lit ce qu'il y a dans le tube
			while(read(T[i][0], &tampon, sizeof(int)) != 0){
				intervalle[compteur] = tampon;
				compteur++;
			}
			
			//on cherche et affiche les nombre premier
			for(int temp = intervalle[0]; temp <= intervalle[1]; temp++){
				if(premier(temp) == 1) printf("%d\n", temp);
			}
			
			//on ferme le côté du tube qui vient d'être utilisé	
			close(T[i][0]);
			
			exit(0);
		}
		else{
			//on créer l'intervalle dans le père
			int intervalle[2];
			if(i == 0){
				intervalle[0] = 2;
				intervalle[1] = 100;
			}else{
				intervalle[0] = (i * 100) + 1;
				intervalle[1] = (i + 1) * 100;
			}
			
			close(T[i][0]);
			write(T[i][1], &intervalle, 2*sizeof(int));
			close(T[i][1]);
		}
	}
	for(i=0; i<10; i++){
		wait(NULL);
	}
}
