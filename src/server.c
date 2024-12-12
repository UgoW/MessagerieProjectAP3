
#include "../Includes/server.h"

#define DEBUG 1

void send_state_packet(int client_socket, int state, char *message) {
    DataPacket dataPacket;
    dataPacket.type = STATE;
    dataPacket.data.state.state = state;
    strncpy(dataPacket.data.state.message, message, sizeof(dataPacket.data.state.message) - 1);
    dataPacket.data.state.message[sizeof(dataPacket.data.state.message) - 1] = '\0';
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);
}

void handle_list(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "List of connected users:");
    list_users(client_socket);
}

void handle_exit(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Exiting the application...");
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
            break;
        }
    }
}

void handle_msg(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Message sent.");
    send_private_message(message.sender, args[1], args[0]);
}

void CreateAndIncrementChannels(int client_socket, char* creator, char* name) {
    int user_channel_count = 0;
    for (int i = 0; i < channel_count; i++) {
        if (strcmp(channels[i].creator, creator) == 0) {
            user_channel_count++;
        }
        if (strcmp(channels[i].name, name) == 0) {
            send_state_packet(client_socket, 0, "Channel already exists.");
            return;
        }
    }

    if (user_channel_count >= 5) {
        send_state_packet(client_socket, 0, "Maximum number of channels (5) reached.");
        return;
    }

    if (channel_count >= 50) {
        send_state_packet(client_socket, 0, "Maximum number of channels (50) reached.");
        return;
    }
    if (name == NULL || strlen(name) == 0 || strspn(name, " ") == strlen(name)) {
        send_state_packet(client_socket, 0, "Channel name cannot be empty.");
        return;
    }

    Channel channel;
    strcpy(channel.creator, creator);
    strcpy(channel.name, name);
    channels[channel_count] = channel;
    channel_count++;
}

void handle_create(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Channel created.");
    CreateAndIncrementChannels(client_socket, message.sender, args[0]);
}

void handle_list_channels(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "List of channels:");
    DataPacket dataPacket;
    dataPacket.type = CHANNELLIST;
    dataPacket.data.channelList.channel_count = channel_count;
    for (int i = 0; i < channel_count; i++) {
        dataPacket.data.channelList.channels[i] = channels[i];
    }
    dataPacket.data.channelList.length = sizeof(dataPacket.data.channelList);
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);
}

void handle_broadcast(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "");
    broadcast_message(message.sender, message.message);
}

void handle_join(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "You have joined the channel.");
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
                send_state_packet(client_socket, 0, "Channel not found.");
            }
        }
    }
    else {
        send_state_packet(client_socket, 0, "Channel not found.");
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
    dataPacket.type = CLIENTLIST;
    dataPacket.data.clientList.client_count = client_count;
    for (int i = 0; i < client_count; i++) {
        dataPacket.data.clientList.clients[i] = clients[i];
    }
    strcpy(dataPacket.data.clientList.title, "List of connected users :");
    dataPacket.data.clientList.length = sizeof(dataPacket.data.clientList);
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);
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
            if (message.type == MESSAGE && message.is_command == 0) {
                int client_index = -1;
                pthread_mutex_lock(&clients_mutex);
                for (int i = 0; i < client_count; i++) {
                    if (clients[i].socket == client_socket) {
                        client_index = i;
                        break;
                    }
                }
                pthread_mutex_unlock(&clients_mutex);

                if (client_index != -1 && clients[client_index].in_messaging_mode == 1) {
                    handle_broadcast(client_socket, args, message);
                } else {
                    send_state_packet(client_socket, 0, "You are not in a channel.");
                }
            } else {
                send_state_packet(client_socket, 0, "Command not found.");
            }
        }

        free(command);
        free_arguments(args, arg_count);
    } else {
        send_state_packet(client_socket, 0, "Invalid command.");
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
            printf(" \t[LOG] Is command: %d\n", message.is_command);
            printf(" \t[LOG] Type: %d\n", message.type);
            printf(" \t[LOG] Length: %d\n", message.length);
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
