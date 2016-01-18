//
// Created by Anis Da Silva Campos on 18/01/2016.
//

#ifndef IRCCHAT_PROTOCOL_H
#define IRCCHAT_PROTOCOL_H

#define SERVER_PORT 1500
#define MAX_MSG 1024
#define FREQ_HEART 30

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
    Connectok,
    Connectuserrefuse,
    Connectnumberrefuse,
    Join,
    Joinok,
    Joinrefuse,
    Disconnect,
    Leave,
    Liste,
    Say,
    Sayok,
    Sayerror,
    Echo,
    Errorcommande,
    Heartbeat
};



#endif //IRCCHAT_CHAT_H
