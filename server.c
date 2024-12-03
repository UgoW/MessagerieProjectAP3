#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

// A supprimer :)
#include "Includes/aesEncryption.h"
#include "Includes/rsaEncryption.h"

//

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
    snprintf(buffer, sizeof(buffer), "[MESSAGING MODE]>%s: %s", sender, message);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].in_messaging_mode && strcmp(clients[i].username, sender) != 0) {
            send(clients[i].socket, buffer, strlen(buffer), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void notify_new_user(const char *username) {
    char notification[BUFFER_SIZE];
    snprintf(notification, sizeof(notification), "%s has joined the messaging mode.\n", username);
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
        strcat(list_message, "-");
        strcat(list_message, clients[i].username);
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

    getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_len);
    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_address, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    int bytes_received = recv(client_socket, username, sizeof(username) - 1, 0);
    if (bytes_received <= 0) {
        close(client_socket);
        return NULL;
    }
    username[bytes_received] = '\0';

    pthread_mutex_lock(&clients_mutex);
    strcpy(clients[client_count].username, username);
    clients[client_count].socket = client_socket;
    strcpy(clients[client_count].ip_address, ip_address);
    clients[client_count].port = client_port;
    clients[client_count].in_messaging_mode = 0;
    client_count++;
    pthread_mutex_unlock(&clients_mutex);

    printf("New connection: %s (%s:%d)\n", username, ip_address, client_port);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "/exit", 5) == 0) {
            break;
        } else if (strncmp(buffer, "/msg", 4) == 0) {
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i].socket == client_socket) {
                    clients[i].in_messaging_mode = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            notify_new_user(username);
        } else if (strncmp(buffer, "list", 4) == 0) {
            list_users(client_socket);
        } else {
            broadcast_message(username, buffer);
        }
    }

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

    // A supprimer :)

    unsigned char aes_key[AES_KEY_SIZE];
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char plaintext[] = "Hello, AES and RSA!";
    unsigned char aes_ciphertext[128];
    unsigned char aes_decrypted[128];

    generate_aes_key_and_iv(aes_key, iv);
    aes_encrypt(plaintext, strlen((char *)plaintext), aes_key, iv, aes_ciphertext);
    aes_decrypt(aes_ciphertext, strlen((char *)plaintext), aes_key, iv, aes_decrypted);
    printf("AES Decrypted: %s\n", aes_decrypted);

    // === RSA ===
    RSA *rsa_key = generate_rsa_keys();
    unsigned char rsa_ciphertext[256];
    int rsa_ciphertext_len;
    unsigned char rsa_decrypted[256];
    int rsa_decrypted_len;

    rsa_encrypt(rsa_key, plaintext, strlen((char *)plaintext), rsa_ciphertext, &rsa_ciphertext_len);
    rsa_decrypt(rsa_key, rsa_ciphertext, rsa_ciphertext_len, rsa_decrypted, &rsa_decrypted_len);
    rsa_decrypted[rsa_decrypted_len] = '\0';
    printf("RSA Decrypted: %s\n", rsa_decrypted);




    //
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating server socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(1);
    }

    printf("Server is running on port 8080...\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, (void *)&client_socket);
    }

    close(server_socket);
    return 0;
}
