#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "protocol.h"
#include <stdlib.h>

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

int sd;

void quit() {
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
      for (w = 0; w<10; w++) {
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
    n = recvfrom(sd, (void*) &trame, sizeof(Trame), 0,
        (struct sockaddr *)&client_addr, &addr_len);
    if (n == -1)
      perror("recvfrom");
    else {
     
      if (trame.ID_OP == 0) {
	printf("Demande de connexion\n");
	int ret = addClient(clients, &trame, client_addr);
	
	if (ret >= 0) {
	  printf("connexion réussie\n");
	  reponseClient.ID_OP = 1;
	  reponseClient.ID_USER = ret;
	}
	else if (ret == -1) {
	  printf("échec : trop de clients\n");
	  reponseClient.ID_OP = 3;
	}
	else {
	  printf("échec : nom déjà utilisé\n");
	  reponseClient.ID_OP = 2;
	}
      }
      else if (trame.ID_OP == 4) {
	int ret = addClientToSalon(salons, &trame);
	
	if (ret >= 0) {
	  printf("connexion réussie\n");
	  reponseClient.ID_OP = 5;
	  reponseClient.ID_USER = trame.ID_USER;
	  reponseClient.ID_SALON = ret;
	  sprintf(message,"%s joined #%s",clients[trame.ID_USER].name,salons[ret].name);
	  echo(salons[ret],ret,message,clients);
	}
	else if (ret == -1) {
	  printf("échec : trop de clients\n");
	  reponseClient.ID_OP = 6;
	}
      }
      else if (trame.ID_OP == 7) {
	printf("Disconnect\n");
      }
      else if (trame.ID_OP == 8) {
	 printf("leave\n");
      }
      else if (trame.ID_OP == 9) {
	 printf("liste\n");
      }
      else if (trame.ID_OP == 10) {
	 printf("say\n");
      }
      else {
	 printf("ERREUR : invalid ID_OP %d\n",trame.ID_OP);
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

void listeServeur(Salon* salon){
  
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
  printf("%d\n",ind);
  
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
	while(w<50 && !exist){
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

