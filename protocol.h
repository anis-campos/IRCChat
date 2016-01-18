//
// Created by Anis Da Silva Campos on 18/01/2016.
//

#ifndef IRCCHAT_PROTOCOL_H
#define IRCCHAT_PROTOCOL_H

#define SERVEUR_PORT 1500
#define MAX_MSG 1024

typedef struct Trame {

    int ID_OP;
    int ID_SEQ;
    int NB_TRAM;
    int NUM_TRAM;
    char DATA[MAX_MSG];
    int ID_USER;
    int ID_SALON;

};



#endif //IRCCHAT_CHAT_H
