
#include "../Includes/server.h"

void list_users(int client_socket) {
    DataPacket dataPacket;
    dataPacket.type = CLIST;
    dataPacket.data.clientList.client_count = client_count;
    for (int i = 0; i < client_count; i++) {
        dataPacket.data.clientList.clients[i] = clients[i];
    }
    strcpy(dataPacket.data.clientList.title, "List of connected users");
    dataPacket.data.clientList.length = sizeof(dataPacket.data.clientList);
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);
}

void broadcast_message(const char *sender, const char *message) {
    DataPacket dataPacket;
    dataPacket.type = MESSAGE;
    strcpy(dataPacket.data.message.sender, sender);
    strcpy(dataPacket.data.message.message, message);
    dataPacket.data.message.length = sizeof(dataPacket.data.message);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].in_messaging_mode && strcmp(clients[i].username, sender) != 0) {
            send(clients[i].socket, &dataPacket, sizeof(DataPacket), 0);
        }
    }
}

void send_state_packet(int client_socket, int state) {
    DataPacket dataPacket;
    dataPacket.type = STATE;
    dataPacket.data.state = state;
    send(client_socket, &dataPacket, sizeof(DataPacket), 0);
}

int parse_command(const char* input, char** command, char*** args) {
    char* input_copy = strdup(input);
    if (!input_copy) {
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
        if ((strcmp(command, "/list") == 0 || strcmp(command, "/msg") == 0) && arg_count > 0) {
            send_state_packet(client_socket, 0);
        }
        else if (strcmp(command, "/list") == 0) {
            send_state_packet(client_socket, 1);
            list_users(client_socket);
        }
        else if (strcmp(command, "/exit") == 0) {
            send_state_packet(client_socket, 1);

        }
        else if (strcmp(command, "/msg") == 0) {
            send_state_packet(client_socket, 1);
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (strcmp(clients[i].username, message.sender) == 0) {
                    clients[i].in_messaging_mode = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
        } else if (strcmp(command, "/create") == 0) {
            if (arg_count == 1) {
                char* arg1 = args[0];
                send_state_packet(client_socket, 1);
            }
            else {
                send_state_packet(client_socket, 0);

            }
        } else if (message.type == 0) {
            send_state_packet(client_socket, 1);
            broadcast_message(message.sender, message.message);
        } else {
            send_state_packet(client_socket, 0);
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

    printf("New connection: %s (%s:%d)\n", username, ip_address, client_port);

    while (1) {
        Message message;
        int bytes_received = recv(client_socket, &message, sizeof(message), 0);
        if (bytes_received <= 0) {
            break;
        }

        printf("[LOG] Received message from %s: %s (%d)\n", message.sender, message.message, message.length);

        handle_command(client_socket, message);


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
