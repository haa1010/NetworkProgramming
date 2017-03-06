//
// Created by SamHV on 2/15/17.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/


int main(int argc, char **argv) {

    if (argc != 2) {
        perror("Usage: TCPClient <IP address of the server");
        exit(1);
    }

    int sockFd;

    // create socket
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        perror("Problem in creating the socket");
        exit(2);
    }

    struct sockaddr_in servaddr;
    //Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(SERV_PORT); //convert to big-endian order


    //Connection of the client to the socket
    if (connect(sockFd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Problem in connecting to the server");
        exit(3);
    }

    char sendline[MAXLINE],
            recvline[MAXLINE];

    while (fgets(sendline, MAXLINE, stdin) != NULL) {

        send(sockFd, sendline, strlen(sendline), 0);

        if (recv(sockFd, recvline, MAXLINE, 0) == 0) {
            //error: server terminated prematurely
            perror("The server terminated prematurely");
            exit(4);
        }
        printf("%s", "String received from the server: ");
        fputs(recvline, stdout);
    }

    return 0;
}