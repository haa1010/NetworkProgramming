#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

#define SERV_PORT 1255
#define MAXLINE 255

int checkExist(struct sockaddr_in *arr, struct sockaddr_in target, int size);

int findAvailable(struct sockaddr_in *arr, int size);

void sendAll(int client, int sockfd, struct sockaddr_in *arr, int lenArr, char *msg, int lenMsg);

int main() {
    int sockfd, n;
    socklen_t len;
    char mesg[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    struct sockaddr_in cliarr[2];
    bzero(cliarr, sizeof(cliaddr));


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) >= 0) {
        printf("Server is running at port %d", SERV_PORT);
    } else {
        perror("bind failed");
        return 0;
    }

    for (;;) {
        len = sizeof(cliaddr);
        bzero(mesg, MAXLINE);
        printf("Receiving data ...");
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);

        int no_client;
        if ((no_client = checkExist(cliarr, cliaddr, 2)) == -1) {
            // haven't save
            no_client = findAvailable(cliarr, 2);
            memcpy(&(cliarr[no_client]), &cliaddr, sizeof(cliaddr));
        }
        // saved



        printf(mesg);
        printf("Sending data : %s", mesg);
        sendAll(no_client, sockfd, cliarr, 2, mesg, n);
        //sendto(sockfd, mesg, n, 0, (struct sockaddr *) &cliaddr, len);
    }
    close(sockfd);

    return 0;
}


int checkExist(struct sockaddr_in *arr, struct sockaddr_in target, int size) {
    int i;

    for (i = 0; i < size; i++) {
        if (memcmp(arr[i], target, sizeof(target)) == 0) {
            return i;
        }
    }

    return -1;
}

int findAvailable(struct sockaddr_in *arr, int size) {
    struct sockaddr_in _empty;
    bzero(&_empty, sizeof(_empty));


    return checkExist(arr, _empty, size);
}


void sendAll(int client, int sockfd, struct sockaddr_in *arr, int lenArr, char *msg, int lenMsg){
    struct sockaddr_in _empty;
    bzero(&_empty, sizeof(_empty));
    int i;
    for (i = 0; i < lenArr; i++) {
        if(i == client) {
            continue;
        }
        if(memcmp(&_empty, ))
        sendto(sockfd, msg, lenMsg, 0, (struct sockaddr *) &arr[i], sizeof(struct sockaddr_in));
    }
}