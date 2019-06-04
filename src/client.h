#ifndef __CLIENT_H__
#define __CLIENT_H__


/*
 * File finito e controllato 16-04-2019 9:53
*/

#include "common.h"
#include "protocol.h"
#include "TcpSocket.h"
#include "ConnectionClient.h"

using namespace std;

class Client {

	ConnectionClient* connectionClient;

 public:
 	Client(const string& ipServer, const uint16_t port);
 	~Client();

	//Funzione che avvia l'interfaccia del client eback
	//inizia la processazione dei comandi
	void run();
};


#endif
