#include "ConnectionServer.h"
#include <iostream>
#include <string.h>

using namespace std;

ConnectionServer::ConnectionServer(const TcpSocket sock, DiskManager* diskManager)
	: Connection()
{
	this->tcpSocket = sock;
	this->diskManager = diskManager;
}

ConnectionServer::~ConnectionServer() {
	/*Utilities::eraseMemory(chipherKey, sizeof(chipherKey));
	Utilities::eraseMemory(hashKey, sizeof(hashKey));

	if(decryptor)
		delete decryptor;
	if(encryptor)
		delete encryptor;
	if(hashCalculator)
		delete hashCalculator;*/
}

bool ConnectionServer::startSecureConnection() {
	/*
	  Iniziamo con il protocollo di scambio delle chiavi e autenticazione
	*/
	string pathCert = "./ServerCERT.pem";
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
	string connectingClient = Cmanager::getSubjectName(otherCert);
	X509_free(otherCert);
	Message m;
	receiveMessage(&m);
	if(m.command != COMMAND_AMIALLOWED) {
		cout << "protocol error. Connection closed.\n";
		return false;
	}
	cout << "----------------------------------------------\n";
	cout << "Connection established with: " << connectingClient <<
	        "\nVerification of credentials in progress...\n";

	fstream listFile;
	listFile.open("./allowedClients.txt", std::ios::in);
	string currentLine;
	bool allowed = false;
	while(getline(listFile,currentLine)) {
		if(currentLine == "" || currentLine == "\r")
			continue;
		if(currentLine[currentLine.size()-1] == '\r')
			currentLine.erase(currentLine.size()-1);
		if(connectingClient == currentLine) {
			allowed = true;
			break;
		}
	}

	if(!allowed) {
		cerr << "The client that is trying to connect is not authorized\n";
		cout << "----------------------------------------------\n\n";
		m.command = REPLY_FALSE;
		sendMessage(&m);
		return false;
	}
	cout << "The client is authorized to communicate\n";
	cout << "----------------------------------------------\n\n";
	m.command = REPLY_TRUE;
	sendMessage(&m);



	/*
	  Adesso gestione dei comandi
	*/
	Message mess;
	int commandStatus;
	while(true){
		
		Utilities::eraseMemory(&mess, sizeof(mess));
		
		mess.command = COMMAND_QUIT; //Non togliere!

		if(!receiveMessage(&mess)) {
			cout << "A connection to a client is terminated prematurely. I'm closing it.\n\n";
			return false;
		}

		switch (mess.command) {
			case COMMAND_UPLOADFILE:
				cout << "Received a message with command COMMAND_UPLOADFILE\n";
				commandStatus = execCommandUploadFile(&mess);
				cout << "COMMAND_UPLOADFILE management finished\n\n";
				break;
			case COMMAND_DOWNLOADFILE:
				cout << "Received a message with command COMMAND_DOWNLOADFILE\n";
				commandStatus = execCommandDownloadFile(&mess);
				cout << "COMMAND_DOWNLOADFILE management finished\n\n";
				break;
			case COMMAND_FILELIST:
				cout << "Received a message with command COMMAND_FILELIST\n";
				commandStatus = execCommandFilelist();
				cout << "COMMAND_FILELIST management finished\n\n";
				break;
			case COMMAND_QUIT:
				cout << "Terminated connection with a client\n\n";
				return true;
				break;
			default:
				cerr << "Consistent message received, but with an unrecognized command. "
				        "This is likely to be a malicious client. "
						"The message will be ignored and the communication terminated.\n\n";
				return false;
				break;
		}
		
		if(commandStatus == CRITICAL_ERROR) {
			cerr << "The execution of the last command failed with a connection error: "
			        "I will terminate the connection\n\n";
			return false;
		} else if(commandStatus == NEGLIGIBLE_ERROR) {
			cout << "The execution of the last command ended with a non-critical error that "
			        "was handled correctly: this is probably a user error\n\n";
		}
		
		/*
		  Se non ci sono stati errori critici, ma ci sonotroppi errori consecutivi,
		  devo terminare la connessione
		*/
		if(consecutiveErrors > CONSECUTIVE_ERRORS_THRESHOLD) {
			cerr << "Termination of the connection due to excessive consecutive errors\n";
			return false;
		}
		
		/*
		  Se sto usando chiavi troppo vecchie, le cambio.
		*/
		if(keysAreOld()) {
			string pathCert = "./ServerCERT.pem";
			X509* otherCert = NULL;
			if(!exchangeKey(pathCert, &otherCert)) {
				if(otherCert)
					X509_free(otherCert);
				cerr << "Error during key refresh\n";
				return false;
			}
			if(otherCert)
				X509_free(otherCert);
			orderNumber = 0;
		}

	}
	return true;
}

int ConnectionServer::execCommandFilelist() {
	vector<FilelistElement> filelist = diskManager->getFilelist();
	Message m;
	m.command = FILELIST_SIZE;
	*((uint32_t*)m.data) = filelist.size();
	if(!sendMessage(&m))
		return CRITICAL_ERROR;

	m.command = FILE_ELEMENT;
	for(uint32_t i = 0; i < filelist.size(); ++i) {
		*((uint64_t*)m.data) = filelist[i].filesize;
		strcpy((char*)m.data+sizeof(uint64_t), filelist[i].filename.c_str());
		if(!sendMessage(&m))
			return CRITICAL_ERROR;
	}
	
	return OK;
}

//Io server che ricevo dal client
int ConnectionServer::execCommandUploadFile(Message* mess){
	//Recupero le informazioni dal messaggio
	uint32_t* ptr = (uint32_t*)(mess->data);
	uint32_t realFileSize = ptr[0];
	//controllare presenza fine stringa (però con lunghezza limitata sei a posto)
	mess->data[sizeof(mess->data)-1] = '\0';
	string filename((char*)&ptr[1]);


	//Mi sta bene o no questo file? Rispondo al client
	Message response;

	if(
		filename.length() > MAX_FILENAME_LENGTH ||
		filename.find("..") != string::npos ||
		filename.find("/") != string::npos
	){
		//nome non corretto
		response.command = WRONG_FILENAME;
		if(!sendMessage(&response))
			return CRITICAL_ERROR;
		return NEGLIGIBLE_ERROR;
	}else if(realFileSize > MAX_FILESIZE){
		//file troppo grosso
		response.command = FILE_TOO_BIG;
		if(!sendMessage(&response))
			return CRITICAL_ERROR;
		return NEGLIGIBLE_ERROR;
	}

	response.command = REPLY_TRUE;
	if(!sendMessage(&response))
		return CRITICAL_ERROR;


	//Se sono arrivato qui è andato tutto ok.
	//Mi preparo a ricevere il file a blocchi. Sarà la receiveFileBlock(...) a dirmi quando ho ricevuto l'ultimo blocco
	FILE* fp = this->diskManager->openFile(filename, SAVING);
	if(!fp) {
		cerr << "Error opening the file\n";
		this->diskManager->closeFile(fp, filename, SAVING);
		return NEGLIGIBLE_ERROR;
	}
	vector<byte> fileBlock;
	int receiveResult = RECEIVE_FILEBLOCK_OK;
	uint64_t totReceivedBytes = 0;
	while(receiveResult != RECEIVE_FILEBLOCK_LAST) {
		receiveResult = receiveFileBlock(fileBlock);

		if(receiveResult == RECEIVE_FILEBLOCK_ERROR) {
			this->diskManager->errorOn(fp, filename, SAVING);
			return NEGLIGIBLE_ERROR;
		}

		//usa diskManager per scrivere fileBlock, se non si riesco si deve chiudere immediatamente la connessione
		if(!this->diskManager->write(fp, fileBlock)) {
			cerr << "Disk write error detected: connection terminated\n";
			this->diskManager->errorOn(fp, filename, SAVING);
			return CRITICAL_ERROR;
		}

		totReceivedBytes += fileBlock.size();
		
		Utilities::eraseMemory(fileBlock.data(), fileBlock.size());
		
		if(totReceivedBytes > realFileSize) {
			cerr << "Error: I was expecting a file " << realFileSize << "Bytes big, but I got " << totReceivedBytes << "Bytes so far (malicious client?)\n";
			this->diskManager->errorOn(fp, filename, SAVING);
			return CRITICAL_ERROR;
		}
	}

	if(!this->diskManager->finalizeFile(fp, filename)){
		cerr  << "Errore nella finalizeFile\n";
		return NEGLIGIBLE_ERROR;
	}
	
	return OK;
}

//Io server che invio al client
int ConnectionServer::execCommandDownloadFile(Message* mess){
	//Leggo il messaggio e recupero le informazioni
	uint32_t* ptr = (uint32_t*)(mess->data);
	//controllare presenza filestringa
	mess->data[sizeof(mess->data)-1] = '\0';
	string filename((char*)&ptr[0]);


	//Adesso mando la mia risposta al client
	Message response;

	if(
		filename.length() > MAX_FILENAME_LENGTH ||
		filename.find("..") != string::npos ||
		filename.find("/") != string::npos
	){
		response.command = WRONG_FILENAME;
		cout << "WRONG_FILENAME" << endl;
		if(!sendMessage(&response))
			return CRITICAL_ERROR;
		return NEGLIGIBLE_ERROR;
	}

	FILE* fp = this->diskManager->openFile(filename, READING);
	if(!fp){
		response.command = FILE_NOT_EXISTS;
		cout << "FILE_NOT_EXISTS" << endl;
		this->diskManager->closeFile(fp,filename,READING);
		if(!sendMessage(&response))
			return CRITICAL_ERROR;
		return NEGLIGIBLE_ERROR;
	}


	//Se sono arrivato qui significa che posso rispondere al client con le informazioni
	//sul file che sta per scaricare salvo successivi errori
	uint32_t realFileSize = 0;
	uint32_t* ptr_ = (uint32_t*)(response.data);
	ptr_[0] = realFileSize;
	response.command = REPLY_TRUE;
	if(!sendMessage(&response)){
		this->diskManager->closeFile(fp,filename,READING);
		return CRITICAL_ERROR;
	}


	//Se sono arrivato qui significa che posso procedere a leggere a blocchi il file
	//e a inviarlo al client
	int read;

	vector<byte> fileBlock;

	while(true) {
		fileBlock.resize(DEFAULT_BLOCK_SIZE);
		read = this->diskManager->read(fileBlock, fp, DEFAULT_BLOCK_SIZE);

		if(read < 0) {
			this->diskManager->closeFile(fp,filename,READING);
			cerr << "Error 1\n";
			return NEGLIGIBLE_ERROR;
		}
		fileBlock.resize(read);

		if(!sendFileBlock(fileBlock, (read < DEFAULT_BLOCK_SIZE))){
			//errore nell'invio
			this->diskManager->closeFile(fp,filename,READING);
			cerr << "Error from sendFileBlock(...)\n";
			Utilities::eraseMemory(fileBlock.data(), fileBlock.size());
			return CRITICAL_ERROR;
		}

		Utilities::eraseMemory(fileBlock.data(), fileBlock.size());

		if(read < DEFAULT_BLOCK_SIZE)
			break;
	}


	//Ho finito e posso chiudere il file
	this->diskManager->closeFile(fp,filename,READING);
	
	return OK;
}

