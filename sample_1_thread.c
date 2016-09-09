/* Threads demonstration */

#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>

void* first_thread(void*); 
void* second_thread(void*);

//flags of cancel thread
unsigned short flag_1, flag_2;

int main(int argc, char* argv[]){
    flag_1=flag_2 = 0;

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

    return 0;
}

void* first_thread(void* arg){
    while(flag_1 == 0){
        printf("1\n");
        sleep(1);
    }
    printf("Done first thread.\n");
}

void* second_thread(void* arg){
	while(flag_2 == 0){
        printf("2\n");
        sleep(1);
    }
	printf("Done second thread.\n");
}
