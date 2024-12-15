#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 7118
#define BACKLOG 5

volatile int server_running = 1; // Launching server variable in volatile type we could modify it in the diff threads

void *connection_handler(void *socket_desc) { // Connection handler wich prock every time we have a new connection in the sockets
    int sock = *(int *)socket_desc;
    printf("Nouvelle connexion acceptee !\n");
    
    close(sock);
    free(socket_desc);
    return NULL;
}

void *shutdown_prompt(void *server_fd) { // server prompt wich stop the server.
    char command[10];
    while (server_running) {
        printf("Tapez 'exit' pour arreter le serveur. \n ");
        scanf("%s", command);
        if (strcmp(command, "exit") == 0) {
            server_running = 0;
            close(*(int *)server_fd);
            printf("Arret du serveur...\n");
        }
    }
    return NULL;
}

int main() {
    int server_fd, new_socket; // 'server_fd' represents the server socket
                               // new_socket represents the new entry which 
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0); // 0 as 3rd parameter means that the system will select a protocol that supporting our socket type 


    // Error part close the server when a fail occurred 
    if (server_fd <= 0) {
        perror("Erreur lors de la creation du socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erreur lors du bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("Erreur lors de l'ecoute");
        close(server_fd);
        exit(1);
    }






    // Serveur oppening 

    printf("Serveur a l'ecoute sur le port : %d\n", PORT);

    pthread_t prompt_thread; // Thread creation
    if (pthread_create(&prompt_thread, NULL, shutdown_prompt, (void *)&server_fd) < 0) { // Error with the creation of prompt thread
        perror("Erreur lors de la creation du thread pour le prompt");
        close(server_fd);
        exit(1);
    }

    while (server_running) { // infinit server launch while not closed with the prompt


        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        // If connection is not established error.
        if (new_socket < 0) {
            if (server_running) {
                perror("Erreur lors de l'acceptation de connexion");
            }
            break;
        }

        pthread_t client_thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;

        if (pthread_create(&client_thread, NULL, connection_handler, (void *)new_sock) < 0) {
            perror("Erreur lors de la creation du thread client");
            free(new_sock);
        }


        pthread_detach(client_thread); // We detach the thread to prevent accumulation.
    }


    pthread_join(prompt_thread, NULL); 
    printf("Serveur arrete.\n");
    return 0;
}
