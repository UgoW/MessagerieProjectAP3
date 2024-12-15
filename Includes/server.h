//
// Created by Ugo WAREMBOURG on 08/12/2024.
//

#ifndef MESSAGERIEPROJECTAP3_SERVER_H
#define MESSAGERIEPROJECTAP3_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#include "protocol.h"

Channel channels[50];
int client_count = 0;
int channel_count = 0;
ClientNode *head = NULL;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;




typedef void (*CommandHandler)(int, char**, Message);

typedef struct {
    char* command;
    CommandHandler handler;
} Command;


/**
 * \brief Lists all connected users.
 *
 * \param client_socket The socket of the client requesting the list.
 */
void list_users(int client_socket);

/**
 * \brief Broadcasts a message to all users in messaging mode.
 *
 * \param sender The username of the sender.
 * \param message The message to be broadcasted.
 */
void broadcast_message(const char *sender, const char *message);

/**
 * \brief Handles communication with a connected client.
 *
 * \param arg Pointer to the client socket.
 * \return NULL
 */
void *handle_client(void *arg);

/**
 * \brief Handles a command received from a client. (/join)
 *
 * \param client_socket The socket of the client sending the command.
 * \param args The arguments of the command.
 * \param message The message containing the command.
 */
void handle_join(int client_socket, char** args, Message message);

/**
 * \brief Handles a command received from a client. (/broadcast)
 *
 * \param client_socket The socket of the client sending the command.
 * \param args The arguments of the command.
 * \param message The message containing the command.
 */
void handle_broadcast(int client_socket, char** args, Message message);

/**
 * \brief Handles a command received from a client. (/list_channels)
 *
 * \param client_socket The socket of the client sending the command.
 * \param args The arguments of the command.
 * \param message The message containing the command.
 */
void handle_list_channels(int client_socket, char** args, Message message);

/**
 * \brief Handles a command received from a client. (/create)
 *
 * \param client_socket The socket of the client sending the command.
 * \param args The arguments of the command.
 * \param message The message containing the command.
 */
void handle_create(int client_socket, char** args, Message message);

/**
 * \brief Create a channel and increment the number of channels.
 *
 * \param client_socket The socket of the client sending the command.
 * \param creator The username of the creator of the channel.
 * \param name The name of the channel.
 */
void create_and_increment_channels(int client_socket, char* creator, char* name);

/**
 * \brief Handles a command received from a client. (/msg)
 *
 * \param client_socket The socket of the client sending the command.
 * \param args The arguments of the command.
 * \param message The message containing the command.
 */
void handle_msg(int client_socket, char** args, Message message);

/**
 * \brief Sends a private message to a specific user.
 *
 * \param sender The username of the sender.
 * \param message The message to be sent.
 * \param destination The username of the recipient.
 */
void send_private_message(const char *sender, const char *message, const char *destination);

/**
 * \brief Handles a command received from a client. (/exit)
 *
 * \param client_socket The socket of the client sending the command.
 * \param args The arguments of the command.
 * \param message The message containing the command.
 */
void handle_exit(int client_socket, char** args, Message message);

/**
 * \brief Handles a command received from a client. (/list)
 *
 * \param client_socket The socket of the client sending the command.
 * \param args The arguments of the command.
 * \param message The message containing the command.
 */
void handle_list(int client_socket, char** args, Message message);

/**
 * \brief Sends a state packet to the client.
 *
 * \param client_socket The socket of the client.
 * \param state The state to be sent.
 * \param message The message to be sent.
 */
void send_state_packet(int client_socket, int state, char *message);

/**
 * \brief Parses a command from the input string.
 *
 * \param input The input string containing the command.
 * \param command Pointer to store the parsed command.
 * \param args Pointer to store the parsed arguments.
 * \return The number of arguments parsed.
 */
int parse_command(const char* input, char** command, char*** args);

/**
 * \brief Frees the memory allocated for the arguments.
 *
 * \param args The arguments to be freed.
 * \param arg_count The number of arguments.
 */
void free_arguments(char** args, int arg_count);

/**
 * \brief Handles a command received from a client.
 *
 * \param client_socket The socket of the client sending the command.
 * \param message The message containing the command.
 */
void handle_command(int client_socket, Message message);

#endif //MESSAGERIEPROJECTAP3_SERVER_H