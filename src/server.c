
#include "../Includes/server.h"

#define DEBUG 1

void send_state_packet(int client_socket, int state) {
    DataPacket dataPacket;
    dataPacket.type = STATE;
    dataPacket.data.state = state;
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);

    if (DEBUG) {
        printf("[LOG] Send DataPacket [\n");
        printf(" \t[LOG] Type: %d\n", dataPacket.type);
        printf(" \t[LOG] State: %d\n", dataPacket.data.state);
        printf("]\n\n");
    }
}

void handle_list(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1);
    list_users(client_socket);
}

void handle_exit(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1);
}

void send_private_message(const char *sender, const char *message, const char *destination) {
    DataPacket dataPacket;
    dataPacket.type = MESSAGE;
    strcpy(dataPacket.data.message.sender, sender);
    strcpy(dataPacket.data.message.message, message);
    strcpy(dataPacket.data.message.destination, destination);
    dataPacket.data.message.length = sizeof(dataPacket.data.message);

    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, destination) == 0) {
            send(clients[i].socket, &dataPacket, sizeof(DataPacket), 0);
            if (DEBUG) {
                printf("[LOG] Send DataPacket [\n");
                printf(" \t[LOG] Type: %d\n", dataPacket.type);
                printf(" \t[LOG] Sender: %s\n", dataPacket.data.message.sender);
                printf(" \t[LOG] Message: %s\n", dataPacket.data.message.message);
                printf(" \t[LOG] Destination: %s\n", dataPacket.data.message.destination);
                printf(" \t[LOG] Length: %d\n", dataPacket.data.message.length);
                printf("]\n\n");
            }
            break;
        }
    }
}

void handle_msg(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1);
    send_private_message(message.sender, args[1], args[0]);

}

void CreateAndIncrementChannels(char* creator, char* name) {
    Channel channel;
    strcpy(channel.creator, creator);
    strcpy(channel.name, name);
    channels[channel_count] = channel;
    channel_count++;
}

void handle_create(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1);
    CreateAndIncrementChannels(message.sender, args[0]);
}

void handle_list_channels(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1);
    DataPacket dataPacket;
    dataPacket.type = ChannelLIST;
    dataPacket.data.channelList.channel_count = channel_count;
    for (int i = 0; i < channel_count; i++) {
        dataPacket.data.channelList.channels[i] = channels[i];
    }
    dataPacket.data.channelList.length = sizeof(dataPacket.data.channelList);
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);

    if (DEBUG) {
        printf("[LOG] Send DataPacket [\n");
        printf(" \t[LOG] Type: %d\n", dataPacket.type);
        printf(" \t[LOG] Channel count: %d\n", dataPacket.data.channelList.channel_count);
        printf(" \t[LOG] Channels [\n");
        for (int i = 0; i < channel_count; i++) {
            printf(" \t\t[LOG] Channel %d [\n", i);
            printf(" \t\t[LOG] Creator: %s\n", dataPacket.data.channelList.channels[i].creator);
            printf(" \t\t[LOG] Name: %s\n", dataPacket.data.channelList.channels[i].name);
            printf("\t]\n");
        }
        printf(" \t[LOG] Length: %d\n", dataPacket.data.channelList.length);
        printf("]\n\n");
    }


}

void handle_broadcast(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1);
    broadcast_message(message.sender, message.message);
}

void handle_join(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1);
    if (channel_count > 0) {
        for (int i = 0; i < channel_count; i++) {
            if (strcmp(channels[i].name, args[0]) == 0) {
                pthread_mutex_lock(&clients_mutex);
                for (int j = 0; j < client_count; j++) {
                    if (strcmp(clients[j].username, message.sender) == 0) {
                        strcpy(clients[j].channel, args[0]);
                        clients[j].in_messaging_mode = 1;
                        strcmp(message.destination, args[0]);
                        break;
                    }
                }
                pthread_mutex_unlock(&clients_mutex);
                break;
            }
            else {
                printf("Channel not found\n");
            }
        }
    }
    else {
        printf("No channels available\n");
    }


}
Command command_table[] = {
        {"/list", handle_list},
        {"/exit", handle_exit},
        {"/msg", handle_msg},
        {"/create", handle_create},
        {"/list_channels", handle_list_channels},
        {"/join", handle_join},
};

void list_users(int client_socket) {
    DataPacket dataPacket;
    dataPacket.type = ClientLIST;
    dataPacket.data.clientList.client_count = client_count;
    for (int i = 0; i < client_count; i++) {
        dataPacket.data.clientList.clients[i] = clients[i];
    }
    strcpy(dataPacket.data.clientList.title, "List of connected users");
    dataPacket.data.clientList.length = sizeof(dataPacket.data.clientList);
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);

    if (DEBUG) {
        printf("[LOG] Send DataPacket [\n");
        printf(" \t[LOG] Type: %d\n", dataPacket.type);
        printf(" \t[LOG] Client count: %d\n", dataPacket.data.clientList.client_count);
        printf(" \t[LOG] Title: %s\n", dataPacket.data.clientList.title);
        printf(" \t[LOG] Clients [\n");
        for (int i = 0; i < client_count; i++) {
            printf(" \t\t[LOG] Client %d [\n", i);
            printf(" \t\t[LOG] Username: %s\n", dataPacket.data.clientList.clients[i].username);
            printf(" \t\t[LOG] IP Address: %s\n", dataPacket.data.clientList.clients[i].ip_address);
            printf(" \t\t[LOG] Port: %d\n", dataPacket.data.clientList.clients[i].port);
            printf(" \t\t[LOG] In messaging mode: %d\n", dataPacket.data.clientList.clients[i].in_messaging_mode);
            printf(" \t\t[LOG] Channel: %s\n", dataPacket.data.clientList.clients[i].channel);
            printf("\t]\n");
        }
        printf(" \t[LOG] Length: %d\n", dataPacket.data.clientList.length);
        printf("]\n\n");
    }
}
void broadcast_message(const char *sender, const char *message) {
    DataPacket dataPacket;
    dataPacket.type = MESSAGE;
    strcpy(dataPacket.data.message.sender, sender);
    strcpy(dataPacket.data.message.message, message);
    dataPacket.data.message.length = sizeof(dataPacket.data.message);
    char sender_channel[50];

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, sender) == 0) {
            strcpy(sender_channel, clients[i].channel);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].in_messaging_mode && strcmp(clients[i].username, sender) != 0 && strcmp(clients[i].channel, sender_channel) == 0) {
            send(clients[i].socket, &dataPacket, sizeof(DataPacket), 0);
        }
    }

    if (DEBUG) {
        printf("[LOG] Send DataPacket [\n");
        printf(" \t[LOG] Type: %d\n", dataPacket.type);
        printf(" \t[LOG] Sender: %s\n", dataPacket.data.message.sender);
        printf(" \t[LOG] Message: %s\n", dataPacket.data.message.message);
        printf(" \t[LOG] Destination: %s\n", dataPacket.data.message.destination);
        printf(" \t[LOG] Length: %d\n", dataPacket.data.message.length);
        printf("]\n\n");
    }
}

int parse_command(const char* input, char** command, char*** args) {
    char* input_copy = strdup(input);
    if (!input_copy) {
        perror("Memory allocation failed");
        return -1;
    }
    char* token = strtok(input_copy, " ");
    if (token == NULL) {
        free(input_copy);
        return -1;
    }

    *command = strdup(token);

    int arg_count = 0;
    char** arguments = malloc(sizeof(char*));
    if (!arguments) {
        free(input_copy);
        return -1;
    }

    while ((token = strtok(NULL, " ")) != NULL) {
        arguments[arg_count] = strdup(token);
        arg_count++;
        arguments = realloc(arguments, sizeof(char*) * (arg_count + 1));
    }

    *args = arguments;
    free(input_copy);

    return arg_count;
}

void free_arguments(char** args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }
    free(args);
}

// Create and increment the number of channels
void handle_command(int client_socket, Message message) {
    char* command = NULL;
    char** args = NULL;

    int arg_count = parse_command(message.message, &command, &args);

    if (arg_count >= 0) {
        int handled = 0;
        for (size_t i = 0; i < sizeof(command_table) / sizeof(command_table[0]); i++) {
            if (strcmp(command, command_table[i].command) == 0) {
                command_table[i].handler(client_socket, args, message);
                handled = 1;
                break;
            }
        }

        if (!handled) {
            if (message.type == 0) {
                handle_broadcast(client_socket, args, message);
            } else {
                send_state_packet(client_socket, 0);
            }
        }

        free(command);
        free_arguments(args, arg_count);
    } else {
        send_state_packet(client_socket, 0);
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    getpeername(client_socket, (struct sockaddr *)&client_addr, &addr_len);
    char ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_address, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    char username[50];
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

    if (DEBUG) {
        printf("[LOG] New client connected: %s (%s:%d)\n", username, ip_address, client_port);
    }

    while (1) {
        Message message;
        int bytes_received = recv(client_socket, &message, sizeof(message), 0);
        if (bytes_received <= 0) {
            break;
        }

        if (DEBUG) {
            printf("[LOG] MESSAGE STRUCTURE [\n");
            printf(" \t[LOG] Received message from %s: %s\n", message.sender, message.message);
            printf(" \t[LOG] Destination: %s\n", message.destination);
            printf(" \t[LOG] Type: %d\n", message.type);
            printf(" \t[LOG] Length: %d\n", message.length);
            printf(" \t[LOG] Channel: %s\n", message.destination);
            printf("]\n");
        }
        handle_command(client_socket, message);

    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            if (DEBUG) {
                printf("[LOG] Client disconnected: %s (%s:%d)\n", username, ip_address, client_port);
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
    if (DEBUG) {
        printf("[LOG] Server is listening on port 8080\n");
    }
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
