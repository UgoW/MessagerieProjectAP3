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
#include "protocol.h"

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Prototypes for server functions

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


#endif //MESSAGERIEPROJECTAP3_SERVER_H
