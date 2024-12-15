#ifndef MESSAGERIEPROJECTAP3_CLIENT_H
#define MESSAGERIEPROJECTAP3_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "protocol.h"
#include <stdbool.h>

// Structure to hold client state
typedef struct {
    pthread_mutex_t print_mutex;
    pthread_cond_t list_received_cond;
    int client_socket;
    int is_list_received;
    int messaging_mode;
    int command_is_received;
} ClientState;

/**
 * \brief Prints the project logo to the console.
 *
 * \param state The client state.
 */
void print_project_logo(ClientState *state);

/**
 * \brief Sends a message and wait verification from the server.
 *
 * \param state The client state.
 * \param msg The message to be sent.
 */
void send_message(ClientState *state, Message *msg);

/**
 * \brief Receives messages from the server and processes them.
 *
 * \param state The client state.
 * \param arg The argument.
 */
void receive_messages(ClientState *state, char *arg);

#endif // MESSAGERIEPROJECTAP3_CLIENT_H