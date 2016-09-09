/* Semaphore demonstration */

#include <pthread.h> 
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

void* first_thread(void*); 
void* second_thread(void*);

//flags of cancel thread
unsigned short flag_1, flag_2;
//semaphire
sem_t sem_1, sem_2; //semaphore id

int main(int argc, char* argv[]){
    flag_1=flag_2 = 0;
    
    sem_init(&sem_1, 0, 1);
    sem_init(&sem_2, 0, 0);

    //pid's
	pthread_t pid_1, pid_2;
	
    //create threads
    int status_th_1 = pthread_create(&pid_1, NULL,
			           second_thread, NULL);
	int status_th_2 = pthread_create(&pid_2, NULL,
                       first_thread, NULL);
    //check pthread_create status
	if(!status_th_1)
		printf("Thread 1 created ...\n");
	else
		printf("Thread 1 creating failed\n");
	
	if(!status_th_2)
		printf("Thread 2 created ...\n");
	else 
		printf("Thread 2 creating failed\n");
    
    //wait key press
    getchar(); 

    //finishing of threads
    flag_1 = flag_2 = 1;
	pthread_join(pid_1, NULL);
    pthread_join(pid_2, NULL);
    
    sem_destroy(&sem_1);
    sem_destroy(&sem_2);

    return 0;
}

void* first_thread(void* arg){
    while(flag_1 == 0){
        sem_wait(&sem_1);
        int i;
        for(i = 0; i<10; i++){
            printf("1\n");
        }
        sem_post(&sem_2);
        sleep(1);
    } 
    printf("Done first thread.\n");
}

void* second_thread(void* arg){
    while(flag_2 == 0){
        sem_wait(&sem_2);
        int i;
        for(i = 0; i<10; i++){
            printf("2\n");
        }
        sem_post(&sem_1);
        sleep(1);
    } 
    printf("Done second thread.\n");
}
