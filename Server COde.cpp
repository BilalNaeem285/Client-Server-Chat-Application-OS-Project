// server.c — Multithreaded Chat Server in C (Linux)
// Compile: gcc server.c -o server -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t lock;

// Broadcast a message to all clients except the sender
void broadcast(char *msg, int sender) {
    pthread_mutex_lock(&lock);

    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

// Thread function to handle each client
void *handle_client(void *client_socket) {
    int sock = *((int *)client_socket);
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            printf("A client disconnected.\n");
            close(sock);
            break;
        }

        printf("Client %d: %s", sock, buffer);
        broadcast(buffer, sock);
    }

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    pthread_t thread;

    pthread_mutex_init(&lock, NULL);

    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("Socket failed"); exit(1); }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Listen for connections
    listen(server_fd, 10);
    printf("Server started. Listening on port %d...\n", PORT);

    // Accept multiple clients
    while (1) {
        addr_size = sizeof(client_addr);
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        printf("Client connected: %d\n", new_socket);

        pthread_mutex_lock(&lock);
        clients[client_count++] = new_socket;
        pthread_mutex_unlock(&lock);

        // Create thread for this client
        pthread_create(&thread, NULL, handle_client, (void *)&new_socket);
        pthread_detach(thread);
    }

    return 0;
}
