#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 7118
#define BACKLOG 5
#define BUFFER_SIZE 1024

volatile int server_running = 1; // Launching server variable in volatile type we could modify it in the diff threads

void send_http_response(int client_socket) { // Function to send an HTTP response to the client
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html lang=\"fr\">"
        "<head><title>Serveur C</title></head>"
        "<body><h1>Bienvenue sur mon serveur C multithread !</h1></body>"
        "</html>";
    
    send(client_socket, response, strlen(response), 0); // Send HTTP response to the client
}

void *connection_handler(void *socket_desc) { // Connection handler which procks every time we have a new connection in the sockets
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
    read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0); // Receive data from the client

    if (read_size > 0) {
        printf("Requête reçue :\n%s\n", buffer); // Print the received request
        if (strncmp(buffer, "GET", 3) == 0) { // Check if the request is a GET request
            send_http_response(sock); // Send an HTTP response to the client
        }
    }

    close(sock); // Close the client socket
    free(socket_desc); // Free the allocated memory for the socket
    return NULL;
}

void *shutdown_prompt(void *server_fd) { // Server prompt which stops the server.
    char command[10];
    while (server_running) { 
        printf("Tapez 'exit' pour arrêter le serveur.\n"); // Prompt the user to stop the server
        scanf("%s", command); // Read user input
        if (strcmp(command, "exit") == 0) { // If the command is 'exit'
            server_running = 0; // Set the server running variable to 0
            shutdown(*(int *)server_fd, SHUT_RDWR); // Stop input/output operations on the server socket
            close(*(int *)server_fd); // Close the server socket
            printf("Arrêt du serveur...\n"); // Print server stopping message
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
        perror("Erreur lors de la création du socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { // Bind the socket to the address and port
        perror("Erreur lors du bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, BACKLOG) < 0) { // Listen for incoming connections
        perror("Erreur lors de l'écoute");
        close(server_fd);
        exit(1);
    }

    // Serveur opening
    printf("Serveur à l'écoute sur le port : %d\n", PORT); // Print server listening message

    pthread_t prompt_thread; // Thread creation
    if (pthread_create(&prompt_thread, NULL, shutdown_prompt, (void *)&server_fd) < 0) { // Error with the creation of prompt thread
        perror("Erreur lors de la création du thread pour le prompt");
        close(server_fd);
        exit(1);
    }

    while (server_running) { // Infinite server launch while not closed with the prompt
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen); // Accept a new connection

        // If connection is not established error.
        if (new_socket < 0) {
            if (server_running) {
                perror("Erreur lors de l'acceptation de connexion");
            }
            break;
        }

        pthread_t client_thread; // Create a thread for each new connection
        int *new_sock = malloc(sizeof(int)); // Allocate memory for the socket descriptor
        *new_sock = new_socket;

        if (pthread_create(&client_thread, NULL, connection_handler, (void *)new_sock) < 0) { // Create a thread for the client
            perror("Erreur lors de la création du thread client");
            free(new_sock);
        }

        pthread_detach(client_thread); // We detach the thread to prevent accumulation.
    }

    pthread_join(prompt_thread, NULL); // Wait for the prompt thread to finish
    printf("Serveur arrêté.\n"); // Print server stopped message
    return 0;
}
