#include "TcpListeningSocket.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define BACKLOG 100

TcpListeningSocket::TcpListeningSocket() :
	sockfd(0),
	listening(false)
{
	//Recupero il file descriptor (che è un intero) di un socket sfruttando le funzionalità Unix
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		throw SocketError(sockfd);
    }
	int yes = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
		throw "setsockopt(SO_REUSEADDR) failed";
}

TcpListeningSocket::~TcpListeningSocket() {
	if(sockfd > 0)
		close(sockfd);
}


bool TcpListeningSocket::listenTo(const uint16_t port){
	if(listening)
		throw "TcpListeningSocket::listenTo(...) -> socket already in listen state";
	
	struct sockaddr_in myAddr;
	memset(&myAddr, 0x00, sizeof(myAddr));
   
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = INADDR_ANY;
	myAddr.sin_port = htons(port);
   
	if(bind(sockfd, (struct sockaddr *) &myAddr, sizeof(myAddr)) < 0) {
		throw "TcpListeningSocket::listenTo(...) -> errore during bind()";
	}
      
	listen(sockfd,BACKLOG);
	
	listening = true;
	
	return listening;
}

TcpSocket TcpListeningSocket::acceptRequest() {
	struct sockaddr_in requestingAddr;
	socklen_t requestingAddrSize = sizeof(requestingAddr);
	int newsockfd = accept(sockfd, (struct sockaddr *)&requestingAddr, &requestingAddrSize);

	if(newsockfd < 0)
		throw "TcpListeningSocket::acceptRequest() -> error";
	
	TcpSocket newTcpSocket = TcpSocket::fromSockfd(newsockfd);
	
	return newTcpSocket;
}

void TcpListeningSocket::closeSock() {
	if(sockfd > 0){
		close(sockfd);
		sockfd = 0;
	}
	listening = false;
}