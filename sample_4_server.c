/* Simple server, sending messages to clients */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/un.h>

#define MAXCNT 10


unsigned short fl_send_msg, fl_wait_cnt;
int sockfd, sockfd_cli;
struct sockaddr_un serv_addr, cli_addr;

char msg[] = "Hello world!";
pthread_t pid_send_msg;

void* send_msg(void* arg);
void* wait_cnt(void* arg);

int main(int argc, char* argv[]){
    printf("***SERVER***\n");
    fl_send_msg = fl_wait_cnt = 0;
    pthread_t pid_wait_cnt;
    sockfd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("SERVER: ERROR not open listening socket\n");
    }
    fcntl(sockfd, F_SETFL,O_NONBLOCK);
    serv_addr.sun_family = AF_LOCAL;
    strcpy(serv_addr.sun_path,"/tmp/socket");

    if (bind(sockfd, &serv_addr, SUN_LEN (&serv_addr)) < 0){
        printf("SERVER: ERROR not bind listening socket\n");
    }
    if(listen(sockfd, MAXCNT) < 0){
        printf("SERVER: ERROR listening socket not start\n");
    }
    int st_wait_cnt = pthread_create(&pid_wait_cnt, NULL, wait_cnt, NULL);

    getchar();
    fl_send_msg = fl_wait_cnt = 1;
    pthread_join(pid_wait_cnt, NULL);
    //pthread_join(pid_send_msg, NULL);
    fcntl(sockfd_cli, F_SETFL,O_ASYNC);
    close(sockfd_cli);
    close(sockfd);
    unlink("/tmp/socket");
    return 0;
}

void* send_msg(void* arg){
    while(fl_send_msg == 0){
        sleep(1);
        if(write(sockfd_cli, (void*) &msg, sizeof(msg)) < 0){
            printf("SERVER: ERROR sending message failed\n");
        } 
    
    }
}

void* wait_cnt(void* arg){
    printf("SERVER: Waiting clients ...\n");
    int clilen = sizeof(struct sockaddr_un);
    while(fl_wait_cnt == 0){
        sockfd_cli = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
        if(sockfd_cli > 0){
            printf("SERVER: Connected client\n");
            int st_send_msg = pthread_create(&pid_send_msg, NULL,
                                             send_msg, NULL);
            if (st_send_msg == 0){
                printf("SERVER: Sending data to client ...\n");
                return NULL;
            } 
        } else {
                sleep(1);
            }
    }
}
