#include "TcpSocket.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <climits>
#include <iostream>

using namespace std;

TcpSocket::TcpSocket() :
	sockfd(0),
	connected(false)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		throw SocketError(sockfd);
    }
}

TcpSocket::~TcpSocket() {
	/*if(sockfd > 0)
		close(sockfd);*/
}

bool TcpSocket::connectTo(const std::string& ip, const uint16_t port) {
	if(connected){
		cerr << "Already connected socket";
		throw "Already connected socket";
	}

	struct sockaddr_in serverAddr;

	memset(&serverAddr, 0x00, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0 ){
		cerr << "Hostname not found";
        throw "Hostname not found";
    }

	int connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(connectionStatus < 0)
		connected = false;
	else
		connected = true;

	return connected;
}

int TcpSocket::send(const void* buffer, const uint32_t size) {
	if(size > INT_MAX){
		cerr << "TcpSocket::send(...) -> size too big";
		throw "TcpSocket::send(...) -> size too big";
	}
	if(!connected){
		cerr << "Impossible to send something using a non connected socket";
		throw "Impossible to send something using a non connected socket";
	}

	int actuallySend = write(sockfd, buffer, size);

	return actuallySend;
}

int TcpSocket::send(const std::vector<byte>& buffer) {
	return send((const byte*)buffer.data(), buffer.size());
}

bool TcpSocket::sendObject(const std::vector<byte>& buffer) {
	uint32_t size = buffer.size();
	if(send((byte*)&size, sizeof(size)) != sizeof(size))
		return false;
	if(send((const byte*)buffer.data(), buffer.size()) != (int)buffer.size())
		return false;
	return true;
}

bool TcpSocket::sendObject(const byte* buffer, const uint32_t size) {
	if(send((byte*)&size, sizeof(size)) != sizeof(size))
		return false;
	if(send(buffer, size) != (int)size)
		return false;
	return true;
}

int TcpSocket::receive(void* buffer, const uint32_t howManyBytes) {
	if(howManyBytes > INT_MAX){
		cerr << "TcpSocket::receive(...) -> howManyBytes too big";
		throw "TcpSocket::receive(...) -> howManyBytes too big";
	}
	if(!isConnected()){
		cerr << "Impossible to send something using a non connected socket";
		throw "Impossible to send something using a non connected socket";
	}

	//int actuallyReceived = read(sockfd, buffer, howManyBytes);
	int actuallyReceived = recv(sockfd, buffer, howManyBytes, MSG_WAITALL);

	return actuallyReceived;
}

std::vector<byte> TcpSocket::receive(const uint32_t howManyBytes) {
	if(howManyBytes > INT_MAX){
		cerr << "TcpSocket::receive(...) -> howManyBytes too big";
		throw "TcpSocket::receive(...) -> howManyBytes too big";
	}
	if(!connected){
		cerr << "Impossible to send something using a non connected socket";
		throw "Impossible to send something using a non connected socket";
	}

	std::vector<byte> receiveVector(howManyBytes);
	int actuallyReceived = receive(receiveVector.data(), howManyBytes);
	if(actuallyReceived < 0){
		cerr << "Error: TcpSocket::receive(...) can not receive correctly";
		return vector<byte>();
	}
	if((uint32_t)actuallyReceived != howManyBytes)
		receiveVector.resize(actuallyReceived);

	return receiveVector;
}

std::vector<byte> TcpSocket::receiveObject() {
	uint32_t size;
	if(receive((byte*)&size, sizeof(size)) != sizeof(size))
		return std::vector<byte>(0);
	std::vector<byte> buffer(size);
	if(receive(buffer.data(), size) != (int)size)
		return std::vector<byte>(0);
	return buffer;
}

void TcpSocket::closeSocket() {
	if(sockfd > 0){
		close(sockfd);
		sockfd = 0;
	}
	connected = false;
}

TcpSocket TcpSocket::fromSockfd(int sockfd) {
	if(sockfd < 0){
		cerr << "TcpSocket::fromSockfd(...) -> ivalid sockfd";
		throw "TcpSocket::fromSockfd(...) -> ivalid sockfd";
	}

	TcpSocket newTcpSocket;
	newTcpSocket.sockfd = sockfd;
	newTcpSocket.connected = true;
	return newTcpSocket;
}

bool TcpSocket::isConnected() {
	return this->connected;
}

int TcpSocket::getErrorStatus() {
	int errorCode;
	socklen_t errorCodeSize = sizeof(errorCode);
	getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &errorCode, &errorCodeSize);
	return errorCode;
}

int TcpSocket::getSockfd() {
	return sockfd;
}

SocketError::SocketError(int sockfd) : resultingSockfd(sockfd){}
