#include <stdio.h>

#include "../socket.hpp"

int main(int argc, char *argv[]) {
	SocketUDP *udp;
	Address *destinatario;
	
	char *server_ip;
	int server_port;
	char *msg;
	
	if(argc != 4) {
		printf("USAGE: %s IND PORT MSG\n", argv[0]);
		return -1;
	}
	
	server_ip = argv[1];
	server_port = atoi(argv[2]);
	msg = argv[3];
	
	udp = new SocketUDP();
	destinatario = new Address(server_ip, server_port);
	
	printf("%s\n", udp->invia(*destinatario, msg)
		? "Inviato" : "Errore nell'invio");
	
	delete udp;
	delete destinatario;
	
	return 0;
}
