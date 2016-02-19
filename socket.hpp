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
#define SERVER_IP "0.0.0.0"

typedef enum {UDP = SOCK_DGRAM, TCP=SOCK_STREAM} Protocollo;

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
		Address(Address& new_addr);	
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
		void set_sockaddr(struct sockaddr_in nAddr) { addr = nAddr; }
		struct sockaddr_in get_sockaddr()
			{ return addr; }
};

class Socket {
	protected:
		int _sockId;
		Address *_addr = NULL;

		Socket(int, Address* = NULL);
		Socket(int, char*, int);
		~Socket() {
			shutdown(_sockId, SHUT_RDWR);
			if(!_addr) delete _addr;
		}
};

class SocketUDP : public Socket {
	public:
		/*
		 * Un socket si basa su un Address, è possibile passarlo
		 * direttamente o specificare solo indirizzo ip e porta
		 * lasciando il compito al costruttore
		 */
		SocketUDP() : Socket(SOCK_DGRAM) {}
		SocketUDP(Address *addr) : Socket(SOCK_DGRAM, addr) {}
		SocketUDP(char *ip, int port) : Socket(SOCK_DGRAM, ip, port) {}
		
		/*
		 * Gestione invio e ricezione
		 */
		bool invia(Address, char*);
		char* ricevi(Address*);
		
		bool enableBroadcast();
};

class Node {
	protected:
		Node* _next = NULL;
	public:
		virtual void* getKey() { return NULL; }
		void set_next(Node *new_next)  { _next = new_next; }
		Node* get_next() { return _next; }
		
		virtual void mostraKey() {};
		
		virtual ~Node();
};

class Iterator;

class Lista {
	private:
		Node *_first;
		void delete_all(Node*);
	
	public:
		Lista() { _first = NULL; }
		~Lista();
		
		Node* getFirst() { return _first; }
			
		void add(Node*);
		bool del(Node*);
	
		void scorri();
		
		Iterator* createIterator();
};

class Iterator {
	private:
		Lista *_lista;
		Node *_nodo;
	public:
		Iterator(Lista *lista) : _lista(lista) { 
#ifdef DEBUG
			printf("Primo elemento\n");
#endif
			goFirst();
		}
		
		Node* goFirst();
		Node* moveNext();
		bool isDone();
		Node* getCurrent();
};

class Connessione {
	protected:
		int _sockId;
		Address *addr;
	public:
		Connessione(int sockId, Address *nAddr)
		    : _sockId(sockId), addr(nAddr) { }
		
		bool invia(char*);
		char* ricevi();
		bool invia(FILE*);

		Address getAddress() { return *addr; }
};

class SocketTCP : public Socket {
	public:
		SocketTCP() : Socket(SOCK_STREAM) {}
		SocketTCP(Address *addr) : Socket(SOCK_STREAM, addr) {}
		SocketTCP(char *ip, int port) : Socket(SOCK_STREAM, ip, port) {}
};

class ConnessioneServer : public Connessione, public Node {
	public:
		ConnessioneServer(int sockId, Address *addr)
			: Connessione(sockId, addr) {}
		
		~ConnessioneServer();
				
		void* getKey();
		void mostraKey();
};

class ServerTCP : public SocketTCP {
	private:
		Lista *connessioni;
	public:		
		ServerTCP(Address*);
		~ServerTCP();
		ServerTCP(char* ip, int port) : ServerTCP(new Address(ip, port)) {}
		
		ConnessioneServer* accetta();
		
		bool close(ConnessioneServer*);
		Iterator* createIterator() { return connessioni->createIterator(); }
};


class ConnessioneClient : public Connessione {
	public:
		ConnessioneClient(int sockId, Address *addr)
			: Connessione(sockId, addr) { }
};

class ClientTCP : public SocketTCP {
	private:
		ConnessioneClient *conn;
	public:

		ClientTCP() : SocketTCP() {}
		~ClientTCP() { delete conn; }
		
		bool connetti(Address*);
		
		bool invia(char*);
		char* ricevi();
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

Address::Address(Address& new_addr) {
	this->setPort(new_addr.getPort());
	this->setIp(new_addr.getIp());
}	
char* Address::toString() {
	char buffer[MAX_STR_IP_PORT];
	char *ip = this->getIp();
	
	sprintf(buffer, "[%s:%d]", ip, this->getPort());
	
	free(ip);
	
	return strdup(buffer);
}

Socket::Socket(int tipo, Address* addr) {
#ifdef DEBUG
	int ret;
#endif
	
	this->_sockId = socket(AF_INET, tipo, 0);
	
#ifdef DEBUG
	printf("Socket: %d\n", this->_sockId);
#endif	
	
	if(addr) {
	
		struct sockaddr_in sAddr = addr->get_sockaddr();
#ifdef DEBUG	
		ret =
#endif
		bind(this->_sockId, (struct sockaddr*)&sAddr, sizeof(struct sockaddr));
	
#ifdef DEBUG
	printf("Bind: %d\n", ret);
#endif
	}
}

Socket::Socket(int tipo, char *ip, int port)
    : Socket(tipo, new Address(ip, port)) {}

bool SocketUDP::invia(Address dest, char *msg) {
	int ret_code;
	
	struct sockaddr_in sAddr = dest.get_sockaddr();
	
	// Lo salvo perchè viene usato in più di un posto
	int len_msg = strlen(msg)+1;
	
	ret_code = sendto(this->_sockId, msg, len_msg, 0,
	                  (struct sockaddr*)&sAddr,
	                  sizeof(struct sockaddr));
	                  
	return ret_code == len_msg;
}

char* SocketUDP::ricevi(Address *mit) {
	char buffer[MAX_STR + 1];
	int ret_code;
	
	struct sockaddr_in sAddr = mit->get_sockaddr();
	
	int len_addr = sizeof(struct sockaddr);
	
	ret_code = recvfrom(this->_sockId, buffer, MAX_STR, 0,
	                    (struct sockaddr*)&sAddr,
	                    (socklen_t*)&len_addr);
	
	if(ret_code <= 0)
		return NULL;
		
	mit->set_sockaddr(sAddr);
	
	buffer[ret_code] = '\0';
	
	return strdup(buffer);
}

bool SocketUDP::enableBroadcast() {
	int val = 1;
	
	return !setsockopt(_sockId, SOL_SOCKET, SO_BROADCAST,
	                   &val, sizeof(int));
}

Node::~Node() {
#ifdef DEBUG
	printf("Distruttore nodo");
#endif
}

void Lista::delete_all(Node *n) {

#ifdef DEBUG
	printf("Elimina nodo\n");
#endif
	if(n->get_next()) {
		this->delete_all(n->get_next());
	}
	delete n;
}

Lista::~Lista() {
#ifdef DEBUG
	printf("Elimina lista\n");
#endif
	delete_all(_first);
}

Iterator* Lista::createIterator() {
	return new Iterator(this);
}

void Lista::scorri() {
	_first->mostraKey();
}

void Lista::add(Node *new_node) {
	new_node->set_next(_first);
	_first = new_node;
}

Node* Iterator::goFirst() {
	return (_nodo = _lista->getFirst());
}
Node* Iterator::moveNext() {
	return (_nodo = _nodo->get_next());
}
bool Iterator::isDone() {

	return (_nodo == NULL);
}
Node* Iterator::getCurrent() {
	return _nodo;
}

bool Connessione::invia(char *msg) {
	int ret_code;
	
	int len_msg = strlen(msg)+1;
	
	ret_code = send(this->_sockId, msg, len_msg, 0);

	return ret_code == len_msg;
}

bool Connessione::invia(FILE *f) {
	char buf[MAX_STR];
	char lettura;
	int i = 0;

	while((buf[i++] = getc(f)) != EOF) {}
	buf[--i] = '\0';

	printf("Invio: %s", buf);

	return this->invia(buf);
}
char* Connessione::ricevi() {
	char buffer[MAX_STR + 1];
	int ret_code;
	
	ret_code = recv(this->_sockId, buffer, MAX_STR, 0);

#ifdef DEBUG
	printf("%d:%s\n", this->_sockId, strerror(errno));
#endif
	
	if(ret_code <= 0)
		return NULL; 
	buffer[ret_code] = '\0';
	
	return strdup(buffer);
}

ConnessioneServer::~ConnessioneServer() {
#ifdef DEBUG
	printf("Chisura sock %d\n", _sockId);
#endif
	close(_sockId);
}

void* ConnessioneServer::getKey() {
	return (void*)&_sockId;
}

void ConnessioneServer::mostraKey() {
	printf("- %d\n", *(int*)this->getKey());
	if(_next)
		_next->mostraKey();
}

ServerTCP::ServerTCP(Address *addr) : SocketTCP(addr) {
	listen(_sockId, MAX_CONN);
	
	connessioni = new Lista();
}

ServerTCP::~ServerTCP() {
#ifdef DEBUG
	printf("Elimina server");
#endif
	delete connessioni;
}

ConnessioneServer* ServerTCP::accetta() {
	Address *addr = new Address();
	ConnessioneServer* conn;
	
	struct sockaddr_in sAddr = addr->get_sockaddr();
	
	socklen_t len = sizeof(struct sockaddr);
	
	int sock = accept(_sockId, (struct sockaddr*)&sAddr, (socklen_t*)&len);
#ifdef DEBUG
	printf("%s\n", strerror(errno));
#endif
	if(sock < 0)
		return NULL;
	addr->set_sockaddr(sAddr);
		
	conn = new ConnessioneServer(sock, addr);
	
	connessioni->add(conn);
	
	return conn;
}

bool ServerTCP::close(ConnessioneServer* conn)
{
	return connessioni->del(conn);
}

bool Lista::del(Node *conn)
{
	Node *prima = NULL;
	Node *nodo = _first;	
	
	while(nodo != NULL && nodo != conn) {
		prima = nodo;
		nodo = nodo->get_next();
	}

	if(nodo == NULL) {
		return false;
	} // Non trovato
	else if(prima == NULL) {
#ifdef DEBUG
		printf("Elimino il primo nodo\n");
#endif
		_first = nodo->get_next();
	}
	else {
#ifdef DEBUG
		printf("Elimino nodo intermedio");
#endif
		prima->set_next(nodo->get_next());
	}

	delete nodo;
	return true;
	
}

bool ClientTCP::connetti(Address *addr) {
	struct sockaddr_in sAddr = addr->get_sockaddr();
	
#ifdef DEBUG
	printf("Connetto\n");

#endif
	conn = new ConnessioneClient(_sockId, addr);
#ifdef DEBUG
	printf("Creata connessione client");
#endif
	return !connect(_sockId, (struct sockaddr*)&sAddr, sizeof(struct sockaddr_in));
}

bool ClientTCP::invia(char *msg) {
	return conn->invia(msg);
}

char* ClientTCP::ricevi() {
	return conn->ricevi();
}

void errore(char *str, int cod) {
	printf("ERROR: %s", str);
	printf("%d:%s\n", errno, strerror(errno));
	printf("Returning %d to system\n", cod);
	exit(cod);
}
#endif
