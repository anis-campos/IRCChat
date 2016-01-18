#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "protocol.h"

struct Client {
  struct sockaddr client_addr;
  char name[100];
};

struct Salon {
  char name[100];
  int clients_id[50];
};

typedef struct Client Client;
typedef struct Salon Salon;

int main(void)
{
  int sd, n;
  socklen_t addr_len;
  struct sockaddr_in client_addr, server_addr;
  Trame trame;
  
  Salon salons[10];
  
  int i;
  for (i = 0; i<10; i++) {
      sprintf(salons[i].name,"salon%d",i);
  }
  
  Client clients[50];
  int j;
  for (j = 0; j<50; j++) {
     
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
  if (bind(sd, (struct sockaddr *)&server_addr, sizeof server_addr) == -1)
  {
    perror("bind");
    return 1;
  }
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
    }
  }
  return 0;
}

