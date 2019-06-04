#include "ConnectionClient.h"
#include <iostream>


using namespace std;

ConnectionClient::ConnectionClient(const string& ipServer, const uint16_t serverPort) 
	: Connection()
{
	this->ipServer = ipServer;
	this->serverPort = serverPort;
}

ConnectionClient::~ConnectionClient() {
	/*Utilities::eraseMemory(chipherKey, sizeof(chipherKey));
	Utilities::eraseMemory(hashKey, sizeof(hashKey));
	
	if(decryptor)
		delete decryptor;
	if(encryptor)
		delete encryptor;
	if(hashCalculator)
		delete hashCalculator;*/
}

bool ConnectionClient::startSecureConnection() {
	bool connection = this->tcpSocket.connectTo(ipServer, serverPort);
	if(!connection)
		return false;
  
	/*
	  Iniziamo con il protocollo di scambio delle chiavi e autenticazione
	*/
	string pathCert = "./ClientCERT.pem";
	X509* otherCert = NULL;
	if(!exchangeKey(pathCert, &otherCert)) {
		if(otherCert)
			X509_free(otherCert);
		return false;
	}
	
	
	/*
	  Step 8. Se sono arrivato fin qui le chiavi simmetriche sono scambiate
	  e la sessione è iniziata. Ma prima di fare qualunque cosa, ho veramente
	  instaurato la connessione con un peer fidato? La validità del certificato
	  non garantisce per le buene intenzioni del peer.
	  Mentre il server deve controllare che il client sia effettivamente fra
	  quelli autorizzati, il client deve mostrare a video l'identità del server
	  e lasciar decidere l'utente se continuare.
	*/
	Message m;
	m.command = COMMAND_AMIALLOWED;
	sendMessage(&m);
	receiveMessage(&m);
	if(m.command == REPLY_TRUE) {
		cout << "\nConnection established with: " << Cmanager::getSubjectName(otherCert) <<
				"\n!! Continue only if you are sure of the identity of that server !!\n" <<
				"To close immediately type CTRL + C\n\n";
		X509_free(otherCert);
	} else {
		cout << "\nUnable to establish a connection with the server " << Cmanager::getSubjectName(otherCert) <<
		        " because the server refused the connection\n\n";
		X509_free(otherCert);
		return false;
	}
	
	
  
	return true;
}

vector<string> ConnectionClient::filelist() {
	if(!this->tcpSocket.isConnected()){
		cerr << "ConnectionClient::filelist() -> socket not connected\n";
		exit(1);
	}

	Message m;
	m.command = COMMAND_FILELIST;

	if(!sendMessage(&m)) {
		cerr << "Serious connection error: I will terminate the program\n";
		exit(1);
	}

	if(!receiveMessage(&m)) {
		exit(1);
	}
	if(m.command != FILELIST_SIZE) {
		cerr << "Protocol error: FILELIST_SIZE\n";
		exit(1);
	}

	uint32_t filelistSize = *((uint32_t*)m.data);

	vector<string> retFilelist;

	for(uint32_t i = 0; i < filelistSize; ++i) {
		if(!receiveMessage(&m)) {
			exit(1);
		}
		if(m.command != FILE_ELEMENT) {
			cerr << "Protocol error: FILE_ELEMENT\n";
			exit(1);
		}
		string filename((char*)m.data+sizeof(uint64_t));
		filename += " (" + sizeToString(*((uint64_t*)m.data)) + ")";
		retFilelist.push_back(filename);
	}

	/*
	  Se ci sono stati troppi errori consecutivi devo terminare la connessione
	*/
	if(consecutiveErrors > CONSECUTIVE_ERRORS_THRESHOLD) {
		cerr << "Termination of the connection due to excessive consecutive errors\n";
		exit(1);
	}

	/*
	  Se sto usando chiavi troppo vecchie, le cambio.
	*/
	if(keysAreOld()) {
		string pathCert = "./ClientCERT.pem";
		X509* otherCert = NULL;
		if(!exchangeKey(pathCert, &otherCert)) {
			if(otherCert)
				X509_free(otherCert);
			cerr << "Error during key refresh\n";
			exit(1);
		}
		if(otherCert)
			X509_free(otherCert);
		orderNumber = 0;
	}
	
	return retFilelist;
}

bool ConnectionClient::uploadFile(const string& path) {
	ifstream inputStream;
	inputStream.open(path, ios::binary|ios::in);
	if(inputStream.fail()){
		cerr << "File not available" << endl;
		return false;
	}

	uint64_t realFileSize = getFilesize(path);
	
	if(realFileSize > MAX_FILESIZE) {
		cerr << "File too big\n";
		return true;
	}
	
	cout << "Uploading " << realFileSize << " Bytes\n";

	string filename = std::experimental::filesystem::path(path).filename();

	Message mess;
	mess.command = COMMAND_UPLOADFILE;

	uint32_t* ptr = (uint32_t*)(mess.data);
	ptr[0] = realFileSize;
	strcpy((char*)&ptr[1], filename.c_str());

	if(!sendMessage(&mess)) {
		cerr << "Serius connection error: the program will be terminated\n";
		exit(1);
	}

	if(!receiveMessage(&mess)) {
		exit(1);
	}
	if(mess.command == WRONG_FILENAME){
		cerr << "WRONG_FILENAME\n";
		return false;
	}else if(mess.command == SERVER_FULL){
		cerr << "SERVER_FULL\n";
		return false;
	}else if(mess.command == FILE_TOO_BIG){
		cerr << "FILE_TOO_BIG\n";
		return false;
	}

	if(mess.command != REPLY_TRUE){
		cerr << "Error into response\n";
		return false;
	}


	//Adesso procedo a leggere a blocchi e a inviarli con le apposite funzioni
	vector<byte> fileBlock;
	uint32_t daLeggere;
	uint32_t dimRimanente = realFileSize;

	while(dimRimanente > 0) {
		daLeggere = (DEFAULT_BLOCK_SIZE < dimRimanente) ? DEFAULT_BLOCK_SIZE : dimRimanente;
		dimRimanente -= daLeggere;
		fileBlock.resize(daLeggere);
		inputStream.read((char*)fileBlock.data(), daLeggere);
		if(inputStream.fail()){
			cerr << "Error reading the file\n";
			Utilities::eraseMemory(fileBlock.data(),fileBlock.size());
			return false;
		}
		if(!sendFileBlock(fileBlock, (dimRimanente == 0))) {
			cerr << "Error while calling sendFileBlock(...) -> Severe connection error: terminating the program...\n";
			Utilities::eraseMemory(fileBlock.data(),fileBlock.size());
			exit(1);
		}
		Utilities::eraseMemory(fileBlock.data(),fileBlock.size());
		//Caricamento
		float percentage = (((float)realFileSize)-((float)dimRimanente)) / ((float)realFileSize) * 100;
		cout << "Completamento: " << (uint32_t)percentage << "%\r" << std::flush;
	}

	/*
	  Se ci sono stati troppi errori consecutivi devo terminare la connessione
	*/
	if(consecutiveErrors > CONSECUTIVE_ERRORS_THRESHOLD) {
		cerr << "Termination of the connection due to excessive consecutive errors\n";
		exit(1);
	}

	/*
	  Se sto usando chiavi troppo vecchie, le cambio.
	*/
	if(keysAreOld()) {
		string pathCert = "./ClientCERT.pem";
		X509* otherCert = NULL;
		if(!exchangeKey(pathCert, &otherCert)) {
			if(otherCert)
				X509_free(otherCert);
			cerr << "Error during key refresh\n";
			exit(1);
		}
		if(otherCert)
			X509_free(otherCert);
		
		orderNumber = 0;
	}
	
	return true;
}

bool ConnectionClient::downloadFile(const string& filename) {
	//Che nome l'utente vuol dare al file scaricato?
	cout << "What name do you want to give to the file?" << endl;
	string newFileName;
	cin>>newFileName;
	while(std::experimental::filesystem::exists(newFileName)){
		cout << "The file already exists. Please, specify a new name" << endl;
		cin >> newFileName;
	}


	//Mando il comando di download con il filename e ricevo la risposta che controllo
	Message mess;
	mess.command = COMMAND_DOWNLOADFILE;

	uint32_t* ptr = (uint32_t*)(mess.data);
	strcpy((char*)&ptr[0], filename.c_str());

	if(!sendMessage(&mess)) {
		cerr << "Serious connection error: I will terminate the program\n";
		exit(1);
	}

	if(!receiveMessage(&mess)) {
		exit(1);
	}

	if(mess.command == WRONG_FILENAME){
		cerr << "WRONG_FILENAME\n";
		return false;
	}else if(mess.command == FILE_NOT_EXISTS){
		cerr << "FILE_NOT_EXISTS\n";
		return false;
	}

	if(mess.command != REPLY_TRUE){
		cerr << "Error in the response\n";
		return false;
	}


	//Se sono arrivato fin qui è tutto ok
	//Posso procedere a ricavare le informazioni dal messaggio di risposta
	//uint32_t* ptr_ = (uint32_t*)(mess.data);
	//uint32_t realFileSize = ptr_[0];


	//Adesso il ciclo per ricevere tutti i blocchi del file
	vector<byte> fileBlock;
	uint64_t totReceivedBytes = 0;
	int receiveResult = RECEIVE_FILEBLOCK_OK;

	ofstream outputStream; //CONTROLLAREEEEEEEEEEEEE!!!!
	outputStream.open(newFileName, std::ofstream::out | std::ofstream::app);

	while(receiveResult != RECEIVE_FILEBLOCK_LAST) {
		receiveResult = receiveFileBlock(fileBlock);

		if(receiveResult == RECEIVE_FILEBLOCK_ERROR) {
			//Il cerr con l'errore è già stato chiamato. Mi limito a chiudere il programma
			Utilities::eraseMemory(fileBlock.data(),fileBlock.size());
			exit(1);
		}

		outputStream.write((char*)fileBlock.data(), fileBlock.size());
		if(outputStream.fail()){
			cerr << "Error writing the file\n";
			Utilities::eraseMemory(fileBlock.data(),fileBlock.size());
			exit(1);
		}

		totReceivedBytes += fileBlock.size();
		
		if(totReceivedBytes > MAX_FILESIZE) {
			cerr << "Error: I was expecting a file smaller than " << MAX_FILESIZE << "Bytes, but I got " << totReceivedBytes << "Bytes so far (malicious server?)\n"
			        "I'll delete the file!\n";
			outputStream.close();
			std::experimental::filesystem::remove(newFileName);
			Utilities::eraseMemory(fileBlock.data(),fileBlock.size());
			exit(1);
		}
		
		Utilities::eraseMemory(fileBlock.data(),fileBlock.size());
	}

	outputStream.close();

	/*
	  Se ci sono stati troppi errori consecutivi devo terminare la connessione
	*/
	if(consecutiveErrors > CONSECUTIVE_ERRORS_THRESHOLD) {
		cerr << "Termination of the connection due to excessive consecutive errors\n";
		exit(1);
	}

	/*
	  Se sto usando chiavi troppo vecchie, le cambio.
	*/
	if(keysAreOld()) {
		string pathCert = "./ClientCERT.pem";
		X509* otherCert = NULL;
		if(!exchangeKey(pathCert, &otherCert)) {
			if(otherCert)
				X509_free(otherCert);
			cerr << "Error during key refresh\n";
			exit(1);
		}
		if(otherCert)
			X509_free(otherCert);
		orderNumber = 0;
	}
	
	return true;
 }

uint64_t ConnectionClient::getFilesize(string path){
	ifstream in_file(path, std::ios::binary | std::ios::ate);
	if(!in_file.is_open()){
		return 0;
	}
	return in_file.tellg();
}

void ConnectionClient::quit() {
	Message m;
	m.command = COMMAND_QUIT;
	if(!sendMessage(&m)) {
		cerr << "Serious connection error: I will terminate the program\n";
		exit(1);
	}
}

string ConnectionClient::sizeToString(uint64_t size) {
	if(size < 1024)
		return std::to_string(size) + " B";
	size /= 1024;
	if(size < 1024)
		return std::to_string(size) + " KiB";
	size /= 1024;
	if(size < 1024)
		return std::to_string(size) + " MiB";
	size /= 1024;
	return std::to_string(size) + " GiB";
}
