#include "../socket.hpp"

#pragma GCC diagnostic ignored "-Wwrite-strings"

int main(int argc, char *argv[]) {
	ServerTCP *tcp = new ServerTCP("0.0.0.0", 54321);
	ServerTCP::ConnessioneServer *conn;
	
	printf("Inizio\n");
	
	if(!(conn = tcp->accetta()))
		printf("Errore connessione\n");
	if(!(conn = tcp->accetta()))
		printf("Errore connessione\n");
	
	conn->mostraKey();
		
	printf("Fine\n");
	
	delete tcp;
}
