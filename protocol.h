//
// Created by Anis Da Silva Campos on 18/01/2016.
//

#ifndef IRCCHAT_PROTOCOL_H
#define IRCCHAT_PROTOCOL_H

#define SERVER_PORT 1501
#define MAX_MSG 1024
#define FREQ_HEART 300

struct Trame {

    int ID_OP;
    int ID_SEQ;
    int NB_TRAM;
    int NUM_TRAM;
    char DATA[MAX_MSG];
    int ID_USER;
    int ID_SALON;

};
typedef struct Trame Trame;


enum ID_OP{
    Connect,
    ConnectOk,
    ConnectUserRefuse,
    ConnectNumberRefuse,
    Join,
    JoinOk,
    JoinRefuse,
    Disconnect,
    Leave,
    Liste,
    Say,
    SayOk,
    SayError,
    Echo,
    ErrorCommande,
    HeartBeat,
    LeaveOk,
    Verify
};

#endif //IRCCHAT_CHAT_H
