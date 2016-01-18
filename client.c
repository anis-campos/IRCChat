#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "client.h"



int sd;
struct sockaddr_in serv_addr;
int idUser;
char pseudo[15];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



int main (int argc, char *argv[])
{
    int sd;

    pthread_t threadHeartBeat;


    char addresseIP[20];

    //Accueil: config user & ip
    system("clear");
    printf("\n\n");
    printf("         ///   ///////       //////  \n");
    printf("        ///   ///    ///  ///        \n");
    printf("       ///   ///////      ///        //   ||_||  //\\\\ =||=\n");
    printf("      ///   ///   ///      ///////   \\\\   || || //~\\\\  || \n\n");
    printf("      ___________________________\n");
    printf("               CONNEXION         \n");
    printf("      ___________________________\n");
    printf("      Pseudo: ");
    scanf("%s",pseudo);

    do{
        printf("      IP serveur: ");
        scanf("%s",addresseIP);
    }while(creerSocket(addresseIP,pseudo)==-1);



    //Thread de perssistance de connexion
    pthread_mutex_lock(&mutex);
    if(pthread_create(&threadHeartBeat, NULL, heartBeats, NULL) == -1) {
        perror("pthread_create threadHeartBeat");
        return EXIT_FAILURE;
    }


    printf("\n===// IRCChat //===========================================\n");

    // Connexion au serveur
    int code;
    do{
        code = connexion();
        switch(code){

            case Connectnumberrefuse:
                printf("Veuillez reaysser plus tard");
                break;

            case Connectuserrefuse:
                printf("\nVeuillez saisir le pseudo  : ");
                scanf("%s",pseudo);
                break;

        }
    }while(code!=Connectok);

    printf("Connexion accepté");

    pthread_mutex_unlock(&mutex);

    close(sd);
    return 0;
}



int connexion(){
    Trame trame;
    trame.ID_OP = Connect;
    strcpy(trame.DATA,"PEC");
    socklen_t taille=sizeof(serv_addr);

    if(envoyer(&trame,&serv_addr)==-1){
        perror("sendto");
        return -1;
    }else{
        if(recevoir(&trame,&serv_addr) == -1){
            perror("sendto");
            return -1;
        }
        else
        {
            idUser = trame.ID_OP==Connectok ? trame.ID_USER : -1;
            printf("%s\n",trame.DATA);
            return trame.ID_OP;
        }
    }

}


int creerSocket(const char * adresseIp, const char* pseud){

    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port        = htons(0);

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
        envoyer(&trame,&serv_addr);
    }
}

int envoyer(Trame *trame, struct sockaddr_in *addresseServeur) {
    return  (int)sendto(sd, &trame , sizeof(trame) , 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

int recevoir(Trame *trame, struct sockaddr_in *addresseServeur) {
    socklen_t taille=sizeof(serv_addr);
    return (int)recvfrom(sd, &trame, sizeof(trame) , 0, (struct sockaddr *)&serv_addr, &taille );
}
