#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>

#define MY_MSG_SIZE 128
#define MAX_NAME 64

key_t key;
int postKey;
int max_post;
int semid;
struct sembuf sb;

typedef struct Post{					//struktura wpisu - do uzycia jako pamiec wspoldzielona
	char name[MAX_NAME];
	char tweet[MY_MSG_SIZE];
	int likes;
	int postsCount;
	int maxPosts;
} post;
post *posts;

union semun{							
	int val;
	struct semid_ds *buf;
	short *array; 
}arg;

void sgnhandle(int signal){						//obluga sygnalow(Ctrl+C; Ctrl+Z)
	switch (signal){
		case SIGINT:
			printf("\n[Serwer]: dostalem SIGINT => koncze i sprzatam...");
			printf(" (odlaczenie: %s, usuniecie shm: %s, usuniecie sem: %s)\n",
				(shmdt(posts) == 0) ? "OK" : "blad shmdt",
				(shmctl(postKey, IPC_RMID, 0) == 0) ? "OK" : "blad shmctl",
				(semctl(semid, 0, IPC_RMID, arg) == 0) ? "OK" : "blad semctl");
			exit(0);
			break;
		case SIGTSTP:
			printf("\n______________  Twitter 2.0:  ____________\n");
		int i=0; 										
		while (i < posts->postsCount){					//wypisanie wpisow
				if (posts[0].name[0] == '\0'){			//obluga braku wpisow
					printf("Brak wpisow\n");
					break;
				}
				else{
					printf("%d.[%s] : %s [Polubienia: %d]\n", i+1, posts[i].name, posts[i].tweet, posts[i].likes);	//wypisanie wpisow
				}
				i++;
			}
		printf("\n");
	}
}

int main(int argc, char *argv[])
{
	if(argc != 3){
		printf("Blad tworzenia serwera !\n");
		sgnhandle(SIGINT);
		exit(1);
	}

	printf("[Serwer]: Twitter 2.0 (wersja C)\n");
	printf("[Serwer]: tworze klucz na podstawie pliku %s...", argv[0]);
	if((key = ftok(argv[1], 1)) == -1){   			// wygeneruj klucz IPC
		printf("Blad tworzenia klucza!\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	int n = atoi(argv[2]); 															//max wpisow				
	printf("[Serwer]: tworze segment pamieci wspolnej na %s wpisow po %db...\n", argv[2], MY_MSG_SIZE);
	postKey = shmget(key, n * sizeof(post), 0666 | IPC_CREAT | IPC_EXCL);			//identyfikator klucza pamieci wspoldzielonej
	if(postKey == -1){
		printf(" blad shmget!\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);				//identyfikator semafora
	if(semid == -1){
		perror("semget");
		exit(1);
	}
	arg.val = 1;
    if(semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
	printf(" OK (id: %d, rozmiar: %zub)\n", postKey, sizeof(postKey));
	printf("[Serwer]: dolaczam pamiec wspolna...");
	posts = (post *)shmat(postKey, (void *)0, 0);					//przylaczenie segmentu pamieci na stukture
	if(posts == (post *)-1){
		perror(" blad shmat!\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	posts->postsCount = 0; 					//ustawiamy poczatkowa wartosc ilosci postow
	posts->maxPosts = n;					//ustawiamy maksymalna wartosc postow
	if(posts->maxPosts <= 0){
		printf("Zla ilosc postow!\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	printf("dobra ilosc postow temp\n");
	printf(" OK (adres: %ld)\n", (long int)posts);
	signal(SIGTSTP, sgnhandle);				//obluga sygnalow
	signal(SIGINT, sgnhandle);
	printf("[Serwer]: nacisnij Crtl^Z by wyswietlic stan serwisu\n");
	printf("[Serwer]: nacisnij Crtl^C by zakonczyc program\n");

	while (8){
		sleep(1);
	}
	return 0;
}
