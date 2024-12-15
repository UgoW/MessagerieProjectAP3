#include "../Includes/server.h"
#include "../Includes/debug.h"

void send_state_packet(int client_socket, int state, char *message) {
    DataPacket data_packet;
    data_packet.type = STATE;
    data_packet.data.state.state = state;
    strncpy(data_packet.data.state.message, message, sizeof(data_packet.data.state.message) - 1);
    data_packet.data.state.message[sizeof(data_packet.data.state.message) - 1] = '\0';
    send(client_socket, &data_packet, sizeof(data_packet), 0);

    DEBUG_PRINT("[LOG] Sent state packet to client %d: %s\n", client_socket, message);
}

void handle_list(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "List of connected users:");
    list_users(client_socket);
}

void handle_exit(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Exiting the application...");
}

void send_private_message(const char *sender, const char *message, const char *destination) {
    DataPacket data_packet;
    data_packet.type = MESSAGE;
    strncpy(data_packet.data.message.sender, sender, sizeof(data_packet.data.message.sender) - 1);
    data_packet.data.message.sender[sizeof(data_packet.data.message.sender) - 1] = '\0';
    strncpy(data_packet.data.message.message, message, sizeof(data_packet.data.message.message) - 1);
    data_packet.data.message.message[sizeof(data_packet.data.message.message) - 1] = '\0';
    strncpy(data_packet.data.message.destination, destination, sizeof(data_packet.data.message.destination) - 1);
    data_packet.data.message.destination[sizeof(data_packet.data.message.destination) - 1] = '\0';
    data_packet.data.message.length = strlen(data_packet.data.message.message);

    int client_found = 0;
    ClientNode *current = head;
    while (current != NULL) {
        if (strcmp(current->client.username, destination) == 0) {
            client_found = 1;
            send(current->client.socket, &data_packet, sizeof(data_packet), 0);
            break;
        }
        current = current->next;
    }

    if (!client_found) {
        DEBUG_PRINT("[LOG] Client %s not found.\n", destination);
    } else {
        DEBUG_PRINT("[LOG] Sent private message to %s: %s\n", destination, message);
    }
}

void handle_msg(int client_socket, char** args, Message message) {
    if (args == NULL || args[0] == NULL || args[1] == NULL) {
        send_state_packet(client_socket, 0, "Usage: /msg <destination> <message>");
        return;
    }

    char full_message[BUFFER_SIZE] = "";
    for (int i = 1; args[i] != NULL; i++) {
        strcat(full_message, args[i]);
        if (args[i + 1] != NULL) {
            strcat(full_message, " ");
        }
    }

    send_state_packet(client_socket, 1, "Message sent.");
    send_private_message(message.sender, full_message, args[0]);
}

void create_and_increment_channels(int client_socket, char* creator, char* name) {
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

    DEBUG_PRINT("[LOG] Channel created: %s\n", name);
}

void handle_create(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Channel created.");
    create_and_increment_channels(client_socket, message.sender, args[0]);
}

void handle_list_channels(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "List of channels:");
    DataPacket data_packet;
    data_packet.type = CHANNELLIST;
    data_packet.data.channelList.channel_count = channel_count;
    for (int i = 0; i < channel_count; i++) {
        data_packet.data.channelList.channels[i] = channels[i];
    }
    data_packet.data.channelList.length = sizeof(data_packet.data.channelList);
    send(client_socket, &data_packet, sizeof(data_packet), 0);
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
                ClientNode *current = head;
                while (current != NULL) {
                    if (strcmp(current->client.username, message.sender) == 0) {
                        strcpy(current->client.channel, args[0]);
                        current->client.in_messaging_mode = 1;
                        break;
                    }
                    current = current->next;
                }
                pthread_mutex_unlock(&clients_mutex);
                break;
            } else {
                send_state_packet(client_socket, 0, "Channel not found.");
            }
        }
    } else {
        send_state_packet(client_socket, 0, "Channel not found.");
    }
}

void handle_leave(int client_socket, char** args, Message message) {
    pthread_mutex_lock(&clients_mutex);
    ClientNode *current = head;
    while (current != NULL) {
        if (strcmp(current->client.username, message.sender) == 0) {
            if (current->client.in_messaging_mode == 1) {
                current->client.in_messaging_mode = 0;
                strcpy(current->client.channel, "");
                send_state_packet(client_socket, 1, "You have left the channel.");
            } else {
                send_state_packet(client_socket, 0, "You are not in a channel.");
            }
            break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}

Command command_table[] = {
        {"/list", handle_list},
        {"/exit", handle_exit},
        {"/msg", handle_msg},
        {"/create", handle_create},
        {"/list_channels", handle_list_channels},
        {"/join", handle_join},
        {"/leave", handle_leave},

};

void list_users(int client_socket) {
    DataPacket data_packet;
    data_packet.type = CLIENTLIST;
    data_packet.data.clientList.client_count = 0;
    ClientNode *current = head;
    while (current != NULL) {
        data_packet.data.clientList.clients[data_packet.data.clientList.client_count++] = current->client;
        current = current->next;
    }
    strcpy(data_packet.data.clientList.title, "List of connected users :");
    data_packet.data.clientList.length = sizeof(data_packet.data.clientList);
    send(client_socket, &data_packet, sizeof(data_packet), 0);

    DEBUG_PRINT("[LOG] Sent list of connected users to client %d\n", client_socket);
}

void broadcast_message(const char *sender, const char *message) {
    DataPacket data_packet;
    data_packet.type = MESSAGE;
    strcpy(data_packet.data.message.sender, sender);
    strcpy(data_packet.data.message.message, message);
    data_packet.data.message.length = sizeof(data_packet.data.message);
    char sender_channel[50];

    pthread_mutex_lock(&clients_mutex);
    ClientNode *current = head;
    while (current != NULL) {
        if (strcmp(current->client.username, sender) == 0) {
            strcpy(sender_channel, current->client.channel);
            break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);

    current = head;
    while (current != NULL) {
        if (current->client.in_messaging_mode && strcmp(current->client.username, sender) != 0 && strcmp(current->client.channel, sender_channel) == 0) {
            send(current->client.socket, &data_packet, sizeof(data_packet), 0);
        }
        current = current->next;
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
                ClientNode *current = head;
                while (current != NULL) {
                    if (current->client.socket == client_socket) {
                        client_index = 1;
                        break;
                    }
                    current = current->next;
                }
                pthread_mutex_unlock(&clients_mutex);

                if (client_index != -1 && current->client.in_messaging_mode == 1) {
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

void add_client(ClientNode **head, Client client) {
    ClientNode *new_node = (ClientNode *)malloc(sizeof(ClientNode));
    if (new_node == NULL) {
        perror("Memory allocation failed");
        return;
    }
    new_node->client = client;
    new_node->next = *head;
    *head = new_node;
    client_count++;
}

void delete_client(ClientNode **head, int client_socket) {
    ClientNode *current = *head;
    ClientNode *prev = NULL;
    while (current != NULL) {
        if (current->client.socket == client_socket) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            client_count--;
            return;
        }
        prev = current;
        current = current->next;
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

    Client client;
    strcpy(client.username, username);
    client.socket = client_socket;
    strcpy(client.ip_address, ip_address);
    client.port = client_port;
    client.in_messaging_mode = 0;

    pthread_mutex_lock(&clients_mutex);
    add_client(&head, client);
    pthread_mutex_unlock(&clients_mutex);

    DEBUG_PRINT("[LOG] Client connected: %s (%s:%d)\n", username, ip_address, client_port);

    while (1) {
        Message message;
        int bytes_received = recv(client_socket, &message, sizeof(message), 0);
        if (bytes_received <= 0) {
            break;
        }

        DEBUG_PRINT("[LOG] Received message from %s: %s\n", username, message.message);
        handle_command(client_socket, message);
    }

    pthread_mutex_lock(&clients_mutex);
    delete_client(&head, client_socket);
    pthread_mutex_unlock(&clients_mutex);

    DEBUG_PRINT("[LOG] Client disconnected: %s (%s:%d)\n", username, ip_address, client_port);

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

    DEBUG_PRINT("[LOG] Server started on port 8080\n");

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