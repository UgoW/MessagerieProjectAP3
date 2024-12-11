#include "../Includes/client.h"
#include "../Includes/aesEncryption.h"
#include "../Includes/rsaEncryption.h"
void print_project_logo() {
    pthread_mutex_lock(&print_mutex);
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
    pthread_mutex_unlock(&print_mutex);
}

void send_message(int socket, Message *msg) {
    msg->length = strlen(msg->message);
    if (msg->message[0] == '/') {
        msg->type = 1;
    } else {
        msg->type = 0;
    }
    send(socket, msg, sizeof(Message), 0);
    pthread_mutex_lock(&print_mutex);
    command_is_received = 0;
    pthread_cond_wait(&list_received_cond, &print_mutex);
    pthread_mutex_unlock(&print_mutex);
}

void handle_exit(Message *msg) {
    close(client_socket);
    exit(0);
}

void receive_messages() {
    DataPacket dataPacket;
    while (1) {
        int bytes_received = recv(client_socket, &dataPacket, sizeof(DataPacket), 0);
        if (bytes_received <= 0) {
            break;
        }

        // Lock before printing
        pthread_mutex_lock(&print_mutex);


        if (dataPacket.type == STATE) {
            if (dataPacket.data.state == 0) {
                printf("Command not found\n");
            }
            command_is_received = 1;
            pthread_cond_signal(&list_received_cond);
        }

        if (dataPacket.type == CLIST) {
            printf("%s (%d)\n", dataPacket.data.clientList.title, dataPacket.data.clientList.client_count);
            for (int i = 0; i < dataPacket.data.clientList.client_count; i++) {
                printf("%s\t%s:%d\n", dataPacket.data.clientList.clients[i].username, dataPacket.data.clientList.clients[i].ip_address,
                       dataPacket.data.clientList.clients[i].port);
            }
        }

        if (dataPacket.type == MESSAGE) {
            printf("\n[%s]> %s\n", dataPacket.data.message.sender, dataPacket.data.message.message);
        }

        pthread_mutex_unlock(&print_mutex);

    }
}

int main() {
    struct sockaddr_in server_addr;
    char username[50];
    Message message;

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        printf("Error while creating socket\n");
        exit(1);
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("172.28.0.2");

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error while connecting to server\n");
        exit(1);
    }

    // Print the project logo
    print_project_logo();

    // Get the username
    printf("Enter your username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;

    send(client_socket, username, strlen(username), 0);

    // Create a thread to receive messages
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, (void *)receive_messages, NULL);

    while (1) {
        pthread_mutex_lock(&print_mutex);

        if (messaging_mode) {
            printf("#> ", username);
        } else {
            printf("[%s@COMSEC]$ ", username);
        }

        pthread_mutex_unlock(&print_mutex);

        strcpy(message.sender, username);
        fgets(message.message, BUFFER_SIZE, stdin);
        message.message[strcspn(message.message, "\n")] = 0;

        send_message(client_socket, &message);
    }
    close(client_socket);
    return 0;
}