#include "server.h"
#include "common.h"
#include "protocol.h"
#include <thread>
#include "ConnectionServer.h"
#include <iostream>
#include <signal.h>


//Parte per la gestione dei thread
void bridge(TcpSocket tcpSock) {
	Server::getTheServer()->connectionManager(tcpSock);
}


void serverExit(int) {
	cout << "\n\nSIGINT or SIGTERM -> Closing the server\n\n";
	Server::getTheServer()->switchOff();
	delete Server::getTheServer();
	exit(0);
}

Server::Server(const uint16_t port) {
	this->status = false;
	this->port = port;
	this->mustContinue = true;
	this->tcpListeningSocket = new TcpListeningSocket();
	this->diskManager = new DiskManager();
	signal(SIGTERM,serverExit);
	signal(SIGINT,serverExit);
	vector<FilelistElement> filelist = this->diskManager->getFilelist();
	cout << "Available files:\n";
	for(uint32_t i = 0; i < filelist.size(); ++i) {
		cout << i << ". " << filelist[i].filename << endl;
	}
	cout << endl;
}

Server::~Server() {
	if(this->tcpListeningSocket)
		delete this->tcpListeningSocket;
	if(this->diskManager)
		delete this->diskManager;
}

void Server::switchOn() {
	if(status)
		return;
	status = true;
	try {
		this->tcpListeningSocket->listenTo(this->port);
	}
	catch(const char* e) {
		cerr << e;
		exit(1);
	}
	while(this->mustContinue){
		//Soluzione multithread
		thread* newThread = new thread(
			bridge,
			this->tcpListeningSocket->acceptRequest()
		);
		threadsPoolMutex.lock();
		threadsPool.push_back(newThread);
		threadsPoolMutex.unlock();
	}
}

void Server::switchOff() {
	if(!status)
		return;
	mustContinue = false;
	threadsPoolMutex.lock();
	while(threadsPool.size() > 0) {
		thread* t = threadsPool.back();
		threadsPool.pop_back();
		delete t;
	}
	threadsPoolMutex.unlock();
	status = false;
}

void Server::connectionManager(TcpSocket tcpSock) {
	//Adesso ho il nuovo TcpSocket: lo uso per instanziare una connessione.
	ConnectionServer conn(tcpSock, this->diskManager);
	conn.startSecureConnection(); //bloccante

	std::cout << "Connection terminated\n\n";

	//Se sono arrivato qui vuol dire che la connessione è terminata
	tcpSock.closeSocket();

	//Con il termine di questa funzione termina anche il thread
	threadsPoolMutex.lock();
	thread* toDelete = NULL;
	for(uint32_t i = 0; i < threadsPool.size(); ++i) {
		if(threadsPool[i]->get_id() == std::this_thread::get_id()) {
			toDelete = threadsPool[i];
			threadsPool[i] = threadsPool[threadsPool.size()-1];
			threadsPool.pop_back();
			break;
		}
	}
	if(!toDelete) {
		cerr << "Error: thread management\n";
		exit(1);
	}
	threadsPoolMutex.unlock();
	toDelete->detach();
	delete toDelete;
}

Server* Server::getTheServer() {
	static Server* theServer = 0;
	if(theServer == 0) {
		theServer = new Server(SERVER_PORT);
	}
	return theServer;
}
