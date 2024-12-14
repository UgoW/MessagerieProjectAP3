
#include "../Includes/server.h"
#include "../Includes/debug.h"



void add_client(Client client) {
    ClientNode *new_node = (ClientNode *)malloc(sizeof(ClientNode));
    if (!new_node) {
        perror("Memory allocation failed");
        return -1;
    }
    new_node->client = client;
    new_node->next = NULL;

    pthread_mutex_lock(&clients_mutex);
    if (head == NULL) {
        head = new_node;
    } else {
        ClientNode *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
    pthread_mutex_unlock(&clients_mutex);

    DEBUG_PRINT("Added client: %s (%s:%d)\n", client.username, client.ip_address, client.port);
}

void remove_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    ClientNode *current = head;
    ClientNode *previous = NULL;

    while (current != NULL && current->client.socket != client_socket) {
        previous = current;
        current = current->next;
    }

    if (current != NULL) {
        if (previous == NULL) {
            head = current->next;
        } else {
            previous->next = current->next;
        }
        free(current);
    }
    pthread_mutex_unlock(&clients_mutex);

    DEBUG_PRINT("Removed client with socket: %d\n", client_socket);
}


void send_state_packet(int client_socket, int state, char *message) {
    DataPacket dataPacket;
    dataPacket.type = STATE;
    dataPacket.data.state.state = state;
    strncpy(dataPacket.data.state.message, message, sizeof(dataPacket.data.state.message) - 1);
    dataPacket.data.state.message[sizeof(dataPacket.data.state.message) - 1] = '\0';
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);

    DEBUG_PRINT("Sent state packet to client with socket: %d\n", client_socket);
}

void handle_list(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "List of connected users:");
    list_users(client_socket);
}

void handle_exit(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Exiting the application...");
}

void handle_msg(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Message sent.");
    send_private_message(message.sender, args[1], args[0]);
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
    DEBUG_PRINT("Sent private message to %s: %s\n", destination, message);
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

    send_state_packet(client_socket, 1, "Channel created.");
}

void handle_create(int client_socket, char** args, Message message) {
    send_state_packet(client_socket, 1, "Channel created.");
    create_and_increment_channels(client_socket, message.sender, args[0]);
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

    DEBUG_PRINT("User %s joined channel %s\n", message.sender, args[0]);

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
    dataPacket.data.clientList.client_count = 0;

    pthread_mutex_lock(&clients_mutex);
    ClientNode *current = head;
    while (current != NULL) {
        dataPacket.data.clientList.clients[dataPacket.data.clientList.client_count] = current->client;
        dataPacket.data.clientList.client_count++;
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);

    strcpy(dataPacket.data.clientList.title, "List of connected users :");
    dataPacket.data.clientList.length = sizeof(dataPacket.data.clientList);
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);

    DEBUG_PRINT("Sent list of connected users to client with socket: %d\n", client_socket);
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

    DEBUG_PRINT("Sent broadcast message from %s: %s\n", sender, message);
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

    Client new_client;
    strcpy(new_client.username, username);
    new_client.socket = client_socket;
    strcpy(new_client.ip_address, ip_address);
    new_client.port = client_port;
    new_client.in_messaging_mode = 0;

    add_client(new_client);

    DEBUG_PRINT("New client connected: %s (%s:%d)\n", username, ip_address, client_port);

    while (1) {
        Message message;
        int bytes_received = recv(client_socket, &message, sizeof(message), 0);
        if (bytes_received <= 0) {
            break;
        }

        DEBUG_PRINT("Received message from %s: %s\n", message.sender, message.message);
        handle_command(client_socket, message);
    }

    remove_client(client_socket);

    DEBUG_PRINT("Client disconnected: %s (%s:%d)\n", username, ip_address, client_port);

    close(client_socket);
    return NULL;
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

    Client new_client;
    strcpy(new_client.username, username);
    new_client.socket = client_socket;
    strcpy(new_client.ip_address, ip_address);
    new_client.port = client_port;
    new_client.in_messaging_mode = 0;

    add_client(new_client);

    DEBUG_PRINT("New client connected: %s (%s:%d)\n", username, ip_address, client_port);

    while (1) {
        Message message;
        int bytes_received = recv(client_socket, &message, sizeof(message), 0);
        if (bytes_received <= 0) {
            break;
        }

        DEBUG_PRINT("Received message from %s: %s\n", message.sender, message.message);
        handle_command(client_socket, message);
    }

    remove_client(client_socket);

    DEBUG_PRINT("Client disconnected: %s (%s:%d)\n", username, ip_address, client_port);

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

    DEBUG_PRINT("Server is listening on port 8080\n");

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
