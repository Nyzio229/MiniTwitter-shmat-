#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#define MY_MSG_SIZE 128
#define MAX_NAME 64

key_t key;
int kluczPost;
int max_post;

typedef struct Post{
	char name[MAX_NAME];
	char tweet[MY_MSG_SIZE];
	int likes;
	int likesCount;
	int maxPosts;
} post;
post *posts;

void sgnhandle(int signal){
	switch (signal){
	case SIGINT:
		printf("\n[Serwer]: dostalem SIGINT => koncze i sprzatam...");
		printf(" (odlaczenie: %s, usuniecie: %s)\n",
			   (shmdt(posts) == 0) ? "OK" : "blad shmdt",
			   (shmctl(kluczPost, IPC_RMID, 0) == 0) ? "OK" : "blad shmctl");
		exit(0);
		break;
	case SIGTSTP:
		printf("\n______________  Twitter 2.0:  ____________\n");
    int i=0; //WPISY
    while (i < posts->likesCount){
			if (posts[0].name[0] == '\0'){
				printf("Brak wpisow\n");
				break;
			}
			else{
				printf("%d.[%s] : %s [Polubienia: %d]\n", i+1, posts[i].name, posts[i].tweet, posts[i].likes);
			}
			i++;
		}
		printf("\n");
	}
}

int main(int argc, char *argv[])
{
	if (argc != 3){
		printf("Blad tworzenia serwera !\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	int n = atoi(argv[2]); //POBIERANIE MAX WPISOW

	printf("[Serwer]: Twitter 2.0 (wersja C)\n");
	printf("[Serwer]: tworze klucz na podstawie pliku %s...", argv[0]);
	if ((key = ftok(argv[1], 1)) == -1){   // dolacz klucz IPC
		printf("Blad tworzenia klucza!\n");
		sgnhandle(SIGINT);
		exit(1);
	}

	printf("[Serwer]: tworze segment pamieci wspolnej na %s wpisow po %db...\n", argv[2], MY_MSG_SIZE);
	kluczPost = shmget(key, n * sizeof(post), 0666 | IPC_CREAT | IPC_EXCL);

	if (kluczPost == -1){
		printf(" blad shmget!\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	printf(" OK (id: %d, rozmiar: %zub)\n", kluczPost, sizeof(kluczPost));
	printf("[Serwer]: dolaczam pamiec wspolna...");
	posts = (post *)shmat(kluczPost, (void *)0, 0);
	if (posts == (post *)-1){
		perror(" blad shmat!\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	posts->likesCount = 0; // SPRAWDZAMY POPRAWNA LICZBE POSTOW
	posts->maxPosts = n;
	if (posts->maxPosts <= 0){
		printf("Zla ilosc postow!\n");
		sgnhandle(SIGINT);
		exit(1);
	}
	printf("dobra ilosc postow temp\n");
	printf(" OK (adres: %ld)\n", (long int)posts);
	signal(SIGTSTP, sgnhandle);
	signal(SIGINT, sgnhandle);
	printf("[Serwer]: nacisnij Crtl^Z by wyswietlic stan serwisu\n");
	printf("[Serwer]: nacisnij Crtl^C by zakonczyc program\n");

	while (8){
		sleep(1);
	}
	return 0;
}
