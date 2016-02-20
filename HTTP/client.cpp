#include <stdio.h>
#include <string.h>
#include "../socket.hpp"

#define HTTP_REQ "GET / HTTP/1.1\
"

#define ANS_200OK "OK 200"

int main(int argc, char *argv[]) {
	int port;
	char *ip;
	ClientTCP *myself;
	Address *server;
	char *answer;

	if(argc != 3) return -1;
	
	port = atoi(argv[1]);
	ip = argv[2];

	myself = new ClientTCP();
	
	server = new Address(ip, port);
	printf("Connessione\n");
	if(!myself->connetti(server))
		errore("Errore connessione", -2);

	printf("Sto inviando");

	if(!myself->invia(HTTP_REQ))
		errore("Errore invio!", -3);
	
	printf("Passo"); 

	if(!(answer = myself->ricevi()))
		errore("Errore rivevuto", -4);

	printf("%s\n", answer);
	
	if(!(strstr(answer, ANS_200OK)))
		errore("Errore ricezione", -5);
	

	free(answer);
	/*if(!(answer = myself->ricevi()))
		errore("Errore pagina web", -6);
	
	printf("%s\n", answer);

	free(answer);*/
	delete server;
	delete myself;

	return 0;	
}
