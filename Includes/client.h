//
// Created by Ugo WAREMBOURG on 08/12/2024.
//

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

// Sync mutex and condition
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t list_received_cond = PTHREAD_COND_INITIALIZER;
int client_socket;
int is_list_received = 0;
int messaging_mode = 0;
int command_is_received = 0;


// Function prototypes

/**
 * \brief Prints the project logo to the console.
 */
void print_project_logo();

/**
 * \brief Sends a message and wait verification from the server.
 *
 * \param socket The socket to send the message to.
 * \param msg The message to be sent.
 */
void send_message(int socket, Message *msg);

/**
 * \brief Handles the /exit command by performing necessary cleanup and exiting the client.
 *
 * \param msg The message containing the /exit command.
 */
void handle_exit(Message *msg);

/**
 * \brief Receives messages from the server and processes them.
 */
void receive_messages(char *arg);

#endif //MESSAGERIEPROJECTAP3_CLIENT_H
