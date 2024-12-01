#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[50];
    char ip_address[INET_ADDRSTRLEN];
    int port;
    int in_messaging_mode;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *sender, const char *message) {
    char buffer[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char timestamp[6];
    strftime(timestamp, sizeof(timestamp), "%H:%M", timeinfo);

    snprintf(buffer, sizeof(buffer), "%s %s: %s\n", timestamp, sender, message);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].in_messaging_mode) {
            send(clients[i].socket, buffer, strlen(buffer), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void notify_new_user(const char *username) {
    char notification[BUFFER_SIZE];
    snprintf(notification, sizeof(notification), "User %s has joined the messaging mode.\n", username);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].in_messaging_mode) {
            send(clients[i].socket, notification, strlen(notification), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void list_users(int client_socket) {
    char list_message[BUFFER_SIZE] = "Connected users:\n";

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        strcat(list_message, clients[i].username);
        strcat(list_message, " - ");
        strcat(list_message, clients[i].ip_address);
        strcat(list_message, ":");
        char port_str[6];
        snprintf(port_str, sizeof(port_str), "%d", clients[i].port);
        strcat(list_message, port_str);
        strcat(list_message, "\n");
    }
    pthread_mutex_unlock(&clients_mutex);

    send(client_socket, list_message, strlen(list_message), 0);
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    char username[50];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Obtenir l'adresse IP et le port du client
    getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_len);
    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_address, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    // Recevoir le nom d'utilisateur
    int bytes_received = recv(client_socket, username, sizeof(username) - 1, 0);
    if (bytes_received <= 0) {
        close(client_socket);
        return NULL;
    }
    username[bytes_received] = '\0';

    // Ajouter l'utilisateur à la liste des clients
    pthread_mutex_lock(&clients_mutex);
    strcpy(clients[client_count].username, username);
    clients[client_count].socket = client_socket;
    strcpy(clients[client_count].ip_address, ip_address);
    clients[client_count].port = client_port;
    clients[client_count].in_messaging_mode = 0;
    client_count++;
    pthread_mutex_unlock(&clients_mutex);

    // Afficher les détails de la connexion
    printf("New connection: %s (%s:%d)\n", username, ip_address, client_port);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "EXIT", 4) == 0) {
            break;
        } else if (strncmp(buffer, "/MSG", 4) == 0) {
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i].socket == client_socket) {
                    clients[i].in_messaging_mode = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            notify_new_user(username);
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            list_users(client_socket);
        } else {
            broadcast_message(username, buffer);
        }
    }

    // Retirer le client de la liste des clients
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    listen(server_socket, 5);
    printf("Server listening on port 8080...\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, &client_socket);
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}
