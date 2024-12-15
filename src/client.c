#include "../Includes/client.h"

void print_project_logo(ClientState *state) {
    pthread_mutex_lock(&state->print_mutex);
    printf("\n");
    printf("  ____ ___  __  ____ ____  _____\n");
    printf(" / ___/ _ \\/  |/  / _  _  \\/ _  \\\n");
    printf("/ /__/ /_/ /|   / / /|  |/  __/\n");
    printf("\\___/\\___/ |__| /_/ |_||_|\\___|\n");
    printf("\n");
    printf("Welcome to COMSEC! The secure communication system.(no secure)\n\n");
    printf("Commands:\n");
    printf("  /msg [username] [content]  : Start messaging with other users.\n");
    printf("  /list  : List all connected users.\n");
    printf("  /exit  : Exit the application.\n");
    printf("  /create [channel_name] : Create a channel.\n");
    printf("  /join [channel_name] : Join a channel.\n");
    printf("  /list_channels : List all channels.\n");
    printf("  /leave : Leave the channel.\n");
    printf("\n");
    pthread_mutex_unlock(&state->print_mutex);
}

void send_message(ClientState *state, Message *msg) {
    msg->length = strlen(msg->message);
    msg->is_command = (msg->message[0] == '/') ? true : false;
    msg->type = MESSAGE;

    send(state->client_socket, msg, sizeof(Message), 0);
    pthread_mutex_lock(&state->print_mutex);
    state->command_is_received = 0;
    pthread_cond_wait(&state->list_received_cond, &state->print_mutex);
    pthread_mutex_unlock(&state->print_mutex);
}

void receive_messages(ClientState *state, char *arg) {
    DataPacket data_packet;
    while (1) {
        int bytes_received = recv(state->client_socket, &data_packet, sizeof(data_packet), 0);
        if (bytes_received <= 0) {
            break;
        }
        pthread_mutex_lock(&state->print_mutex);
        if (data_packet.type == STATE) {
            if (data_packet.data.state.state == 0) {
                printf("Command not found\n");
            }
            state->command_is_received = 1;
            pthread_cond_signal(&state->list_received_cond);
        }

        if (data_packet.type == CLIENTLIST) {
            printf("%s (%d)\n", data_packet.data.clientList.title, data_packet.data.clientList.client_count);
            for (int i = 0; i < data_packet.data.clientList.client_count; i++) {
                printf("%s\t%s:%d\n", data_packet.data.clientList.clients[i].username, data_packet.data.clientList.clients[i].ip_address,
                       data_packet.data.clientList.clients[i].port);
            }
        }

        if (data_packet.type == MESSAGE) {
            printf("\r\033[K");
            printf("[%s]> %s\n", data_packet.data.message.sender, data_packet.data.message.message);
            fflush(stdout);
        }

        if (data_packet.type == CHANNELLIST) {
            printf("%s (%d)\n", "List of channels", data_packet.data.channelList.channel_count);
            for (int i = 0; i < data_packet.data.channelList.channel_count; i++) {
                printf("%s\t%s\n", data_packet.data.channelList.channels[i].creator, data_packet.data.channelList.channels[i].name);
            }
        }

        pthread_mutex_unlock(&state->print_mutex);
    }
}

int main() {
    ClientState state = {
            .print_mutex = PTHREAD_MUTEX_INITIALIZER,
            .list_received_cond = PTHREAD_COND_INITIALIZER,
            .client_socket = 0,
            .is_list_received = 0,
            .messaging_mode = 0,
            .command_is_received = 0
    };

    struct sockaddr_in server_addr;
    char username[50];
    Message message;

    // Create a socket
    state.client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (state.client_socket < 0) {
        perror("Error creating client socket");
        return -1;
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("172.28.0.2");

    // Connect to the server
    if (connect(state.client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        return -1;
    }

    // Print the project logo
    print_project_logo(&state);

    // Get the username
    printf("Enter your username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;

    send(state.client_socket, username, strlen(username), 0);

    // Create a thread to receive messages
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, (void *)receive_messages, &state);

    while (1) {
        pthread_mutex_lock(&state.print_mutex);
        if (state.messaging_mode) {
            printf("[%s]> ", username);
        } else {
            printf("[%s@COMSEC]$ ", username);
        }
        pthread_mutex_unlock(&state.print_mutex);

        strcpy(message.sender, username);
        fgets(message.message, BUFFER_SIZE, stdin);

        // Si le message commence par /join
        if (strncmp(message.message, "/join", 5) == 0) {
            state.messaging_mode = 1;
        }
        if (strncmp(message.message, "/leave", 6) == 0) {
            state.messaging_mode = 0;
        }
        message.message[strcspn(message.message, "\n")] = 0;
        if (strlen(message.message) == 0 || strcmp(message.message, " ") == 0) {
            continue;
        }
        send_message(&state, &message);
    }
    close(state.client_socket);
    return 0;
}