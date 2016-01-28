#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"

#define MAX_STR 4096

// 3*4 (num. ip) + 3 (punti) + 1 (:) + 4 (port) + 1 ('\0') + 2 ([ ... ])
#define MAX_STR_IP_PORT 23

#define MAX_CONN 50

#define DEBUG

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

class Socket {
	protected:
		int sock_id;
		
		Socket(int tipo);
		~Socket() {shutdown(sock_id, SHUT_RDWR);}
		Socket(Address, int);
		Socket(char*, int, int);
};

Socket::Socket(int tipo) {
	this->sock_id = socket(AF_INET, tipo, 0);
	
#ifdef DEBUG
	printf("Socket: %d\n", this->sock_id);
#endif
}

Socket::Socket(Address addr, int tipo) : Socket(tipo) {
	int ret;
	
	ret = bind(this->sock_id, addr.get_sockaddr(), sizeof(struct sockaddr));
	
#ifdef DEBUG
	printf("Bind: %d\n", ret);
#endif
}

Socket::Socket(char *ip, int port, int tipo)
    : Socket(Address(ip, port), tipo) {}
 
class SocketUDP : public Socket {
	public:
		/*
		 * Un socket si basa su un Address, è possibile passarlo
		 * direttamente o specificare solo indirizzo ip e porta
		 * lasciando il compito al costruttore
		 */
		SocketUDP() : Socket(SOCK_DGRAM) {}
		SocketUDP(Address addr) : Socket(addr, SOCK_DGRAM) {}
		SocketUDP(char *ip, int port) : Socket(ip, port, SOCK_DGRAM) {}
		
		/*
		 * Gestione invio e ricezione
		 */
		bool invia(Address, char*);
		char* ricevi(Address*);
		
		bool enableBroadcast();
};

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

class Node {
	protected:
		Node* next = NULL;
		virtual void* getKey() = 0;
	public:
		~Node();
		void set_next(Node *new_next)  { next = new_next; }
		
		virtual void mostraKey() = 0;
};

Node::~Node() {
	if(next)
		delete next;
}

class Lista {
	Node *first;
	
	public:
		Lista() { first = NULL; }
		~Lista();
		void add(Node*);
		
		void scorri();
};

Lista::~Lista() {
	delete first;
}

void Lista::scorri() {
	first->mostraKey();
}

void Lista::add(Node *new_node) {
	new_node->set_next(first);
	first = new_node;
}

class Connessione {
	protected:
		int sock_id;
		Address addr;
	public:
		Connessione(int new_sock_id, Address new_addr)
		    : sock_id(new_sock_id), addr(new_addr) {}
		
		bool invia(char *);
		char* ricevi();
};

bool Connessione::invia(char *msg) {
	int ret_code;
	
	int len_msg = strlen(msg)+1;
	
	ret_code = send(this->sock_id, msg, len_msg, 0);
	                  
	return ret_code == len_msg;
}

char* Connessione::ricevi() {
	char buffer[MAX_STR + 1];
	int ret_code;
	
	ret_code = recv(this->sock_id, buffer, MAX_STR, 0);

#ifdef DEBUG
	printf("%d:%s\n", this->sock_id, strerror(errno));
#endif
	
	if(ret_code <= 0)
		return NULL;
	
	buffer[ret_code] = '\0';
	
	return strdup(buffer);
}

class SocketTCP : public Socket {
	public:
		SocketTCP() : Socket(SOCK_STREAM) {}
		SocketTCP(Address addr) : Socket(addr, SOCK_STREAM) {}
		SocketTCP(char *ip, int port) : Socket(ip, port, SOCK_STREAM) {}
};

class ServerTCP : public SocketTCP {
	private:
		
	public:
		class ConnessioneServer : public Connessione, public Node {
			public:
				ConnessioneServer(int new_sock_id, Address new_addr)
					: Connessione(new_sock_id, new_addr) {}
				~ConnessioneServer() {
#ifdef DEBUG
					printf("Chisura sock %d\n", sock_id);
#endif
					close(sock_id);
				}
				
				void* getKey() {
					return (void*)&sock_id;
				}
				
				void mostraKey() {
					printf("- %d\n", *(int*)this->getKey());
					if(next)
						next->mostraKey();
				}
		};
		
		Lista *connessioni;
		
		ServerTCP(Address addr);
		~ServerTCP() { delete connessioni; }
		ServerTCP(char* ip, int port) : ServerTCP(Address(ip, port)) {}
		
		ConnessioneServer* accetta();
};

ServerTCP::ServerTCP(Address addr) : SocketTCP(addr) {
	listen(sock_id, MAX_CONN);
	
	connessioni = new Lista();
}

ServerTCP::ConnessioneServer* ServerTCP::accetta() {
	Address addr;
	ConnessioneServer* conn;
	
	socklen_t len = sizeof(struct sockaddr);
	
	int sock = accept(sock_id, addr.get_sockaddr(), (socklen_t*)&len);
#ifdef DEBUG
	printf("%s\n", strerror(errno));
#endif
	if(sock < 0)
		return NULL;
		
	conn = new ConnessioneServer(sock, addr);
	
	connessioni->add(conn);
	
	return conn;
}

class ClientTCP : public SocketTCP {
	public:
		class ConnessioneClient : public Connessione {
			public:
				ConnessioneClient(int new_sock_id, Address new_addr)
					: Connessione(new_sock_id, new_addr) {}
		};
		
		ConnessioneClient *conn;

		ClientTCP() : SocketTCP() {}
		~ClientTCP() { delete conn; }
		
		bool connetti(Address addr) {
			conn = new ConnessioneClient(sock_id, addr);
			return !connect(sock_id, addr.get_sockaddr(), sizeof(struct sockaddr_in));
		}
		
		bool invia(Address dest, char *msg);
		char* ricevi(Address *mit);
};

#endif
