#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#define MY_MSG_SIZE 128
#define MAX_NAME 64

key_t key;
int shmid;
int semid;
struct sembuf sb;

typedef struct Post{		
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

int main(int argc, char *argv[]){
	if(argc != 3){			//sprawdzenie czy zgadza sie liczba podanych argumentow w wierszu polecen
		printf("Blad otworzenia klienta !\n");
		exit(1);
	}
	if((key = ftok(argv[1], 1)) == -1){			//generacja klucza
		printf("Blad tworzenia klucza!\n");
		exit(1);
	}
	shmid = shmget(key, 0, 0);					//pobranie segmentu pamieci wspoldzielonej
	if(shmid == -1){
		perror("blad shmget!\n");
		return -1;
	}
	sb.sem_num = 0; sb.sem_op = -1; sb.sem_flg = 0; //ustawienia semafora: numer 0, flaga 0 i operacja -1
	
	if((semid = semget(key, 1, 0)) == -1){
		perror("semget");
		exit(1);
	}
	posts = (struct Post *)shmat(shmid, (void *)0, 0);		//dolaczenie segmentu pamieci wspoldzielonej jako strukture post 
	if(posts == (struct Post *)-1){
		perror(" blad shmat!\n");
		exit(1);
	}
	printf("Twitter 2.0 wita! (wersja C)\n");
	if(posts->maxPosts == posts->postsCount){ // sprawdzamy moĹĽliwoĹ›Ä‡ dodania postĂłw
		printf("[Brak wolnych postow]\n");
	}
	printf("[Wolnych %d postow (na %d)]\n", (posts->maxPosts - posts->postsCount), posts->maxPosts);
	printf("Podaj akcje (N)owy post, (L)ike\n");
	char wybor;
	scanf("%c", &wybor);
	while (getchar() != '\n');

	printf("[Klient]: Oczekiwanie na obsĹ‚uge przez serwer...\n");	
	if(semop(semid, &sb, 1) == -1) {	// czekanie na semafor
		perror("semop");
		exit(1);
	}
	if(wybor == 'N'){ 			//utworzenie postu
	    if(posts->postsCount == posts->maxPosts){
            printf("Brak miejsca na posty: \n");
            return -1;
        }
		int i=0;
		while(i < posts->postsCount){
			if(posts[0].name[0] != '\0'){
				printf("%d.[%s] : %s [Polubienia: %d]\n", i+1, posts[i].name, posts[i].tweet, posts[i].likes);
			}
			i++;
		}
		char* text = NULL;
		size_t textSize = MY_MSG_SIZE;
		strcpy(posts[posts->postsCount].name, argv[2]);				//dodanie nazwy uzytkownika do pamieci wspoldzielonej
		printf("Napisz co ci chodzi po glowie: \n");
		getline(&text, &textSize, stdin);
		text[strlen(text)-1] = '\0';
		strcpy(posts[posts->postsCount].tweet, text);				//dodanie postu do pamieci wspoldzielonej
		posts[posts->postsCount].likes = 0;							//ustawienie liczby polubien na 0
		posts->postsCount++;										//zwiekszenie biezacej liczby postow
	}
	else if(wybor == 'L'){
		printf("Ktory wpis chcesz polubic:\n");
		int i=0;
		while(i < posts->postsCount){								//wypisanie postow
			if(posts[0].name[0] != '\0'){
				printf("%d.[%s] : %s [Polubienia: %d]\n", i+1, posts[i].name, posts[i].tweet, posts[i].likes);
			}
			i++;
		}
		int postNumber;
		scanf("%d", &postNumber);									//odczytanie numeru wpisu do polubienia
		if(postNumber > posts->postsCount || postNumber <= 0){
			printf("Brak takego posta");
			shmdt(posts);
			return -1;
		}
		posts[postNumber-1].likes++;								//polubienie wpisu
	}
	else{
		printf("Oczekiwano (N) lub (L)\n");
		return -1;
	}
	
	sb.sem_op = 1;
    if(semop(semid, &sb, 1) == -1) {								//zwolnienie osblugi przez semafor
        perror("semop");
        exit(1);
    }
	printf("[Klient]: ZakoĹ„czono oblugiwanie przez serwer\n");

	shmdt(posts);				
	printf("\n");
	return 0;
}
