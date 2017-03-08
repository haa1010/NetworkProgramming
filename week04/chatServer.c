//
// Created by li on 07/03/2017.
//

#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE   1
#define FALSE  0
#define PORT 3000
#define MAX_CLI 30
#define BSIZE 1024
int main(int argc, char **argv) {
    int opt = 1;
    int master_socket; // master socket
    int i, addrlen, max_sd, activity, new_socket, sd, j, valread;
    int client_socket[MAX_CLI];
    struct sockaddr_in servaddr;
    char buffer[BSIZE]; // 1kb buffer
    char *welcome = "Welcome";
    // set of socket descriptor
    fd_set readfds;
    // time out

    // init client socket to 0 so not connected
    for (i = 0; i < MAX_CLI; i++) {
        client_socket[i] = 0;
    }

    // create master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Fail to create socket\n");
        exit(1);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // type of socket created
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // bind master socket to localhost
    if (bind(master_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Fail to bind socket\n");
        exit(1);
    }

    printf("Chat server run on port %d\n", PORT);

    // now listen with the maximum number of pending connections is 3
    if (listen(master_socket, 3) < 0) {
        perror("Fail to listen\n");
        exit(1);
    }

    addrlen = sizeof(servaddr);

    // now waiting for connections
    while (1) {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to the socket set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // add client socket to the socket set
        for (i = 0; i < MAX_CLI; i++) {
            if (client_socket[i] > 0) {
                FD_SET(client_socket[i], &readfds);
            }

            if (client_socket[i] > max_sd) {
                max_sd = client_socket[i];
            }
        }

        // now wait for data from clients
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Error occurred in select\n");
        } else if (activity == 0) {
            perror("Time out\n");
        }

        printf("activity = %d\n", activity);
        // if something happened on master socket
        // it's new client
        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *) &servaddr, (socklen_t *) &addrlen)) < 0) {
                perror("Error on accept");
                exit(1);
            }

            printf("New connection , socket fd is %d , ip is : %s , port : %d\n",
                   new_socket,
                   inet_ntoa(servaddr.sin_addr),
                   ntohs(servaddr.sin_port)
            );

            // send welcome message to the new client
            if (send(new_socket, welcome, strlen(welcome), 0) != strlen(welcome)) {
                perror("Error on send\n");
            }

            //add new socket to array of sockets
            for (i = 0; i < MAX_CLI; i++) {
                //if position is empty
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < MAX_CLI; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read(sd, buffer, BSIZE)) == 0) {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *) &servaddr, \
                        (socklen_t *) &addrlen);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_socket[i] = 0;
                }
                    //Echo back the message that came in
                else {
                    //set the string terminating NULL byte on the end
                    //of the data read
//                    char message[BSIZE];
//                    sprintf(message, "Send from %d: ", sd);
//                    strcat(message, buffer);
                    printf("\'%s\'", buffer);
                    // send to other clients
                    for (j = 0; j < MAX_CLI; j++) {
                        if (client_socket[j] <= 0 || client_socket[j] == sd) {
                            continue;
                        }
                        printf("send to %d\n", client_socket[j]);
                        send(client_socket[j], buffer, strlen(buffer) + 1, 0);
                    }
                }
            }
        }
    }
}