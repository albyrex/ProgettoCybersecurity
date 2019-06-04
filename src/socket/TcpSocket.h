#ifndef __TCPSOCKET_H__
#define __TCPSOCKET_H__


#include <string>
#include <vector>
#include "common.h"
#include "protocol.h"

class TcpSocket {
public:
	int sockfd;
	bool connected;

public:
	TcpSocket();
	~TcpSocket();
	bool connectTo(const std::string& ip, const uint16_t port);
	
	int send(const void* buffer, const uint32_t size);
	int send(const std::vector<byte>& buffer);
	
	bool sendObject(const std::vector<byte>& buffer);
	bool sendObject(const byte* buffer, const uint32_t size);
	
	int receive(void* buffer, const uint32_t howManyBytes);
	std::vector<byte> receive(const uint32_t howManyBytes);
	
	std::vector<byte> receiveObject();

	void closeSocket();
	bool isConnected();
	
	int getErrorStatus();
	int getSockfd();

public:
	static TcpSocket fromSockfd(int sockfd);
};


struct SocketError {
	int resultingSockfd;
	SocketError(int sockfd);
};

enum TcpException {CONNECTION, BOH, BOH2};


#endif
