#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "protocol.h"
#include <stdlib.h>
#include <time.h>

struct Client {
    int actif;
    struct sockaddr_in client_addr;
    char name[100];
    int timestamp;
};

struct Salon {
    char name[100];
    int clients_id[50];
};

typedef struct Client Client;
typedef struct Salon Salon;

int addClient(Client* clients, Trame* trame, struct sockaddr_in client_addr);
int addClientToSalon(Salon* salons, Trame* trame);
void echo(Salon salon,int id_salon, char* message, Client* clients);
void deleteFromSalons(Client* clients,int clientID,Salon* salons);
void timeoutHandle(Client* clients, Salon* salons);
void listeServeur(Salon* salons, Client* clients);


int sd;

void quit( int signalId) {
  close(sd);
  exit(0);
}

int main(void)
{
  int n;
  int a = 1;
  socklen_t addr_len;
  struct sockaddr_in client_addr, server_addr;
  Trame trame;
  Trame reponseClient;

  Salon salons[10];

  char message[100];

  int i;
  for (i = 0; i<10; i++) {
    sprintf(salons[i].name,"salon%d",i);
    int w;
    for (w = 0; w<50; w++) {
      salons[i].clients_id[w] = -1;
    }
  }

  Client clients[50];
  int j;
  for (j = 0; j<50; j++) {
    clients[j].actif = 0;
  }

  // Create socket
  if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("socket creation");
    return 1;
  }
  // Bind it
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERVER_PORT);

  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &a, sizeof(a) );

  if (bind(sd, (struct sockaddr *)&server_addr, sizeof server_addr) == -1)
  {
    perror("bind");
    return 1;
  }
  signal(SIGINT,quit);
  for (;;)
  {
    addr_len = sizeof(client_addr);
    n = recvfrom(sd, (void*) &trame, sizeof(Trame), 0, (struct sockaddr *)&client_addr, &addr_len);
    if (n == -1)
      perror("recvfrom");
    else {

int ret;
      switch(trame.ID_OP){

        case Connect :

              printf("Demande de connexion\n");
               ret = addClient(clients, &trame, client_addr);

              if (ret >= 0) {
                printf("connexion réussie\n");
                reponseClient.ID_OP = ConnectOk;
                reponseClient.ID_USER = ret;
		clients[ret].timestamp = time(NULL);
		timeoutHandle(clients,salons);
              }
              else if (ret == -1) {
                printf("échec : trop de clients\n");
                reponseClient.ID_OP = ConnectNumberRefuse;
              }
              else {
                printf("échec : nom déjà utilisé\n");
                reponseClient.ID_OP = ConnectUserRefuse;
              }

              break;

        case Join:

	    ret = addClientToSalon(salons, &trame);
	    clients[trame.ID_USER].timestamp = time(NULL);
              if (ret >= 0) {
                printf("connexion réussie\n");
                reponseClient.ID_OP = JoinOk;
                reponseClient.ID_USER = trame.ID_USER;
                reponseClient.ID_SALON = ret;
                sprintf(reponseClient.DATA,"%s",trame.DATA);
                sprintf(message,"%s joined #%s",clients[trame.ID_USER].name,salons[ret].name);
                echo(salons[ret],ret,message,clients);
		timeoutHandle(clients,salons);
              }
              else if (ret == -1) {
                printf("échec : tu es déjà dans le salon\n");
                reponseClient.ID_OP = JoinRefuse;
		reponseClient.ID_USER = trame.ID_USER;
		strcpy(reponseClient.DATA,"échec : tu es déjà dans le salon");
              }

          break;

	  case Disconnect:
	    printf("Disconnect\n");
	    deleteFromSalons(clients,trame.ID_USER,salons);
	    clients[trame.ID_USER].actif = 0;
	    clients[trame.ID_USER].name[0] = '\0';
	    timeoutHandle(clients,salons);
	    break;

	  case Say:
	    clients[trame.ID_USER].timestamp = time(NULL);
	    printf("say\n");
	    sprintf(message,"#%s<%s> %s",salons[trame.ID_SALON].name,clients[trame.ID_USER].name,trame.DATA);
	    echo(salons[trame.ID_SALON],trame.ID_SALON,message,clients);
	    reponseClient.ID_OP = 11;
	    reponseClient.ID_USER = trame.ID_USER;
	    break;

	  case Leave :
	    clients[trame.ID_USER].timestamp = time(NULL);
	    printf("leave\n");
	    i = 0;
	    while (i<10 && salons[trame.ID_SALON].clients_id[i] != trame.ID_USER) {
	      i++;
	    }
	    salons[trame.ID_SALON].clients_id[i] = -1;
	    sprintf(message,"%s a quitté #%s",clients[trame.ID_USER].name,salons[trame.ID_SALON].name);
	    timeoutHandle(clients,salons);
	    break;

	  case Liste :
	    clients[trame.ID_USER].timestamp = time(NULL);
	    timeoutHandle(clients,salons);
	    printf("liste\n");
	    message[0] = '\0';
	    for (i = 0; i<10; i++) {
	      strcat(message,salons[i].name);
	      strcat(message,"\n");
	    }
	    reponseClient.ID_OP = 13;
	    reponseClient.ID_USER = trame.ID_USER;
	    strcpy(reponseClient.DATA,message);
	    break;
	  case Verify :
	    timeoutHandle(clients,salons);
	    break;
	  case HeartBeat :
	    clients[trame.ID_USER].timestamp = time(NULL);
	    break;


        default:
          printf("ERREUR : invalid ID_OP %d\n",trame.ID_OP);
          exit(-1);
      }



      if (sendto(sd, (void*) &reponseClient, sizeof(reponseClient), 0,
                 (struct sockaddr *)&(client_addr), sizeof(client_addr)) == -1)
      {
        perror("sendto");
      }
    }
  }
  return 0;
}

void timeoutHandle(Client* clients, Salon* salons) {
    int i;
    int date_now = time(NULL);
    for (i = 0; i<50; i++) {
      if (clients[i].actif && clients[i].timestamp<date_now-900) {
	printf("%d\n",clients[i].timestamp );
	printf("%d\n",date_now-900 );
	deleteFromSalons(clients,i,salons);
	clients[i].actif = 0;
	clients[i].name[0] = '\0';
      }
    }
    listeServeur(salons,clients);
}

void echo(Salon salon,int id_salon, char* message, Client* clients){
  Trame trame;
  trame.ID_OP = 13;
  trame.ID_SALON = id_salon;
  strcpy(trame.DATA, message);
  int w;
  for (w = 0; w<10; w++) {
    if(salon.clients_id[w] != -1){
      trame.ID_USER = salon.clients_id[w];
      if (sendto(sd, (void*) &trame, sizeof(trame), 0,
                 (struct sockaddr *)&(clients[salon.clients_id[w]].client_addr), sizeof(clients[salon.clients_id[w]].client_addr)) == -1)
      {
        perror("sendto");
      }
    }
  }
}

void listeServeur(Salon* salons, Client* clients){
    int i, y;
    system("clear");
    printf("Liste des utilisateurs dans les salons du serveur.\n");
    for (i = 0; i<10; i++) {
	printf("\n%s :\n", salons[i].name);
	for (y = 0; y<50; y++) {
	  if(salons[i].clients_id[y] != -1)
	    printf("\t%s\n", clients[salons[i].clients_id[y]].name);
	}
    }
}

int addClient(Client* clients, Trame* trame, struct sockaddr_in client_addr) {
  int i = 0;
  int memeNom = 0;
  int toutActif = 1;
  int ind = -1;

  while (i<50 && !memeNom) {
    memeNom = (strcmp(clients[i].name,trame->DATA)==0);

    if (!clients[i].actif && toutActif) {
      toutActif = 0;
      ind = i;
    }
    i++;
  }

  if (memeNom) {
    return -2;
  }
  else if (toutActif) {
    return -1;
  }
  else {
    clients[ind].actif = 1;
    strcpy(clients[ind].name,trame->DATA);
    clients[ind].client_addr = client_addr;

    return ind;
  }
}

int addClientToSalon(Salon* salons, Trame* trame) {
  int i;
  for(i = 0; i<10; i ++){
    if(strcmp(salons[i].name,trame->DATA) == 0){
      int a;
      for(a = 0; a<50; a ++){
        if(salons[i].clients_id[a] == trame->ID_USER){
           return -1;
        }
      }
      int w =0, exist = 0;
      while(w<50 && exist ==0){
        if(salons[i].clients_id[w] == -1){
          salons[i].clients_id[w] = trame->ID_USER;
          exist = 1;
        }
        w++;
      }
      return w;
    }
  }
  return -1;
}

void deleteFromSalons(Client* clients, int clientID, Salon* salons) {
  int i,j;
  char message[100];
  for (i = 0; i<10; i++) {
    for (j = 0; j<10; j++) {
      if (salons[i].clients_id[j] == clientID) {
	salons[i].clients_id[j] = -1;
	sprintf(message,"%s vient de se déconnecter",clients[clientID].name);
	echo(salons[i],i,message,clients);
      }
    }
  }
}
