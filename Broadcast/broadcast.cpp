#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "../socket.hpp"

typedef struct {
	SocketUDP *udp;
	Address  *addr;
} param_t;

void* ascoltatore(void*);
void lancia_shell(SocketUDP*, Address);

int main(int argc, char *argv[]) {
	Address *bctAddr;
	SocketUDP *udp;
	Address *mittente;
	
	char *broadcast_ip;
	int broadcast_port;
	
	param_t p;
	pthread_t ascolta;
	
	if(argc != 3) {
		printf("USAGE: %s IND PORT\n", argv[0]);
		return -1;
	}
	
	broadcast_ip = argv[1];
	broadcast_port = atoi(argv[2]);
	
	bctAddr = new Address(broadcast_ip, broadcast_port);
	
	udp = new SocketUDP(bctAddr);
	udp->enableBroadcast();
	
	mittente = new Address();
 	
	p.udp  = udp;
	p.addr = mittente;
	

	pthread_create(&ascolta, NULL, ascoltatore, (void*)&p);
	
	lancia_shell(udp, *bctAddr);
	
	delete udp;
	delete mittente;
}

void* ascoltatore(void *p) {
	param_t *param = (param_t*)p;
	
	Address *mittente = param->addr;
	SocketUDP *udp = param->udp;
	
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
			printf("Errore nella ricezione\n");
		}

	}
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

void lancia_shell(SocketUDP *udp, Address addr) {
	char *lettura;
	
	while(strcmp(lettura = input_str(), "exit")) {
		
		udp->invia(addr, lettura);
		
		
		free(lettura);
	}
	
	free(lettura);
	
}
