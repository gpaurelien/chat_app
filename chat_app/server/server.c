#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7118
#define BACKLOG 5

int main() {
    int server_fd, new_socket;  // 'server_fd' represents the server socket

    server_fd = socket()
    if (server_fd <= 0) {
        perror("An error occurs when creating the socket")
        exit(1)
    }
}
