#include "../socket.hpp"

// The little book of lisp

#pragma GCC diagnostic ignored "-Wwrite-strings"

int main(int argc, char *argv[]) {
	ServerTCP *tcp = new ServerTCP("0.0.0.0", 54321);
	ConnessioneServer *conn;
	
	printf("Inizio\n");
	
	if(!(conn = tcp->accetta()))
		printf("Errore connessione\n");
	if(!(conn = tcp->accetta()))
		printf("Errore connessione\n");
	
	conn->mostraKey();
	
	for(Iterator* it = tcp->createIterator();
	         !it->isDone(); it->moveNext()) {	
		
		conn = (ConnessioneServer*)it->getCurrent();
		
		printf("Comunco con un client\n");
		
		conn->invia("Termina");
		
	}
		
	printf("Fine\n");
	
	delete tcp;
}
