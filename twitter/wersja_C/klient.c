#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define MY_MSG_SIZE 128
#define MAX_NAME 64

key_t key;
int shmid;

typedef struct Post{
	char name[MAX_NAME];
	char tweet[MY_MSG_SIZE];
	int likes;
	int likesCount;
	int maxPosts;
} post;
post *posts;

int main(int argc, char *argv[]){
	if (argc != 3){// ILOSC ARGUMENTOW
		printf("Blad otworzenia klienta !\n");
		exit(1);
	}
	if ((key = ftok(argv[1], 1)) == -1){
		printf("Blad tworzenia klucza!\n");
		exit(1);
	}
	shmid = shmget(key, 0,0);
	if (shmid == -1){
		perror("blad shmget!\n");
		return -1;
	}
	posts = (struct Post *)shmat(shmid, (void *)0, 0);
	if (posts == (struct Post *)-1){
		perror(" blad shmat!\n");
		exit(1);
	}
	printf("Twitter 2.0 wita! (wersja C)\n");
	if (posts->maxPosts == posts->likesCount){ // SPRAWDZAMY MOZLIWOSC DODANIA POSTOW
		printf("[Brak wolnych postow]\n");
	}
	printf("[Wolnych %d postow (na %d)]\n", (posts->maxPosts - posts->likesCount), posts->maxPosts);
	printf("Podaj akcje (N)owy post, (L)ike\n");
	char wybor;
	scanf("%c", &wybor);
	while (getchar() != '\n');
	if (wybor == 'N'){ // DODAWANIE POSTU
	    if(posts->likesCount == posts->maxPosts){
            printf("Brak miejsca na posty: \n");
            return -1;
        }
		int i=0;
		while (i < posts->likesCount){
			if (posts[0].name[0] != '\0'){
				printf("%d.[%s] : %s [Polubienia: %d]\n", i+1, posts[i].name, posts[i].tweet, posts[i].likes);
			}
			i++;
		}
		char* text = NULL;
		size_t textSize = MY_MSG_SIZE;
		strcpy(posts[posts->likesCount].name, argv[2]);
		printf("Napisz cos: \n");
		getline(&text, &textSize, stdin);
		text[strlen(text)-1] = '\0';
		strcpy(posts[posts->likesCount].tweet, text);
		posts[posts->likesCount].likes = 0;
		posts->likesCount++;
	}
	else if (wybor == 'L'){
		printf("Ktory wpis chcesz polubic \n");
		int i=0;
		while (i < posts->likesCount){
			if (posts[0].name[0] != '\0'){
				printf("%d.[%s] : %s [Polubienia: %d]\n", i+1, posts[i].name, posts[i].tweet, posts[i].likes);
			}
			i++;
		}
		int numer_postu;
		scanf("%d", &numer_postu);
		if(numer_postu > posts->likesCount || numer_postu <= 0){
			printf("Brak takego posta");
			shmdt(posts);
			return -1;
		}
		posts[numer_postu-1].likes++;
	}
	else{
		printf("Oczekiwano (N) lub (L)\n");
		exit(1);
	}
	shmdt(posts);
	printf("\n");
	return 0;
}
