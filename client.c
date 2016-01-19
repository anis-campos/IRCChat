#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "client.h"

#define MAX_SALON 10


int sd;
struct sockaddr_in serv_addr;
int idUser;
char pseudo[15];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

fd_set set; // Ensemble des descripteurs de fichiers en lecture

struct timeval timeout;

void nextSalon();

void prevSalon();

void setSalon(char *token);

char *nomSalon[MAX_SALON];
int idSalons[MAX_SALON];
int indexSalonCurent=0;
int indexLast=0;


void clean_stdin(void)
{
    int c;

    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}


void initSelect(){

    /* Initialize the file descriptor set. */
    FD_ZERO (&set);
    FD_SET (STDIN_FILENO, &set);
    FD_SET (sd, &set);


    /* Initialize the timeout data structure. */
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;
}

int main (int argc, char *argv[])
{
    //Variables du main
    int cptr=0;
    pthread_t threadHeartBeat;
    char addresseIP[20];
    
    /*******************ACCEUIL********************/
    //Accueil: config user & ip
    system("clear");
    printf("\n\n");
    printf("         ///   ///////       //////  \n");
    printf("        ///   ///    ///  ///        \n");
    printf("       ///   ///////      ///        // ||_||  //\\\\ =||=\n");
    printf("      ///   ///   ///      ///////   \\\\ || || //~~\\\\ || \n\n");
    printf("      ___________________________\n");
    printf("               CONNEXION         \n");
    printf("      ___________________________");
    printf("\n      Pseudo: ");
    scanf("%s",pseudo);
    clean_stdin();
    do{
        printf("      IP serveur: ");
        scanf("%s",addresseIP);
        clean_stdin();
    }while(creerSocket(addresseIP)==-1);



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

            case ConnectNumberRefuse:
                printf("\n      *Veuillez réessayer plus tard.");
                exit(-1);


            case ConnectUserRefuse:
                printf("\n      Veuillez saisir un autre pseudo : ");
                scanf("%s",pseudo);
                clean_stdin();
                break;
        }
     }while(code!= ConnectOk);

    /*******************MODE T'CHAT********************/
    system("clear");

    printf("\n     | *Connexion acceptée");

    //lancer le thread HeartBeat
    pthread_mutex_unlock(&mutex);





    int retval;
    Trame trame;


    printf("\nListe des salons\n");

    trame.ID_OP=Liste;
    trame.ID_USER=idUser;
    envoyer(trame,serv_addr);
    recevoir(&trame,serv_addr);
    printf("%s",trame.DATA);



    printf("\n===// IRCChat //==============================================");


    while(1){

        if(indexLast>0){
            printf("#%s>",nomSalon[indexSalonCurent]);
            fflush(stdout);
        }

        initSelect();
        retval = select(sd+1,&set, NULL, NULL,&timeout);


        if (retval == -1)
        {
            perror ( "\n     | *Erreur de select." ) ;
        }
        else if (retval == 1)
        {


            // Nouvelles données clavier
            if (FD_ISSET(0, &set))
            {

                if(traitementEnvoye()>0){

                    //si envoyé
                    FD_ZERO (&set);
                    FD_SET (sd, &set);

                    /* Initialize the timeout data structure. */
                    timeout.tv_sec = 3;
                    timeout.tv_usec = 0;

                    //timeout de l'aquitement
                    if(select(sd+1,&set, NULL, NULL,&timeout)!=1){
                        printf("Le serveur ne repond pas. Fin du programme....");
                        exit(0);
                    }

                    //recevoir l'aquittement
                    recevoir(&trame,serv_addr);
                    traitementReception(trame);
                }

            }

                //Nouveau message du serveur;
            else if(FD_ISSET(sd, &set)){
                recevoir(&trame,serv_addr);
                traitementReception(trame);

            }

        }
        else{
            //printf("\n     | *TIMEOUT\n");
            cptr++;
            if(cptr==3){
                printf("\n     | *Le serveur n'est plus disponible...Connexion perdue.");
                exit(-1);
            }
        }



    }

    close(sd);
    return 0;
}


void traitementReception(Trame trameRecue){
    int i;
    switch(trameRecue.ID_OP){
        case ConnectOk :
            idUser = trameRecue.ID_USER;//maj id
            //printf("\t<ConnectionOK> : %s\n",trameRecue.DATA);
            break;

        case JoinOk :
	    //printf("\t<JoinOk> : %s\n",trameRecue.DATA);
             idSalons[indexLast]=trameRecue.ID_SALON;
             nomSalon[indexLast]=malloc(strlen(trameRecue.DATA)+1);
            strcpy(nomSalon[indexLast],trameRecue.DATA);
            indexSalonCurent = indexLast;
            indexLast++;
            break;

        case LeaveOk:
            //printf("\t<LeaveOk> : %s\n",trameRecue.DATA);

            for ( i = 0; i < indexLast; ++i) {
                if(idSalons[i]==trameRecue.ID_SALON){
                    free(nomSalon[i]);
                    nomSalon[i]=nomSalon[indexLast];
                    idSalons[i]=idSalons[indexLast];
                    if (indexSalonCurent == i)
                        indexSalonCurent--;

                    indexLast--;
                    break;
                }
            }
            break;

        case JoinRefuse :
	    printf("\t<JoinRefuse> : %s\n",trameRecue.DATA);
            break;

        case SayOk :
	    //printf("\t<SayOk> : %s\n",trameRecue.DATA);
            break;

        case SayError :
	    printf("\t<SayError> : %s\n",trameRecue.DATA);
            break;

	case ErrorCommande :
	    printf("\t<ErrorCommande> : %s\n",trameRecue.DATA);
            break;

        case Echo :
            printf("\t%s",trameRecue.DATA);
            break;
        default:
            printf("\tERREUR : %d - %s",trameRecue.ID_OP,trameRecue.DATA);
    }

}




int traitementEnvoye() {

    Trame trame;

    char buf[1024];
    //strcpy(buf,"");


    scanf("%[^\n]",buf);
    clean_stdin();

    const char s[2] = " ";

    char *token;

    /* get the first token */
    token = strtok(buf, s);

    if(strlen(token)<4) return -1;

    if(token[0]==':'){
        if(!(strcmp(token,":next"))){
            nextSalon();
            return 0;
        }
        if(!(strcmp(token,":prev")))
        {
            prevSalon();
            return 0;
        }
        if(!(strcmp(token,":set"))){
            token = strtok(NULL, s);
            setSalon(token);
            return 0;
        }
        trame.ID_OP = commandToInt(token);
        if(trame.ID_OP==-1){
            printf("\nCommande inconue : %s\n",token);
            return -1;
        }else{

            if(trame.ID_OP==Join && indexLast==MAX_SALON-1){
                printf("Désolé, nombre maximum de salon atteint");
                return -1;
            }


            token = strtok(NULL, s);

            strcpy(trame.DATA,"");
            /* walk through other tokens */
            while( token != NULL )
            {
                strcat(trame.DATA,token);
                token = strtok(NULL, s);
            }

        }
    }
    else{
        trame.ID_OP=Say;
        strcpy(trame.DATA,buf);
    }


    trame.ID_SALON = idSalons[indexSalonCurent];
    trame.ID_USER = idUser;

    int ret =  envoyer(trame,serv_addr);


    if(trame.ID_OP==Disconnect)
    {
        printf("ByeBye....\n");
        exit(0);
    }

    return ret;


}

void setSalon(char *token) {

    int i;
    for ( i = 0; i < indexLast; ++i) {
        if(!strcmp(token,nomSalon[i])){
            indexSalonCurent = i;
        }
    }
}

void prevSalon() {
    if(indexSalonCurent!=0){
      indexSalonCurent--;
    }
}

void nextSalon() {
    if(indexSalonCurent!=indexLast){
        indexSalonCurent++;
    }
}


int commandToInt(char * command) {

    if(!strcmp(command,":say"))
        return Say;
    if(!strcmp(command,":join"))
        return Join;
    if(!strcmp(command,":leave"))
        return Leave;
    if(!strcmp(command,":liste"))
        return Liste;
    if(!strcmp(command,":disconnect"))
        return Disconnect;


    //n'est pas une commande
    return -1;
}




int connexion(){
    Trame trame;
    trame.ID_OP = Connect;
    strcpy(trame.DATA,pseudo);
    socklen_t taille=sizeof(serv_addr);

    if(envoyer(trame,serv_addr)==-1){
        perror("sendto");
        exit( -1);
    }


    if(recevoir(&trame,serv_addr) == -1){
        perror("sendto");
        exit(-1);
    }

    idUser = trame.ID_OP== ConnectOk ? trame.ID_USER : -1;
    printf("%s\n",trame.DATA);
    return trame.ID_OP;

}




int creerSocket(const char * adresseIp){

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

    trame.ID_OP = HeartBeat;
    trame.ID_USER = idUser;

    pthread_mutex_lock(&mutex);

    while(1){
        sleep(FREQ_HEART);
        envoyer(trame,serv_addr);
    }
}

int envoyer(Trame trame, struct sockaddr_in addresseServeur) {
    return  (int)sendto(sd, &trame , sizeof(trame) , 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

int recevoir(Trame * trame, struct sockaddr_in addresseServeur) {
    socklen_t taille=sizeof(serv_addr);
    return (int)recvfrom(sd, trame, sizeof(*trame) , 0, (struct sockaddr *)&serv_addr, &taille );
}
