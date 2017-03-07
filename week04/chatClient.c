#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/

int sockfd;

void rev_from_server(int signo) {
    char buff[MAXLINE];
    int n = recv(sockfd, buff, MAXLINE, 0);
    if (n > 0) {
        printf("Received from server (%d bytes), content: %s\n", n, buff);
    }

}

int main(int argc, char **argv) {

    struct sockaddr_in servaddr;
    char sendline[MAXLINE];


    //basic check of the arguments
    //additional checks can be inserted
    if (argc != 2) {
        perror("Usage: TCPClient <IP address of the server");
        exit(1);
    }

    //Create a socket for the client
    //If sockfd<0 there was an error in the creation of the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Problem in creating the socket");
        exit(2);
    }

    //Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(SERV_PORT); //convert to big-endian order

    //Connection of the client to the socket
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Problem in connecting to the server");
        exit(3);
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK | O_ASYNC))
        printf("Error in setting socket to async, nonblock mode");

    signal(SIGIO, rev_from_server);

    if (fcntl(sockfd, F_SETOWN, getpid()) < 0) {
        perror("fcntl F_SETOWN");
        exit(1);
    }

    while (1) {
        printf("Client: ");
        gets(sendline);
        send(sockfd, sendline, MAXLINE, 0);
    }
}
