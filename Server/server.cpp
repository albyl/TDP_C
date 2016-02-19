#include <stdio.h>
#include <pthread.h>

#include "../socket.hpp"

typedef struct {
	SocketUDP *udp;
	Address  *addr;
} param_t;

void* ascoltatore(void*);

int main(int argc, char *argv[]) {
	SocketUDP *udp;
	Address *mittente;
	
	param_t p;
	pthread_t ascolta;
	
	if(argc != 2) {
		printf("USAGE: %s PORT\n", argv[0]);
		return -1;
	}
	
	udp = new SocketUDP("0.0.0.0", atoi(argv[1]));
	mittente = new Address();
 	
	p.udp  = udp;
	p.addr = mittente;
	
	pthread_create(&ascolta, NULL, ascoltatore, (void*)&p);
	
	getchar();
	
	delete udp;
	delete mittente;
	
	return 0;
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
