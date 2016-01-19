#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "protocol.h"

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

int main(void)
{
  int sd, n;
  socklen_t addr_len;
  struct sockaddr_in client_addr, server_addr;
  Trame trame;
  Trame reponseClient;
  Trame reponseMultiples;
  
  Salon salons[10];
  
  int i;
  for (i = 0; i<10; i++) {
      sprintf(salons[i].name,"salon%d",i);
  }
  
  Client clients[50];
  int j;
  for (j = 0; j<50; j++) {
     clients[j].actif = 0;
  }
  
  int destinatairesId[50];
  int nbDestinataires;
  
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
  if (bind(sd, (struct sockaddr *)&server_addr, sizeof server_addr) == -1)
  {
    perror("bind");
    return 1;
  }
  for (;;)
  {
    nbDestinataires = 0;
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
	printf("JOIN\n");
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
	 printf("ERREUR\n");
      }
      
      if (sendto(sd, (void*) &reponseClient, sizeof(reponseClient), 0,
	    (struct sockaddr *)&(clients[reponseClient.ID_USER].client_addr), sizeof(client_addr)) == -1)
      {
	perror("sendto");
      }
      
      for (i = 0; i<nbDestinataires; i++) {
	if (sendto(sd, (void*) &reponseMultiples, sizeof(reponseMultiples), 0,
	    (struct sockaddr *)&(clients[destinatairesId[i]].client_addr), sizeof(client_addr)) == -1)
      {
	perror("sendto");
      }
      }
    }
  }
  return 0;
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
