#include "../socket.hpp"

#pragma GCC diagnostic ignored "-Wwrite-strings"

int main(int argc, char *argv[]) {
	ClientTCP *tcp = new ClientTCP();
	
	if(!tcp->connetti(Address("0.0.0.0", 54321)))
		printf("Errore connessione\n");
	
	printf("%s", tcp->conn->ricevi());
	
	delete tcp;
}
