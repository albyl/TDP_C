#include "../socket.hpp"

#pragma GCC diagnostic ignored "-Wwrite-strings"

int main(int argc, char *argv[]) {
	ClientTCP *tcp = new ClientTCP();
	Address *addr = new Address("0.0.0.0", 54321);
	
	if(!tcp->connetti(addr))
		printf("Errore connessione\n");
	
	printf("%s\n", tcp->ricevi());
	
	delete addr;
	delete tcp;
}
