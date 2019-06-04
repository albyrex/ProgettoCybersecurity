#ifndef __TCPLISTENINGSOCKET_H__
#define __TCPLISTENINGSOCKET_H__


//TcpSocket gestisce socket tcp per connettersi a socket in ascolto.
//TcpSocket non è per ascoltare su una porta.

#include "TcpSocket.h"

class TcpListeningSocket {
private:
	int sockfd;
	bool listening;
	
public:
	TcpListeningSocket();
	~TcpListeningSocket();
	bool listenTo(const uint16_t port);
	TcpSocket acceptRequest();
	void closeSock();
};


#endif