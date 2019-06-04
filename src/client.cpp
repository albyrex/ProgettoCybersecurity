#include "client.h"
#include <iostream>
#include <string.h>
#include <experimental/filesystem>

#define HELP_CONNECTION_FAILED "Error: connection failed!\n"

using namespace std;

Client::Client(const string& ipServer, const uint16_t port) {
	this->connectionClient = new ConnectionClient(ipServer, port);
}

Client::~Client() {
	if(connectionClient)
		delete connectionClient;
}

void Client::run() {
	//La clientConnection va avviata (bisogna iniziare la comunicazione
	//istanziando le chiavi, ecc)
	bool connected = connectionClient->startSecureConnection();

	if(!connected) {
		cerr << HELP_CONNECTION_FAILED;
		exit(1);
	}

	//Adesso devo realizzare l'interfaccia utente che interpreterà i comandi
	//digitati dall'utente e saprà quali funzioni di connectionClient chiamare
	string userCommand;
	bool mustContinue = true;
	while(mustContinue){
		cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
		cout << "Welcome into SecSTOR: the secure storing system!\n\n";
		cout << " ____________________________________________________\n";
		cout << "|                                                    |\n";
		cout << "| 1) Show the file list    : write \"filelist\"        |\n";
		cout << "| 2) Upload a file         : write \"upload\"          |\n";
		cout << "| 3) Download a file       : write \"download\"        |\n";
		cout << "| 4) Exit from the program : write \"exit\"            |\n";
		cout << "|____________________________________________________|\n";

		while(mustContinue){
			cout << "\nSelect a command: ";
			cin >> userCommand;
			if(userCommand == "filelist") {

				cout << "Available file on the server:\n";
				vector<string> files = connectionClient->filelist();
				for(uint32_t i = 0; i < files.size(); ++i) {
					cout << i << ". " << files[i] << "\n";
				}

			} else if(userCommand == "download") {

				string filename;
				cout << "Type the filename: ";
				cin >> filename;
				cout << "Download request for: " << filename << "\n";
				bool success = connectionClient->downloadFile(filename);
				cout << "Outcome of the operation: " << (success ? "success\n" : "failure\n");

			} else if(userCommand == "upload") {
				//Ho modificcato solo una cosina

				for (const auto & entry : std::experimental::filesystem::directory_iterator("./")){
					 string pathFile = entry.path().filename();
					 cout << pathFile << "\n";
				}

				string filepath;
				cout << "Type the filepath: ";
				cin >> filepath;
				cout << "Upload request for: " << filepath << "\n";
				bool success = connectionClient->uploadFile(filepath);
				cout << "Outcome of the operation: " << (success ? "success\n" : "failure\n");

			} else if(userCommand == "exit") {

				connectionClient->quit();
				mustContinue = false;

			} else {

				cout << "Unrecognized command\n";

			}
		}
		cout << "\nBye bye!\n";
	}
}
