/* Client application */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>

#define SIZE_BUF 256

unsigned short fl_serv_cnt, fl_receive_msg;
int sockfd;
pthread_t pid_receive_msg;
struct sockaddr_un serv_addr;

void* serv_cnt(void*);
void* receive_msg(void*);

int main(int argc, char* argv[]){
    printf("***CLIENT***\n");
    fl_serv_cnt = fl_receive_msg = 0;
    pthread_t pid_serv_cnt;
    sockfd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("CLIENT: ERROR not open reciving socket\n");
    }
   // fcntl(sockfd, F_SETFL,O_NONBLOCK);
    serv_addr.sun_family = AF_LOCAL;
    strcpy(serv_addr.sun_path,"/tmp/socket");
    int st_serv_cnt = pthread_create(&pid_serv_cnt, NULL, serv_cnt, NULL);

    getchar();
    fl_serv_cnt = fl_receive_msg = 1;
    pthread_join(pid_serv_cnt, NULL);
    //pthread_join(pid_receive_msg, NULL);
    fcntl(sockfd, F_SETFL, O_ASYNC);
    close(sockfd);

return 0;
}

void* serv_cnt(void* arg){
    while(fl_serv_cnt == 0){
        if(connect(sockfd, &serv_addr, SUN_LEN(&serv_addr)) < 0){
            printf("CLIENT: ERROR not connecting to server\n");
            sleep(1);
        } else {
            pthread_create(&pid_receive_msg, NULL, receive_msg, NULL);
            return NULL;
        }
    }
}

void* receive_msg(void* arg){
    char recv_msg[SIZE_BUF];
    while(fl_receive_msg == 0){
        //bzero(recv_msg, SIZE_BUF);
        if(read(sockfd, (void*) &recv_msg, sizeof(recv_msg)) > 0){
            printf("%s\n", recv_msg);
        } else {
            printf("CLIENT: ERROR server was shutdown\n");
            return NULL;
        }
    }
}
