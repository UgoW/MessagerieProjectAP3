//
// Created by Ugo WAREMBOURG on 08/12/2024.
//

#ifndef MESSAGERIEPROJECTAP3_PROTOCOL_H
#define MESSAGERIEPROJECTAP3_PROTOCOL_H

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

typedef struct {
    int socket;
    char username[50];
    char ip_address[INET_ADDRSTRLEN];
    int port;
    int in_messaging_mode;
} Client;

typedef struct {
    char sender[50];
    char message[BUFFER_SIZE];
    int type;
    int length;
} Message;

typedef struct {
    int client_count;
    char title[50];
    Client clients[MAX_CLIENTS];
    int length;
} ClientList;

typedef enum {
    CLIST,
    MESSAGE,
    STATE,
} DATA_TYPE;

typedef union {
    ClientList clientList;
    Message  message;
    int state;
} Data;

typedef struct {
    DATA_TYPE type;
    Data data;
} DataPacket;

#endif //MESSAGERIEPROJECTAP3_PROTOCOL_H