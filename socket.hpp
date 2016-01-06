#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"

#define MAX_STR 4096

// 3*4 (num. ip) + 3 (punti) + 1 (:) + 4 (port) + 1 ('\0') + 2 ([ ... ])
#define MAX_STR_IP_PORT 23

/*
 * Permette l'identificazione di un processo memorizzando
 * una coppia indirizzo ip:porta. Non vengono dichiarati
 * i campi direttamente, ma memorizzati all'interno di
 * struttura sockaddr_in
 */

class Address {
	private:
		struct sockaddr_in addr; // indirizzo ip:porta
	public:		
		Address(char*, int);
		Address() : Address("0.0.0.0", 0) {}
		
		/*
		 * La visibilità dell'indirizzo e della porta è pubblica,
		 * ma l'accesso non è diretto, avviene tramite i metodi
		 * seguenti
		 */
		char* getIp() { return strdup(inet_ntoa(addr.sin_addr)); }
		int getPort() { return ntohs(addr.sin_port); }
		void setPort(int port) { addr.sin_port = htons(port); }
		void setIp(char *ip)  { inet_aton(ip, &addr.sin_addr); }
		
		/*
		 * Scorciatoia per la conversione di indirizzo ip:porta in
		 * stringa.
		 * RICORDATI: La stringa è allocata dinamicamente quindi finito
		 *            l'utilizzo la memoria deve essere liberata
		 */
		char* toString();
		
		/*
		 * Interfaccia per funzioni native
		 */
		struct sockaddr* get_sockaddr()
			{ return (struct sockaddr*)&addr; }
		
		/*bool operator(Address a1, Address a2) {
			bool ret;
			char *ind1 = a1.getIp();
			char *ind2 = a2.getIp();
			
			int cmp = strcmp(ind1, ind2);
			
			free(ind1); free(ind2);
			
			if(cmp < 0) return false;
			if(cmp > 0) return true;
			
			return a1.getPort() < a2.getPort();
		}*/
};

Address::Address(char *ip, int port) {
	
	//addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	
	/*
	 * Compilo i campi
	 */
	
	addr.sin_family = AF_INET;
	
	this->setPort(port);
	this->setIp(ip);
	
	for(int i=0; i<8; i++)
		addr.sin_zero[i] = 0;
}

char* Address::toString() {
	char buffer[MAX_STR_IP_PORT];
	char *ip = this->getIp();
	
	sprintf(buffer, "[%s:%d]", ip, this->getPort());
	
	free(ip);
	
	return strdup(buffer);
}

/*
 * Gestione socket per trasferimento connectionless
 */
 
class SocketUDP {
	private:
		int sock_id;
	public:
		/*
		 * Un socket si basa su un Address, è possibile passarlo
		 * direttamente o specificare solo indirizzo ip e porta
		 * lasciando il compito al costruttore
		 */
		SocketUDP();
		SocketUDP(Address*);
		SocketUDP(char*, int);
			
		~SocketUDP() {shutdown(sock_id, SHUT_RDWR); }
		
		/*
		 * Gestione invio e ricezione
		 */
		bool invia(Address, char*);
		char* ricevi(Address*);
		
		bool enableBroadcast();
};

SocketUDP::SocketUDP() {
	this->sock_id = socket(AF_INET, SOCK_DGRAM, 0);
}

SocketUDP::SocketUDP(Address *addr) : SocketUDP() {
	bind(this->sock_id, addr->get_sockaddr(), sizeof(struct sockaddr));
}

SocketUDP::SocketUDP(char *ip, int port) : SocketUDP() {
	Address addr(ip, port);
	
	bind(this->sock_id, addr.get_sockaddr(), sizeof(struct sockaddr));
}

bool SocketUDP::invia(Address dest, char *msg) {
	int ret_code;
	
	// Lo salvo perchè viene usato in più di un posto
	int len_msg = strlen(msg)+1;
	
	ret_code = sendto(this->sock_id, msg, len_msg, 0,
	                  dest.get_sockaddr(),
	                  sizeof(struct sockaddr));
	                  
	return ret_code == len_msg;
}

char* SocketUDP::ricevi(Address *mit) {
	char buffer[MAX_STR + 1];
	int ret_code;
	
	int len_addr = sizeof(struct sockaddr);
	
	ret_code = recvfrom(this->sock_id, buffer, MAX_STR, 0,
	                    mit->get_sockaddr(),
	                    (socklen_t*)&len_addr);
	
	if(ret_code <= 0)
		return NULL;
	
	buffer[ret_code] = '\0';
	
	return strdup(buffer);
}

bool SocketUDP::enableBroadcast() {
	int val = 1;
	
	return !setsockopt(sock_id, SOL_SOCKET, SO_BROADCAST,
	                   &val, sizeof(int));
}

#endif
