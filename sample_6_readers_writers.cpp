/* Задача читателей и писателей */

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_READERS 5 //число потоков чтения
#define MAX_WRITERS 5 //число потоков записи

using namespace std;

/*Класс Сигнал - обертка для работы с условными переменными*/
class Signal{
	private:
		pthread_cond_t cond;
		int queue_count;
	public:
		Signal(){
			pthread_cond_init(&cond,NULL);
			queue_count = 0;
		}
		void wait(pthread_mutex_t* mut){
			queue_count++;
			pthread_cond_wait(&cond, mut);
		}
		void send(){
			queue_count--;
			pthread_cond_signal(&cond);
		}
		int getQueueCount(){
			return queue_count;
		}
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Signal* access_read; //условная переменная чтения
Signal* access_write; //условная переменная записи
unsigned int readers = 0; //текущее число читателей
bool writing = false; //кто-то пишет?
pthread_barrier_t threads_ready;

void* reader(void* data){
	pthread_barrier_wait(&threads_ready);
	usleep(1);
	while(1){
		pthread_mutex_lock(&mutex);
		if(writing || access_write->getQueueCount() > 0){ //если кто-то уже пишет или писатели в очереди
			access_read->wait(&mutex); //cтановимся в очередь читателей
		}
		readers++; //начинаем читать
		cout << "reading.. (pid: " << pthread_self() << ")" << endl;
		readers--; //заканчиваем читать
		pthread_mutex_unlock(&mutex);
		usleep(1000);
	}
	return 0;
}
void* writer(void* data){
	pthread_barrier_wait(&threads_ready);
	while(1){
		pthread_mutex_lock(&mutex);
		if(writing){ //если кто-то уже пишет
			access_write->wait(&mutex);//становимся в очередь писателей
		}

		writing = true; //иначе, сообщаем что мы пишем
		cout << "writing.. (pid: " << pthread_self() << ")" << endl;
		writing = false; //сообщаем, что мы закончили писать

		if(access_write->getQueueCount() == 0){ //если нет писателей в очереди
			if(access_read->getQueueCount() > 0) //если в очереди читателей кто-то есть
				access_read->send(); //открываем доступ потокам из очереди читателей
		}
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	return 0;
}

int main(int argc, char *argv[]){
	pthread_t readers_pids[MAX_READERS], writers_pids[MAX_WRITERS];
	access_read = new Signal();
	access_write = new Signal();
	pthread_mutex_init(&mutex,NULL);
	pthread_barrier_init(&threads_ready, NULL, MAX_WRITERS + MAX_READERS);
	for(int i = 0; i < MAX_READERS; i++){
		pthread_create(&readers_pids[i], NULL, &reader, NULL);
	}
	for(int i = 0; i < MAX_WRITERS; i++){
		pthread_create(&writers_pids[i], NULL, &writer, NULL);
	}
	for(int i = 0; i < MAX_READERS; i++){
		pthread_join(readers_pids[i], NULL);
	}
	for(int i = 0; i < MAX_WRITERS; i++){
		pthread_join(writers_pids[i], NULL);
	}
	return 0;
}
