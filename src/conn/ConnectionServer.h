#ifndef __CONNECTION_H__
#define __CONNECTION_H__


#include <vector>
#include "TcpSocket.h"
#include "DiskManager.h"
#include "Encryptor.h"
#include "Decryptor.h"
#include "HmacCalculator.h"
#include "Connection.h"
#include "KeyManager.h"

#define CONSECUTIVE_ERRORS_THRESHOLD 1

using namespace std;

class Connection;

class ConnectionServer : public Connection {

	DiskManager* diskManager;

private:
	int execCommandFilelist();
	int execCommandUploadFile(Message* mess);
	int execCommandDownloadFile(Message* mess);

public:
	ConnectionServer(const TcpSocket sock, DiskManager* diskManager);
	~ConnectionServer();
	bool startSecureConnection();

};


#endif
