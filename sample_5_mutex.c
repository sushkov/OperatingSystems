#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <queue.h>
#include <semaphore.h>
#include <unistd.h>

#define STACK_MAX_SIZE 20
#define MAX_STACK_VALUE 666
#define STACK_OVERFLOW  -100
#define STACK_UNDERFLOW -101

typedef int element;
typedef struct {
    element data[STACK_MAX_SIZE];
    size_t size;
} Stack;
void push(Stack *stack, const element value){
    if (stack->size >= STACK_MAX_SIZE) {
    	printf("stack_overflow\n");
    }
    stack->data[stack->size] = value;
    stack->size++;
}
element pop(Stack *stack) {
    if (stack->size <= 0) {
    	printf("stack_underflow\n");
        return STACK_UNDERFLOW;
    }
    stack->size--;
    return stack->data[stack->size];
}
element peek(const Stack *stack) {
    if (stack->size <= 0) {
        printf("stack_underflow\n");
    	return STACK_UNDERFLOW;
    }
    return stack->data[stack->size - 1];
}

void* consumer_1(void* arg);
void* producer_1(void* arg);
void* consumer_2(void* arg);
void* producer_2(void* arg);
void task_1();
void task_2();

unsigned int cancel_1, cancel_2;

int main(int argc, char *argv[]){
	int ch = 0;
	printf("Enter task number: ");
	scanf("%d",&ch);
	printf("\n");
	switch(ch){
	case 1:
		task_1();
		break;
	case 2:
		task_2();
		break;
	default:
		printf("Task not found. Bye.\n");
	}
	return EXIT_SUCCESS;
}

//task 1
/**
 * Решение задачи "Производитель-Потребитель", использующее 3 семафора
 * Двоичный семафор access_sem используется для организации
 * согласованного доступа к критическим данным,
 * счетные семафоры Full и Empty используются для сигнализации ожидающим потокам
 * о наступлении ожидаемого события (появлении заполненной или пустой записи соответственно).
 */

sem_t access_sem;
sem_t empty;
sem_t full;

Stack storage;

void task_1(){
	cancel_1 = cancel_2 = 0;
	storage.size = 0;

	// Инициализация семафоров
	// int sem_init(sem_t *sem, int pshared, int value);
	sem_init(&access_sem, 0, 1); // управляет доступом к разделяемым данным
	sem_init(&empty, 0, STACK_MAX_SIZE); // количество пустых записей
	sem_init(&full, 0, 0); // количество заполненых записей

	//pid's
	pthread_t producer_pid;
	pthread_t consumer_pid;

	//Создаем нити
	// int pthread_create(pthread_t *thread, pthread_attr_t *attr, void * (*start_routine)(void *), void *arg);
	pthread_create(&producer_pid, NULL, &producer_1, NULL);
	pthread_create(&consumer_pid, NULL, &consumer_1, NULL);

	sleep(60);

	// Ждем завершения нитей
	// int pthread_join (pthread_t thread, void **status_addr)
	cancel_1 = cancel_2 = 1;
	pthread_join(producer_pid, NULL);
    pthread_join(consumer_pid, NULL);
}

void* producer_1(void* arg){
	printf("Thread producer_1 (%d) started. \n", pthread_self());
	element push_value = 0;
	while(cancel_1 == 0) {

		// int sem_wait(sem_t *sem);
		// Значение семафора > 0 => уменьшаем на 1
		// Значение семафора = 0 => ждем, пока станет больше 0 (становится в очередь к семафору)

		sem_wait(&empty); // ждем появления свободной записи
		sem_wait(&access_sem); // блокируем доступ остальных потоков

		if(push_value < MAX_STACK_VALUE){
			push(&storage, push_value);
			printf("push: %d \n", peek(&storage));
		} else {
			push_value = 0;
		}
		push_value++;

		// int sem_post(sem_t *sem);
		// Есть потоки в очереди семафора => делаем поток из очереди к семафору работоспособным
		// Нет потоков в очереди семафора => увеличиваем на 1

		sem_post(&access_sem); // разблокируем доступ для остальных потоков
		sem_post(&full); // сигнализируем о появлении заполненной записи

		sleep(1);

	}
	printf("Thread producer_1 (%d) stop. \n", pthread_self());
	return EXIT_SUCCESS;
}

void* consumer_1(void* arg) {
	printf("Thread consumer_1 (%d) started. \n", pthread_self());
	while(cancel_2 == 0){

		sleep(5);

		sem_wait(&full); // ждем появления заполненной записи
		sem_wait(&access_sem); // блокируем доступ остальных потоков

		printf("pop: %d \n", pop(&storage));

		sem_post(&access_sem); // разблокируем доступ для остальных потоков
		sem_post(&empty); // сигнализируем о появлении пустой записи
	}
	printf("Thread consumer_1 (%d) stop. \n", pthread_self());
	return EXIT_SUCCESS;
}

//task 2
/**
 * Решение задачи "Производитель-Потребитель", использующее один семафор и две условные переменные.
 * Двоичный семафор access_sem используется для организации
 * согласованного доступа к критическим данным,
 * условные переменные используется для сигнализации ожидающим потокам
 * о наступлении ожидаемого события.
*/

pthread_mutex_t mutex; // дескриптор мьютекса
pthread_cond_t cond; //дескриптор условной переменной

void task_2(){
	cancel_1 = cancel_2 = 0;
	storage.size = 0;

	// инициализация мьютекса
	pthread_mutex_init(&mutex, NULL);

	// инициализация условной переменной
	pthread_cond_init(&cond, NULL);

	pthread_t producer_pid;
	pthread_t consumer_pid;

	pthread_create(&producer_pid, NULL, &producer_2, NULL);
	pthread_create(&consumer_pid, NULL, &consumer_2, NULL);

	sleep(60);

	cancel_1 = cancel_2 = 1;
	pthread_join(producer_pid, NULL);
	pthread_join(consumer_pid, NULL);
}
void* producer_2(void* arg){
	element push_value = 0;
	while(cancel_1 == 0){
		pthread_mutex_lock(&mutex); //захват мьютекса
		if (storage.size != STACK_MAX_SIZE) {
			if(push_value < MAX_STACK_VALUE){
				push(&storage, push_value);
				printf("push: %d \n", peek(&storage));
			} else {
				push_value = 0;
			}
			push_value++;
			pthread_cond_signal(&cond);
		} else {
			printf("stack is full: producer wait\n");
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex); //освобождение мьютекса
	}
	return EXIT_SUCCESS;
}

void* consumer_2(void* arg) {
	while(cancel_2 == 0){
		pthread_mutex_lock(&mutex);
		if (storage.size != 0) {
			printf("pop: %d \n", pop(&storage));
			pthread_cond_signal(&cond);
		} else {
			printf("stack is empty: consumer wait\n");
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);

	}
	return EXIT_SUCCESS;
}
