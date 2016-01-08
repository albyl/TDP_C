#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "../socket.hpp"

#define MY_ADDR "0.0.0.0"
#define BROADCAST_IP "255.255.255.255"
#define EXIT_CMD "exit"

#define ERRORE_RICEZIONE "Errore nella ricezione\n"

int broadcast_port;

void* ascoltatore(void*);
void lancia_shell(SocketUDP*);

int main(int argc, char *argv[]) {
	SocketUDP *udp;
	
	pthread_t ascolta;
	
	if(argc != 2) {
		printf("USAGE: %s PORT\n", argv[0]);
		return -1;
	}
	
	broadcast_port = atoi(argv[1]);
	
	udp = new SocketUDP(strdup(MY_ADDR), broadcast_port);
	udp->enableBroadcast();	

	pthread_create(&ascolta, NULL, ascoltatore, (void*)udp);
	
	lancia_shell(udp);
	
	delete udp;
}

void* ascoltatore(void *p) {	
	SocketUDP *udp = (SocketUDP*)p;
	
	Address *mittente = new Address();
	
	char *dato, *da;
		
	for( ; ; ) {
		dato = udp->ricevi(mittente);
		
		if(dato) {
			da = mittente->toString();
			
			printf("%s %s\n", da, dato);
			
			free(da);
			free(dato);
		}
		else {
			printf(ERRORE_RICEZIONE"\n");
		}
	}
	
	delete mittente;
}

char *input_str()
{
	char buffer[4096];
	int i;
	
	printf("$> ");
	for(i=0; (buffer[i] = getchar()) != '\n'; i++);
	buffer[i] = '\0';

	return strdup(buffer);
}

void lancia_shell(SocketUDP *udp) {
	char *lettura;
	Address addr(BROADCAST_IP, broadcast_port);
	
	while(strcmp(lettura = input_str(), EXIT_CMD)) {
		
		udp->invia(addr, lettura);
		
		free(lettura);
	}
	
	free(lettura);
	
}
