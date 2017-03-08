//
// Created by SamHV on 3/8/17.
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


#define PORT 3000
#define SIZE 100
#define MAX_CLI 10


typedef enum mode_e {
    LOGIN, BROADCAST, DEST, NONE
} Mode;

typedef struct client_s {
    int socketfd;
    char name[10];
} Client;

void processMsg(Client *client, Client *clientList, char *data);

Mode getMode(char *msg);

int main(int argc, char **argv) {
    int opt = 1;
    int master, i;
    struct sockaddr_in address;
    Client clientList[MAX_CLI];
    char *welcome = "Welcome !!";
    char buffer[SIZE];


    // set of socket descriptor
    fd_set readfds;

    // init client socket to 0 so not connected
    for (i = 0; i < MAX_CLI; i++) {
        clientList[i].socketfd = 0;
        clientList[i].name[0] = '\0';
    }

    // create master socket
    if ((master = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Fail to create socket\n");
        exit(1);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if (setsockopt(master, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    // bind master socket to localhost
    if (bind(master, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Fail to bind socket\n");
        exit(1);
    }

    printf("server run on port %d\n", PORT);

    // now listen with the maximum number of pending connections is 3
    if (listen(master, 3) < 0) {
        perror("Fail to listen\n");
        exit(2);
    }

    int addrlen = sizeof(address);

    // now waiting for connections
    while (1) {
        int max_sd, activity;
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to the socket set
        FD_SET(master, &readfds);
        max_sd = master;

        // add client socket to the socket set

        for (i = 0; i < MAX_CLI; i++) {
            int cCSocket = clientList[i].socketfd;
            if (cCSocket > 0) {
                FD_SET(cCSocket, &readfds);
            }

            if (cCSocket > max_sd) {
                max_sd = cCSocket;
            }
        }

        // now wait for data from clients
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Error occurred in select\n");
        }

        // if something happened on master socket
        // it's new client
        if (FD_ISSET(master, &readfds)) {
            int new_socket;
            if ((new_socket = accept(master,
                                     (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
                perror("Error on accept");
                exit(1);
            }

            printf("New connection , socket fd is %d , ip is : %s , port : %d\n",
                   new_socket,
                   inet_ntoa(address.sin_addr),
                   ntohs(address.sin_port)
            );

            // send welcome message to the new client
            if (send(new_socket, welcome, strlen(welcome) + 1, 0) != (strlen(welcome) + 1)) {
                perror("Error on send\n");
            }

            //add new socket to array of sockets
            for (i = 0; i < MAX_CLI; i++) {
                //if position is empty
                if (clientList[i].socketfd == 0) {
                    clientList[i].socketfd = new_socket;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < MAX_CLI; i++) {
            int sd = clientList[i].socketfd;
            if (sd == 0)
                continue;


            if (FD_ISSET(sd, &readfds)) {
                int valread;
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read(sd, buffer, SIZE)) == 0) {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *) &address, (socklen_t * ) & addrlen);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    clientList[i].socketfd = 0;
                    continue;
                }

                processMsg(&clientList[i], clientList, buffer);
            }

        }
    }

    return 0;
}

void processMsg(Client *client, Client *clientList, char *data) {

    if (client == NULL || data == NULL) {
        perror("process msg client, msg = NULL");
        exit(11);
    }

    char msg[100];
    strcpy(msg, data + 1);
    switch (getMode(data)) {
        case LOGIN:
            printf("User login %s \n", msg);
            strcpy(client->name, msg);
            {
                char sendMsg[200];
                bzero(sendMsg, sizeof sendMsg);
                strcat(sendMsg, client->name);
                strcat(sendMsg, " Login successful !");
                send(client->socketfd, sendMsg, strlen(sendMsg) + 1, 0);
            }
            break;
        case BROADCAST: {
            if (strlen(client->name) == 0) {
                char *error = "Need login first";
                send(client->socketfd, error, strlen(error) + 1, 0);
                break;
            }
            int i;

            char sendMsg[200];
            bzero(sendMsg, sizeof sendMsg);
            strcat(sendMsg, client->name);
            strcat(sendMsg, " : ");
            strcat(sendMsg, msg);

            for (i = 0; i < MAX_CLI; i++) {
                if (clientList->socketfd == 0) {
                    clientList++;
                    continue;
                }

                if (strcmp(client->name, clientList->name) == 0) {
                    clientList++;
                    continue;
                }

                send(clientList->socketfd, sendMsg, strlen(sendMsg) + 1, 0);
                clientList++;
            }


        }
            break;

        case DEST: {
            if (strlen(client->name) == 0) {
                char *error = "Need login first";
                send(client->socketfd, error, strlen(error) + 1, 0);
                break;
            }
            int i;


            char destUser[20];
            sscanf(msg, "%s", destUser);

            char sendMsg[200];
            bzero(sendMsg, sizeof sendMsg);
            strcat(sendMsg, client->name);
            strcat(sendMsg, " : ");
            strcat(sendMsg, msg + strlen(destUser));

            for (i = 0; i < MAX_CLI; i++) {
                if (clientList->socketfd == 0) {
                    clientList++;
                    continue;
                }

                if(strcmp(destUser, clientList->name) != 0) {
                    clientList++;
                    continue;
                }

                send(clientList->socketfd, sendMsg, strlen(sendMsg) + 1, 0);
                break;
            }
        }
            break;
        default: {
            char *error = "Undefined protocol";
            perror(error);
            send(client->socketfd, error, strlen(error) + 1, 0);
        }

    }
}

Mode getMode(char *msg) {
    if (msg[0] == '-')
        return LOGIN;
    if (msg[0] == '+')
        return BROADCAST;
    if (msg[0] == '*')
        return DEST;
    return NONE;
}