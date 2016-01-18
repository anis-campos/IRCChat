#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "protocol.h"

#define SERVER_PORT 1500
#define MAX_MSG 80
#define FREQ_HEART 30


int sd;
struct sockaddr_in serv_addr;
int idUser;
char pseudo[15];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int creerSocket(const char * adresseIp, const char* pseud);
void* heartBeats(void* arg);
int connexion();

int main (int argc, char *argv[])
{
    int sd;

    pthread_t threadHeartBeat;

    
    char addresseIP[20];

    //Accueil: config user & ip
    system("clear");
    printf("\n\n         ///   ///////       //////\n        ///   ///    ///  ///        \n       ///   ///////      ///        //   ||_||  //\\\\ =||=\n      ///   ///   ///      ///////   \\\\   || || //~\\\\  || \n\n      ___________________________\n               CONNEXION\n      ___________________________");
    printf("\n      Pseudo: ");
    scanf("%s",pseudo);

    do{
        printf("      IP serveur: ");
        scanf("%s",addresseIP);
    }while(creerSocket(addresseIP,pseudo)==-1);

    //Lancement du heart-beat
    pthread_mutex_lock(&mutex);
    if(pthread_create(&threadHeartBeat, NULL, heartBeats, NULL) == -1) {
        perror("pthread_create threadHeartBeat");
        return EXIT_FAILURE;
    }


    if(!connexion()){
	    printf("erreur de connexion");
	    exit(0);
	    }
    
    printf("\n===// IRCChat //===========================================\n");

    /*
    for (i = 2; i < argc; i++)
    {
        if (sendto(sd, argv[i], strlen(argv[i]) + 1, 0,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        {
            perror("sendto");
            return 1;
        } else {
            printf("[%s] - Sent to %s: %s\n", argv[0], inet_ntoa(serv_addr.sin_addr), argv[i]);
            printf("En Attente de la réponse...\n");
            taille=sizeof(serv_addr);

            if (recvfrom(sd, msgbuf, MAX_MSG, 0,
                         (struct sockaddr *)&serv_addr, &taille ) == -1)
                perror("recvfrom");
            else{

                printf("\t=>Reply from %s: %s\n",

                       inet_ntoa(client_addr.sin_addr), msgbuf);
            }
        }
    }*/
    close(sd);
    return 0;
}

int connexion(){
    Trame trame;
    trame.ID_OP = Connect;
    strcpy(trame.DATA,pseudo);

    return sendto(sd, &trame , sizeof(trame) , 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
}


int creerSocket(const char * adresseIp, const char* pseud){

    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port        = htons(0);

    printf("Creation du socket vers %s\n",adresseIp);
    if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Impossible de creer le socket");
        return -1;
    }

    if (bind(sd,(struct sockaddr *)&client_addr, sizeof client_addr) == -1)
    {
        perror("bind");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    if (inet_aton(adresseIp, &(serv_addr.sin_addr)) == 0)
    {
        printf("Invalid IP address format <%s>\n",adresseIp);
        return -1;
    }
    serv_addr.sin_port = htons(SERVER_PORT);

    return 1;
}



void* heartBeats(void* arg){

    Trame trame;

    trame.ID_OP = Heartbeat;
    trame.ID_USER = idUser;

    pthread_mutex_lock(&mutex);
    printf("\nLe Coeur commence à battre^^\n");

    while(1){
        sleep(FREQ_HEART);
        sendto(sd, &trame , sizeof(trame) , 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    }
}
