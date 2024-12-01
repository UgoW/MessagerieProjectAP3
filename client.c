#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/select.h>

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
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "/exit", 5) == 0) {
            printf("You have exited messaging mode.\n");
            messaging_mode = 0;
        } else {
            // Affichage immédiat du message reçu
            printf("\n%s\n[MESSAGING MODE]@> ", buffer);  // On ajoute un saut de ligne avant chaque message
            fflush(stdout);  // Assurez-vous que la sortie est immédiatement écrite
        }
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

    // Créer un thread pour recevoir les messages du serveur
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, (void *)receive_messages, NULL);

    char input[BUFFER_SIZE];
    fd_set read_fds;
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);  // Ajout de l'entrée standard (stdin) pour détecter l'entrée de l'utilisateur
        FD_SET(client_socket, &read_fds); // Ajout du socket pour détecter les messages entrants

        // Utilisation de select pour gérer les entrées et sorties simultanées
        int max_fd = (client_socket > STDIN_FILENO) ? client_socket : STDIN_FILENO;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select error");
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // L'utilisateur entre une commande ou un message
            if (messaging_mode) {
                printf("[MESSAGING MODE]@%s> ", username);
            } else {
                printf("COMSEC@$%s> ", username);
            }

            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';

            if (messaging_mode && strcmp(input, "/exit") == 0) {
                // Sortir du mode de messagerie
                messaging_mode = 0;
                send_to_server("/exit");
                printf("You have exited messaging mode.\n");
            } else if (messaging_mode) {
                // Envoi d'un message en mode messagerie
                send_to_server(input);
            } else {
                // Commande principale
                handle_command(input);
            }
        }

        if (FD_ISSET(client_socket, &read_fds)) {
            // Le serveur a envoyé un message
            // Cela est déjà géré par le thread de réception des messages, donc rien à faire ici
        }
    }

    close(client_socket);
    return 0;
}