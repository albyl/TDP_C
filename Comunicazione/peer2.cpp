#include "../socket.hpp"

#pragma GCC diagnostic ignored "-Wwrite-strings"

int main(int argc, char *argv[]) {
	SocketUDP myself("0.0.0.0", 8082);
	Address dest("0.0.0.0", 8081);
	char *risposta, *da;
	
	
	risposta = myself.ricevi(&dest);
	da = dest.toString();	
	printf("%s Ricevo: %s\n", da, risposta);
	free(da);
	free(risposta);	
	
	myself.invia(dest, "Msg1");
	printf("Invia\n");
}
