#ifndef __SERVER_H__
#define __SERVER_H__


#include <vector>
#include "common.h"
#include "protocol.h"
#include "TcpListeningSocket.h"
#include "DiskManager.h"

using namespace std;

class Server {

private:
	TcpListeningSocket* tcpListeningSocket;
	bool status;
	uint16_t port;
	bool mustContinue;
	DiskManager* diskManager;
	mutex threadsPoolMutex;
	vector<thread*> threadsPool;
	Server(const uint16_t PORT);

public:
	void connectionManager(TcpSocket tcpSock);

public:
	~Server();
	void switchOn();
	void switchOff();
	
public:
	static Server* getTheServer();
	
};


#endif
