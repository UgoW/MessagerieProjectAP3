//
// Created by Ugo WAREMBOURG on 08/12/2024.
//

#ifndef MESSAGERIEPROJECTAP3_PROTOCOL_H
#define MESSAGERIEPROJECTAP3_PROTOCOL_H

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

//

typedef struct {
    char creator[50];
    char name[50];
} Channel;

// List of channels
typedef struct {
    Channel channels[50];
    int channel_count;
    int length;

} ChannelList;

Channel channels[50];
int channel_count = 0;



typedef struct {
    int socket;
    char username[50];
    char ip_address[INET_ADDRSTRLEN];
    int port;
    int in_messaging_mode;
    char channel[50];
} Client;

typedef struct {
    char sender[50];
    char message[BUFFER_SIZE];
    char destination[50];
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
    ClientLIST,
    ChannelLIST,
    MESSAGE,
    STATE,
} DATA_TYPE;

typedef union {
    ClientList clientList;
    Message  message;
    int state;
    ChannelList channelList;
} Data;

typedef struct {
    DATA_TYPE type;
    Data data;
} DataPacket;

#endif //MESSAGERIEPROJECTAP3_PROTOCOL_H
