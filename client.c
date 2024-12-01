#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 1024

int client_socket;
int messaging_mode = 0;
int welcome_message_sent = 0;  // Flag pour éviter d'envoyer le message de bienvenue plusieurs fois

void print_project_logo() {
    printf("\n");
    printf("  ____ ___  __  ____ ____  _____\n");
    printf(" / ___/ _ \\/  |/  / _  _  \\/ _  \\\n");
    printf("/ /__/ /_/ /|   / / /|  |/  __/\n");
    printf("\\___/\\___/ |__| /_/ |_||_|\\___|\n");
    printf("\n");
    printf("Welcome to COMSEC! The secure communication system.\n\n");
    printf("Commands:\n");
    printf("  /msg   : Start messaging with other users.\n");
    printf("  /list  : List all connected users.\n");
    printf("  /exit  : Exit the application.\n\n");
}

void send_to_server(const char *message) {
    send(client_socket, message, strlen(message), 0);
}

void handle_exit() {
    send_to_server("/exit\n");
    close(client_socket);
    exit(0);
}

void handle_help() {
    printf("Commands:\n");
    printf("  /msg   : Start messaging with other users.\n");
    printf("  /list  : List all connected users.\n");
    printf("  /exit  : Exit the application.\n\n");
}

void handle_list() {
    send_to_server("list\n");
}

void handle_command(const char *command) {
    if (strncmp(command, "/exit", 4) == 0) {
        handle_exit();
    }
    else if (strncmp(command, "/msg", 4) == 0) {
        if (!messaging_mode) { // Activation du mode messagerie si ce n'est pas déjà fait
            messaging_mode = 1;
            welcome_message_sent = 1;  // Assurer qu'on envoie le message de bienvenue une seule fois
            send_to_server("/msg");
            printf("Messaging mode activated. Type your message to send to all connected users.\n");
        }
    }
    else if (strncmp(command, "/list", 5) == 0) {
        handle_list();
    } else if (strncmp(command, "/help", 5) == 0) {
        handle_help();
    }
    else if (strlen(command) == 0) {
        return;
    }
    else {
        printf("Invalid command. Type /help to see the list of commands.\n");
    }

}

void receive_messages() {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    }
}


int main() {
    struct sockaddr_in server_addr;
    char username[50];

    // Connexion au serveur
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(1);
    }

    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';
    send(client_socket, username, strlen(username), 0);

    print_project_logo();

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, (void *)receive_messages, NULL);

    char input[BUFFER_SIZE];
    while (1) {
        if (messaging_mode) {
            if (!welcome_message_sent) {
                // Le message de bienvenue n'est envoyé qu'une seule fois.
                printf("User %s has joined the messaging mode.\n", username);
                welcome_message_sent = 1;
            }

            printf("[MESSAGING MODE]@%s> ", username);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';

            if (strcmp(input, "/exit") == 0) {
                // Sortir du mode de messagerie
                messaging_mode = 0;
                send_to_server("/exit");
                printf("You have exited messaging mode.\n");
            } else {
                send_to_server(input);
            }
        } else {
            // Commande principale
            printf("COMSEC@$%s> ", username);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';
            handle_command(input);
        }
    }

    close(client_socket);
    return 0;
}
