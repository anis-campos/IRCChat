//
// Created by Anis Da Silva Campos on 18/01/2016.
//

#include "protocol.h"

#ifndef IRCCHAT_CLIENT_H
#define IRCCHAT_CLIENT_H

int creerSocket(const char * adresseIp, const char* pseud);
void* heartBeats(void* arg);
int connexion();

int envoyer(Trame  trame, struct sockaddr_in addresseServeur);
int recevoir(Trame  * trame, struct sockaddr_in addresseServeur);


void traitementEnvoye();
void traitementReception(Trame trame);

int initSelect();

#endif //IRCCHAT_CLIENT_H
