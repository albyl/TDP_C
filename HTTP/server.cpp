#include "..\socket.hpp"
#define HTTP_200 "HTTP/1.0 200 OK\
\
"
#define HTTP_404 "HTTP/1.0 404 Not Found\n\n"

int main(int argc, char* argv[]){
	int port;
	char* home_page;
	ServerTCP* myself;
	Address* my_addr;

	if(argc!=3){ return(-1);}

	port=atoi(argv[1]);
	home_page=argv[2];

	my_addr= new Address(SERVER_IP,port);

	myself = new ServerTCP(my_addr);

	/*Server Http*/
	ConnessioneServer* conn;
    	conn = myself->accetta();
	if(conn){
        	char* msg;
        	msg = conn->ricevi();
        	if(msg){
            		char* client = conn->getAddress().toString();
            		printf("[%s]%s\n", client, msg);

            		FILE* file_home_page;
			file_home_page = fopen(home_page, "r");
            		if(!file_home_page){
                		conn->invia(HTTP_404);
            		}
			else{
				printf("Invio pagina\n");
                		conn->invia("HTTP/1.1 200 OK\n\n");
                		//conn->invia("<html>Ciao</html>\0");
				conn->invia(file_home_page);
                		fclose(file_home_page);
            		}

            		myself->close(conn);
        	}
    	}
	printf("Mi chiudo");

	delete(myself);
	printf("Chiudo addr");
	delete(my_addr);
	return(0);
}
