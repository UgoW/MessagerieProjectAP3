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

// Sync mutex and condition
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t list_received_cond = PTHREAD_COND_INITIALIZER;
int client_socket;
int is_list_received = 0;
int messaging_mode = 0;
int is_message_sent = 0;

// Function prototypes

/**
 * \brief Prints the project logo to the console.
 */
void print_project_logo();

/**
 * \brief Sends a message to the specified socket.
 *
 * \param socket The socket to send the message to.
 * \param msg The message to be sent.
 */
void send_message(int socket, Message *msg);

/**
 * \brief Handles the /list command by sending a request to the server to list all users.
 *
 * \param msg The message containing the /list command.
 */
void handle_list_command(Message *msg);

/**
 * \brief Handles the /exit command by performing necessary cleanup and exiting the client.
 *
 * \param msg The message containing the /exit command.
 */
void handle_exit(Message *msg);

/**
 * \brief Receives messages from the server and processes them.
 */
void receive_messages();

/**
 * \brief Handles the /msg command by switching the client to messaging mode.
 *
 * \param msg The message containing the /msg command.
 */
void handle_msg_command(Message *msg);

#endif //MESSAGERIEPROJECTAP3_CLIENT_H
