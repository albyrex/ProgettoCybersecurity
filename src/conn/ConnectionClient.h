#ifndef __CONNECTIONCLIENT_H__
#define __CONNECTIONCLIENT_H__


#include "common.h"
#include "protocol.h"
#include "TcpSocket.h"
#include <experimental/filesystem>
#include <string>
#include <fstream>
#include <string.h>
#include "Encryptor.h"
#include "Decryptor.h"
#include "HmacCalculator.h"
#include "Connection.h"
#include "KeyManager.h"

#define CONSECUTIVE_ERRORS_THRESHOLD 128

using namespace std;

class Connection;

class ConnectionClient : public Connection {

	string ipServer;
	uint16_t serverPort;

private:
	uint64_t getFilesize(string path);
	inline string sizeToString(uint64_t size);

public:
	ConnectionClient(const string& ipServer, const uint16_t serverPort);
	~ConnectionClient();
	bool startSecureConnection();
	vector<string> filelist();
	bool uploadFile(const string& path);
	bool downloadFile(const string& path);
	void quit();

};


#endif
