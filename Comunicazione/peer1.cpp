#include "../socket.hpp"

#pragma GCC diagnostic ignored "-Wwrite-strings"

int main(int argc, char *argv[]) {
	SocketUDP myself("0.0.0.0", 8081);
	Address dest("0.0.0.0", 8082);
	char *risposta, *da;
	
	
	if(!myself.invia(dest, "Msg1"))
		printf("Errore nell'invio\n");
	printf("Invia\n");
	
	risposta = myself.ricevi(&dest);
	
	da = dest.toString();
	printf("%s Ricevo: %s\n", da, risposta);
	
	free(da);
	free(risposta);
	
}
