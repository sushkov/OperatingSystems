/* Демонстрация обмена сообщениями между клиентами через сервер. ОС реального времени QNX */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/neutrino.h>

#define CLIENTS 10 //число клиентов

typedef struct {
	int type;
	int client_id;
	char data[255];
} message;
//0 - запрос получения id клиента
//1 - прослушивание
//2 - запрос пересылки клиенту
//3 - ответ

typedef struct { //атрибуты клиента
	pthread_t pid;
	int coid;
	int id;
	pthread_barrier_t barier;
} c_attr;

int chid; //id канала
c_attr clients[CLIENTS];
pthread_barrier_t clients_ready, server_ready, clients_terminated;

void* server(void* data){
	int rcvid, client_id;
	message rcmsg;
	chid = ChannelCreate(0); //Создаем канал для взаимодействия с клиентами
	if(chid == -1){
		printf("[Server] Error create channel\n");
		return EXIT_FAILURE;
	}
	printf("[Server] Started OK\n");
	pthread_barrier_wait(&server_ready);
    while(1) {
    	rcvid = MsgReceive(chid, &rcmsg, sizeof(rcmsg), NULL); //Блокируем сервер пока не пришло сообщение от клиента
    	if(rcvid == -1){
    		printf("[Server] Error receive message\n");
    		continue;
    	}
    	switch(rcmsg.type){//Обрабатываем сообщение
    		case 0:
    			rcmsg.client_id = rcvid;
    			rcmsg.type = 3;
    			strcpy(rcmsg.data, "New client id");
    			if(MsgReply(rcvid, 0, &rcmsg, sizeof(rcmsg)) == -1){//Передаем сообщение-ответ клиенту
    				printf("[Server] Error reply message\n");
    			}
    			break;
    		case 1:
    			//Слушаем клиента (не отвечаем). Отвечаем по этому соединению, только когда пришло сообщение от другого клиента ему.
    			break;
    		case 2:
				rcmsg.type = 2;
				if(MsgReply(rcmsg.client_id, 0, &rcmsg, sizeof(rcmsg)) == -1){
					printf("[Server] Error reply message\n");
				}
				printf("[Server] Transfered message to client with id = %d\n", rcmsg.client_id);
				rcmsg.type = 3;
				strcpy(rcmsg.data, "Message sent to client");
				if(MsgReply(rcvid, 0, &rcmsg, sizeof(rcmsg)) == -1){
					printf("[Server] Error reply message\n");
				}
				break;
    	}
   }
   return EXIT_SUCCESS;
}
void* client_listener(int index){
	message smsg, rmsg;
	smsg.type = 0;
	strcpy(smsg.data, "");
	if(MsgSend(clients[index].coid, &smsg, sizeof(smsg), &rmsg, sizeof(rmsg)) == -1){ //Отправляем сообщение серверу и блокируем поток до получения ответа
		printf("[Client %d] Error send message\n", index);
	}
	clients[index].id = rmsg.client_id;
	pthread_barrier_wait(&clients[index].barier); //Сообщаем, что слушающий поток готов
	smsg.type = 1;
	do {
		if(MsgSend(clients[index].coid, &smsg, sizeof(smsg), &rmsg, sizeof(rmsg)))
			break;
		printf("[Client %d] Received message: %s\n", index, rmsg.data);
	} while(1);
	return EXIT_SUCCESS;
}
void* client(int index){
	message smsg, rmsg;
	int msg_result;
	pthread_barrier_t listener_ready;
	clients[index].barier = listener_ready;
	pthread_barrier_init(&clients[index].barier, NULL, 2);
	clients[index].coid = ConnectAttach(0, getpid(), chid, 0, 0);//Создаем соединение к каналу сервера
	if(clients[index].coid == -1){
		printf("[Client %d] Connection error\n", index);
	}
	pthread_t listener_pid;
	pthread_create(&listener_pid, NULL, &client_listener, index);
	pthread_barrier_wait(&clients[index].barier); //Ждем пока слушающий поток будет готов
	printf("[Client %d] started (id = %d)\n", index, clients[index].id);

	pthread_barrier_wait(&clients_ready); //Ждем пока все клиенты инициализируются

	sleep(2 + index);
	smsg.type = 2;
	smsg.client_id = clients[(index + 1) % CLIENTS].id;

	char msg[20];
	sprintf(msg, "Message for client %d", (index + 1) % CLIENTS);

	strcpy(smsg.data, msg);

	printf("[Client %d] to client %d: %s\n", index, (index + 1) % CLIENTS, smsg.data);
	if(MsgSend(clients[index].coid, &smsg, sizeof(smsg), &rmsg, sizeof(rmsg)) == -1){
		printf("[Client %d] Error send message\n", index);
	}

	pthread_barrier_wait(&clients_terminated);
	pthread_join(listener_pid, NULL);
	return EXIT_SUCCESS;
}
int main(int argc, char* argv[]){
	pthread_t server_pid;
	pthread_barrier_init(&clients_ready, NULL, CLIENTS);
	pthread_barrier_init(&clients_terminated, NULL, CLIENTS + 1);
	pthread_barrier_init(&server_ready, NULL, 2);
	pthread_create(&server_pid, NULL, &server, NULL);
	pthread_barrier_wait(&server_ready); //Ждем пока сервер запустится, только потом стартуем клиентов
	int i;
	for(i = 0; i < CLIENTS; i++){
		c_attr attr;
		clients[i] = attr;
		pthread_create(&clients[i].pid, NULL, &client, i);
	}

	pthread_barrier_wait(&clients_terminated);

	//for(i = 0; i < CLIENTS; i++){
	//	pthread_join(clients[i].pid, NULL);
	//}
	//pthread_join(server_pid, NULL);
	return EXIT_SUCCESS;
}
