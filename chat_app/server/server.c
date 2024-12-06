#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7118
#define BACKLOG 5

int main() {
    int server_fd, new_socket;  // 'server_fd' represents the server socket
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0)  // 0 as 3rd parameter means that the system will select a protocol that supporting our socket type 
    if (server_fd <= 0) {
        perror("An error occurs when creating the socket")
        exit(1)
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Erreur occurs when trying to bind socket to local address");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("Erreur occurs while trying to listen to PORT %d", PORT);
        close(server_fd);
        exit(1);
    }
    
    printf("Server listening to port: %d\n", PORT)

    if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0) {
        perror("Server failed to accept entry connections, retry later!");
        close(server_fd);
        exit(1);
    }

    printf("Connection accepted!\n");

    return 0;
}
