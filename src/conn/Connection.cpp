#include "Connection.h"
#include "Utilities.h"
#include "Random.h"
#include "SimpleHash.h"
#include <iostream>
#include <string.h>
#include <openssl/rand.h>

#define IV_SIZE 16

using namespace std;

Connection::Connection() {
	orderNumber = 0;
	decryptor = NULL;
	encryptor = NULL;
	consecutiveErrors = 0;
}

Connection::~Connection() {
	Utilities::eraseMemory(chipherKey, sizeof(chipherKey));
	Utilities::eraseMemory(hashKey, sizeof(hashKey));

	if(decryptor)
		delete decryptor;
	if(encryptor)
		delete encryptor;
}

bool Connection::receiveMessage(Message* mess) {
	uint32_t expectedOrder = getOrder();
	vector<byte> currentIv = tcpSocket.receive(IV_SIZE);

	if(currentIv.size() != (int)IV_SIZE){
		cerr << "Connection error during the reception of the IV (Message). Cannot receive the message. (1)\n\n";
		++consecutiveErrors;
		return false;
	}

	vector<byte> crypted = tcpSocket.receive(C_MESSAGE_SIZE);

	if(crypted.size() != C_MESSAGE_SIZE){
		cerr << "Connection error during the reception of the message. Cannot receive the message. (2)\n\n";
		++consecutiveErrors;
		return false;
	}

	vector<byte> hashReceived = this->tcpSocket.receive(SHA256_HASH_SIZE);
	if(hashReceived.size() != SHA256_HASH_SIZE){
		cerr << "Connection error during the reception of the message. Cannot receive the message. (3)\n\n";
		++consecutiveErrors;
		return false;
	}

	HmacCalculator hc(this->hashKey);
	hc.updateHash(currentIv);
	hc.updateHash(crypted);
	vector<byte> hashCalculated = hc.getHash();

	if(!(hashReceived == hashCalculated)){
		cerr << "HMAC inconsistency: tampering attempt detected. Cannot receive the message. (4)\n\n";
		++consecutiveErrors;
		return false;
	}

	//Ricorda: i parametri sono: key, source, sourceSize, dest
	uint32_t decrLen = Decryptor::staticDecrypt(this->chipherKey, currentIv.data(), crypted.data(), crypted.size(), mess);

	if(decrLen != sizeof(Message)){
		cerr << "Error: plaintext has a different size" << endl;
		++consecutiveErrors;
		return false;
	}


	if(mess->order != expectedOrder) {
		cerr << "Wrong order number\n";
		cerr << "  mess->order: "<<mess->order<<endl;
		cerr << "  expectedOrder: "<<expectedOrder<<endl;
		stepBackOrder();
		++consecutiveErrors;
		return false;
	}

	consecutiveErrors = 0;
	return true;
}

bool Connection::sendMessage(Message* mess) {
	mess->order = getOrder();
	vector<byte> currentIv = getIv();

	if(tcpSocket.send(currentIv) != (int)currentIv.size()){
		cerr << "Connection::sendMessage(...) -> error (0)\n";
		++consecutiveErrors;
		return false;
	}

	vector<byte> crypted(C_MESSAGE_SIZE);
	uint32_t crLen = Encryptor::staticEncrypt(this->chipherKey, currentIv.data(), mess, sizeof(Message), crypted.data());

	if(crLen != C_MESSAGE_SIZE) {
		cerr << "Connection::sendMessage(...) -> error (1)\n";
		++consecutiveErrors;
		return false;
	}

	if(tcpSocket.send(crypted) != C_MESSAGE_SIZE){
		cerr << "Connection::sendMessage(...) -> error (2)\n";
		++consecutiveErrors;
		return false;
	}

	HmacCalculator hc(this->hashKey);
	hc.updateHash(currentIv);
	hc.updateHash(crypted);
	vector<byte> hash = hc.getHash();
	if((int)hash.size() != this->tcpSocket.send(hash)){
		cerr << "Connection::sendMessage(...) -> error (3)\n";
		++consecutiveErrors;
		return false;
	}

	consecutiveErrors = 0;
	return true;
}

uint32_t Connection::getOrder() {
	return orderNumber++;
}

bool Connection::sendFileBlock(vector<byte>& fileBlock, bool lastPart) {
	FileBlockDescriptor fbd;
	fbd.order = getOrder();
	fbd.flag = (lastPart == true) ? FILEBLOCK_LAST : FILEBLOCK_ELEMENT;
	
	if(this->encryptor == NULL) {
		vector<byte> currentIv = getIv();
		this->encryptor = new Encryptor(this->chipherKey, currentIv.data());
		memcpy(fbd.blockIV, currentIv.data(), currentIv.size());
	}
	
	vector<byte> crypted = encryptor->encrypt(fileBlock);

	if(lastPart){
		vector<byte> finalCrypt = encryptor->finalize();
		crypted.insert(crypted.end(), finalCrypt.begin(), finalCrypt.end());
		delete this->encryptor;
		this->encryptor = NULL;
	}
	
	fbd.blockDim = crypted.size();
	
	if(tcpSocket.send(&fbd, sizeof(fbd)) != sizeof(fbd)) {
		cerr << "Connection::sendFileBlock(...) -> error (1)\n";
		++consecutiveErrors;
		return false;
	}
	
	vector<byte> fbdHmac = HmacCalculator::staticHash(hashKey, &fbd, sizeof(fbd));
	if(tcpSocket.send(fbdHmac.data(), SHA256_HASH_SIZE) != SHA256_HASH_SIZE) {
		cerr << "Connection::sendFileBlock(...) -> error (2)\n";
		++consecutiveErrors;
		return false;
	}
	
	if(tcpSocket.send(crypted.data(), crypted.size()) != (int)crypted.size()) {
		cerr << "Connection::sendFileBlock(...) -> error (3)\n";
		++consecutiveErrors;
		return false;
	}
	
	HmacCalculator hc(hashKey);
	hc.updateHash(&fbd,sizeof(fbd));
	hc.updateHash(fbdHmac);
	hc.updateHash(crypted);
	vector<byte> hmac = hc.getHash();
	if(tcpSocket.send(hmac.data(), SHA256_HASH_SIZE) != SHA256_HASH_SIZE) {
		cerr << "Connection::sendFileBlock(...) -> error (4)\n";
		++consecutiveErrors;
		return false;
	}
	
	consecutiveErrors = 0;
	
	return true;
}

int Connection::receiveFileBlock(vector<byte>& dest) {
	//Ricezione del FileBlockDescriptor
	uint32_t expectedOrder = getOrder();
	FileBlockDescriptor fbd;
	if(tcpSocket.receive(&fbd,sizeof(fbd)) != sizeof(fbd)) {
		cerr << "receiveFileBlock(...) -> error (1)\n";
		++consecutiveErrors;
		return RECEIVE_FILEBLOCK_ERROR;
	}
	if(fbd.order != expectedOrder) {
		cerr << "receiveFileBlock(...) -> error (2)\n";
		++consecutiveErrors;
		return RECEIVE_FILEBLOCK_ERROR;
	}
	
	//Ricezione e comparazione del tag hmac del FileBlockDescriptor
	vector<byte> receivedFbdHmac(SHA256_HASH_SIZE);
	if(tcpSocket.receive(receivedFbdHmac.data(),SHA256_HASH_SIZE) != SHA256_HASH_SIZE) {
		cerr << "receiveFileBlock(...) -> error (3)\n";
		++consecutiveErrors;
		return RECEIVE_FILEBLOCK_ERROR;
	}
	vector<byte> calculatedFbdHmac = HmacCalculator::staticHash(hashKey,&fbd,sizeof(fbd));
	
	if(receivedFbdHmac != calculatedFbdHmac) {
		cerr << "receiveFileBlock(...) -> error (4) -> Tamperin attempt detected\n";
		++consecutiveErrors;
		return RECEIVE_FILEBLOCK_ERROR;
	}
	
	//Controllo della validità della dimensione e ricezione del fileblock
	if(fbd.blockDim > DEFAULT_BLOCK_SIZE + 32) {
		cerr << "receiveFileBlock -> error (5)" << endl;
		++consecutiveErrors;
		return RECEIVE_FILEBLOCK_ERROR;
	}
	
	vector<byte> crypted(fbd.blockDim);
	if(tcpSocket.receive(crypted.data(),crypted.size()) != (int)crypted.size()) {
		cerr << "receiveFileBlock -> error (6)" << endl;
		++consecutiveErrors;
		return RECEIVE_FILEBLOCK_ERROR;
	}
	
	//Controllo del tag hmac
	HmacCalculator hc(hashKey);
	hc.updateHash(&fbd,sizeof(fbd));
	hc.updateHash(receivedFbdHmac);
	hc.updateHash(crypted);
	vector<byte> calculatedHmac = hc.getHash();
	vector<byte> receivedHmac(SHA256_HASH_SIZE);
	if(tcpSocket.receive(receivedHmac.data(), SHA256_HASH_SIZE) != SHA256_HASH_SIZE) {
		cerr << "receiveFileBlock -> error (7)" << endl;
		++consecutiveErrors;
		return RECEIVE_FILEBLOCK_ERROR;
	}
	
	//Operazioni per decifrare il fileblock
	if(this->decryptor == NULL) {
		this->decryptor = new Decryptor(this->chipherKey, fbd.blockIV);
	}
	dest = decryptor->decrypt(crypted);
	if(fbd.flag == FILEBLOCK_LAST) {
		vector<byte> final = decryptor->finalize();
		dest.insert(dest.end(), final.begin(), final.end());
		delete this->decryptor;
		this->decryptor = NULL;
	}
	
	consecutiveErrors = 0;
	
	return (fbd.flag == FILEBLOCK_LAST ? RECEIVE_FILEBLOCK_LAST : RECEIVE_FILEBLOCK_OK);
}

vector<byte> Connection::getIv() {
	vector<byte> ret(IV_SIZE);

	int success;
	do{
		success = RAND_bytes(ret.data(), IV_SIZE);
	}while(success != 1);

	return ret;
}

void Connection::stepBackOrder() {
	--orderNumber;
}

bool Connection::keysAreOld() {
	return (orderNumber >= KEYS_LIFESPAN);
}

bool Connection::exchangeKey(const string& pathMyCert, X509** otherCert) {
	/*
	   Step 1.
	     1. Creazione sessione Diffie-Hellman
		 2. Invio di Ya
		 3. Ricezione di Y_other
	*/
	vector<byte> Ya;
	vector<byte> a;
	DH* dh_session = KeyManager::generateDHPair_3072(Ya, a);
	if(!tcpSocket.sendObject(Ya)) {
		cerr << "Error: can not send Ya" << endl;
		return false;
	}

	vector<byte> Y_other = tcpSocket.receiveObject();
	if(Y_other.size() == 0) {
		cerr << "Can not receive Y" << endl;
		return false;
	}


	/*
	   Step 2. La Ya (chiave pubblica) appena mandata deve essere firmata insieme alla Y_other,
	   perciò creo una firma e la mando.
	   La firma deve essere fatta con la chiave privata che è archivita in un file pem
	   sul disco, quindi prima leggo il file e poi firmo.
	   Distruggo la chiave privata il prima possibile.
	*/
	vector<byte> toSign = Ya;
	vector<byte> toVerify = Y_other;
	
	toSign.insert(toSign.end(), Y_other.begin(), Y_other.end());
	toVerify.insert(toVerify.end(), Ya.begin(), Ya.end());
	
	string pathPRIV = "./priv.pem";
	EVP_PKEY* my_priv = KeyManager::loadPRIV(pathPRIV);
	vector<byte> sign = Signer::staticSign(my_priv, toSign);
	EVP_PKEY_free(my_priv);
	if(!tcpSocket.sendObject(sign)) {
		cerr << "Error: can not send the sign of Y" << endl;
		return false;
	}


	/*
	  Step 3. Segue l'invio del proprio certificato. Tale certificato è presente sul
      disco alla path pathMyCert. Quindi prima leggo il certificato, poi lo serializzo
	  ed infine lo invio. Appena finito lo distruggo.
	*/
	X509* myCert = Cmanager::loadCERT(pathMyCert);
	vector<byte> serializedCert = Cmanager::serialize(myCert);
	X509_free(myCert);
	if(!tcpSocket.sendObject(serializedCert)) {
		cerr << "Error: can not send my certificate" << endl;
	}


	/*
	  Step 4. Ricevo:
	    1. la chiave pubblica Diffie-Hellman dell'altro peer,
		2. la firma che l'altro peer ha fatto per firmare tale chiave,
		3. il certificato dell'altro peer che mi sta mandando.
	*/


	vector<byte> sign_other = tcpSocket.receiveObject();
	if(sign_other.size() == 0) {
		cerr << "Can not receive the sign of Y" << endl;
		return false;
	}

	vector<byte> serializedOtherCert = tcpSocket.receiveObject();
	if(serializedOtherCert.size() == 0) {
		cerr << "Can not receive the certificate" << endl;
		return false;
	}


	/*
	  Step 5. Il certificato deserializzato e poi verificato. Se non
	  valido annullo tutto, se valido, continuo.
	  Inoltre dal certificato appena ricevuto (se valido), estraggo la chiave
	  pubblica dell'altro peer e la uso per verificare la firma che ho ricevuto
	  allo step precedente.
	*/
	*otherCert = Cmanager::deserialize(serializedOtherCert);
	bool validCert = this->cmanager.verify(*otherCert);

	if(!validCert) {
		cerr << "Invalid certificate\n";
		return false;
	}

	EVP_PKEY* pub_other = Cmanager::getPUBKEY(*otherCert);
	if(!Verifier::staticVerification(pub_other, toVerify, sign_other)) {
		cerr << "Error during sign verification\n";
		EVP_PKEY_free(pub_other);
		return false;
	}
	EVP_PKEY_free(pub_other);


	/*
	  Step 6. Adesso posso, con la sessione Diffie-Hellman, generare la chiave
	  simmetrica. Questa, attraverso un meccasimo di hash, genera le due chiavi
	  di sessione:
	    1. chiave simmetrica per la cifratura (this->chipherKey)
		2. chiave simmetrica per gli HMAC (this->hashKey)
	  Alla fine distruggo la mia chiave privata Diffie-Hellman e la shared_key.
	*/
	vector<byte> shared_key = KeyManager::getSimmetricKeyDH(Y_other, dh_session);
	setChiperKeyFromSecret(shared_key.data(),shared_key.size());
	setHashKeyFromSecret(shared_key.data(),shared_key.size());

	Utilities::eraseMemory(a.data(), a.size());
	Utilities::eraseMemory(shared_key.data(), shared_key.size());
	Utilities::eraseMemory(toSign.data(), toSign.size());
	Utilities::eraseMemory(toVerify.data(), toVerify.size());

	return keyConfermation();
}

/*=========================================================================================*/
bool Connection::exchangeKey_EC(const string& pathMyCert, X509** otherCert, uint32_t sizeReq) {
	/*
	   Step 1.
	     1. Creazione sessione Diffie-Hellman
		 2. Invio di Ya
		 3. Ricezione di Y_other
	*/
	if(sizeReq != 256 && sizeReq != 521) sizeReq = 256;
	EVP_PKEY* Ya_key = KeyManager::createDH_PUB_EC(sizeReq);
	vector<byte> Ya = KeyManager::serializeEVP_PUB(Ya_key);
	if(!tcpSocket.sendObject(Ya)) {
		cerr << "Error: can not send Ya" << endl;
		return false;
	}


	vector<byte> Y_other = tcpSocket.receiveObject();
	if(Y_other.size() == 0) {
		cerr << "Can not receive Y" << endl;
		return false;
	}


	/*
	   Step 2. La Ya (chiave pubblica) appena mandata deve essere firmata insieme alla Y_other,
	   perciò creo una firma e la mando.
	   La firma deve essere fatta con la chiave privata che è archivita in un file pem
	   sul disco, quindi prima leggo il file e poi firmo.
	   Distruggo la chiave privata il prima possibile.
	*/
	vector<byte> toSign = Ya;
	vector<byte> toVerify = Y_other;
	
	toSign.insert(toSign.end(), Y_other.begin(), Y_other.end());
	toVerify.insert(toVerify.end(), Ya.begin(), Ya.end());
	
	string pathPRIV = "./priv.pem";
	EVP_PKEY* my_priv = KeyManager::loadPRIV(pathPRIV);
	vector<byte> sign = Signer::staticSign(my_priv, toSign);
	EVP_PKEY_free(my_priv);
	if(!tcpSocket.sendObject(sign)) {
		cerr << "Error: can not send the sign of Y" << endl;
		return false;
	}


	/*
	  Step 3. Segue l'invio del proprio certificato. Tale certificato è presente sul
      disco alla path pathMyCert. Quindi prima leggo il certificato, poi lo serializzo

	  ed infine lo invio. Appena finito lo distruggo.
	*/
	X509* myCert = Cmanager::loadCERT(pathMyCert);
	vector<byte> serializedCert = Cmanager::serialize(myCert);
	X509_free(myCert);
	if(!tcpSocket.sendObject(serializedCert)) {
		cerr << "Error: can not send my certificate" << endl;
	}


	/*
	  Step 4. Ricevo:
	    1. la chiave pubblica Diffie-Hellman dell'altro peer,
		2. la firma che l'altro peer ha fatto per firmare tale chiave,

		3. il certificato dell'altro peer che mi sta mandando.
	*/

	vector<byte> sign_other = tcpSocket.receiveObject();
	if(sign_other.size() == 0) {
		cerr << "Can not receive the sign of Y" << endl;
		return false;
	}

	vector<byte> serializedOtherCert = tcpSocket.receiveObject();
	if(serializedOtherCert.size() == 0) {
		cerr << "Can not receive the certificate" << endl;
		return false;
	}


	/*
	  Step 5. Il certificato deserializzato e poi verificato. Se non
	  valido annullo tutto, se valido, continuo.
	  Inoltre dal certificato appena ricevuto (se valido), estraggo la chiave
	  pubblica dell'altro peer e la uso per verificare la firma che ho ricevuto
	  allo step precedente.
	*/
	*otherCert = Cmanager::deserialize(serializedOtherCert);
	bool validCert = this->cmanager.verify(*otherCert);

	if(!validCert) {
		cerr << "Certificato non valido\n";
		return false;
	}

	EVP_PKEY* pub_other = Cmanager::getPUBKEY(*otherCert);
	if(!Verifier::staticVerification(pub_other, toVerify, sign_other)) {
		cerr << "Error during sign verification\n";
		EVP_PKEY_free(pub_other);
		return false;
	}
	EVP_PKEY_free(pub_other);


	/*
	  Step 6. Adesso posso, con la sessione Diffie-Hellman, generare la chiave
	  simmetrica. Questa, attraverso un meccasimo di hash, genera le due chiavi
	  di sessione:
	    1. chiave simmetrica per la cifratura (this->chipherKey)
		2. chiave simmetrica per gli HMAC (this->hashKey)
	  Alla fine distruggo la mia chiave privata Diffie-Hellman e la shared_key.

	*/
	EVP_PKEY* Y_other_des = KeyManager::deserializeEVP_PUB(Y_other);
	vector<byte> shared_key = KeyManager::getSimmetricKeyDH_EC(Ya_key, Y_other_des);
	setChiperKeyFromSecret(shared_key.data(),shared_key.size());
	setHashKeyFromSecret(shared_key.data(),shared_key.size());
	EVP_PKEY_free(Y_other_des);
	EVP_PKEY_free(Ya_key);
	Utilities::eraseMemory(shared_key.data(), shared_key.size());
	Utilities::eraseMemory(Ya.data(), Ya.size());
	Utilities::eraseMemory(toSign.data(), toSign.size());
	Utilities::eraseMemory(toVerify.data(), toVerify.size());

	return keyConfermation();
}

/*=====================================================================*/

void Connection::setChiperKeyFromSecret(const byte* source, const uint32_t sourceSize) {
	//Scramble della chiave preservando la randomicità
	byte* tmp = new byte[sourceSize];
	for(uint32_t i = 0; i < sourceSize; ++i)
		tmp[i] = source[i] ^ (byte)i;

	//Genero un hash e lo comprimo preservando la randomicità, poi lo metto nella chiave
	vector<byte> tmpChipherKey = SimpleHash::hash(source,sourceSize);
	for(uint32_t i = 0; i < sizeof(this->chipherKey); ++i)
		this->chipherKey[i] = tmpChipherKey[i];

	//Pulizia
	Utilities::eraseMemory(tmp,sourceSize);
	Utilities::eraseMemory(tmpChipherKey.data(), tmpChipherKey.size());
	delete[] tmp;
}

void Connection::setHashKeyFromSecret(const byte* source, const uint32_t sourceSize) {
	SimpleHash::hash(source,sourceSize,this->hashKey);
}

bool Connection::keyConfermation() {
	/*
	  Preparo il random, lo invio e preparo gli HMAC che mi aspetto
	*/
	byte randToSend[32];
	Random::getRandomBytes(randToSend,sizeof(randToSend));
	if(tcpSocket.send(randToSend,sizeof(randToSend)) != sizeof(randToSend)) {
		cerr << "keyConfermation error (1)\n";
		return false;
	}
	vector<byte> HmacForChiperKey = HmacCalculator::staticHash(chipherKey, randToSend, sizeof(randToSend), sizeof(chipherKey));
	vector<byte> HmacForHashKey = HmacCalculator::staticHash(hashKey, randToSend, sizeof(randToSend), sizeof(hashKey));
	
	/*
	  Ricevo il random dell'altro peer e ne calcolo di Hmac, poi li invio
	*/
	byte receivedRand[32];
	if(tcpSocket.receive(receivedRand,sizeof(receivedRand)) != sizeof(receivedRand)) {
		cerr << "keyConfermation error (2)\n";
		return false;
	}
	vector<byte> HmacForChiperKeyToSend = HmacCalculator::staticHash(chipherKey, receivedRand, sizeof(randToSend), sizeof(chipherKey));
	vector<byte> HmacForHashKeyToSend = HmacCalculator::staticHash(hashKey, receivedRand, sizeof(randToSend), sizeof(hashKey));
	if(!tcpSocket.sendObject(HmacForChiperKeyToSend) || !tcpSocket.sendObject(HmacForHashKeyToSend)) {
		cerr << "keyConfermation error (3)\n";
		return false;
	}
	
	/*
	  A questo punto ricevo i due HMAC e li confronto
	*/
	vector<byte> HmacForChiperKeyReceived = tcpSocket.receiveObject();
	vector<byte> HmacForHashKeyReceived = tcpSocket.receiveObject();
	if(HmacForChiperKeyReceived.size() == 0 || HmacForHashKeyToSend.size() == 0) {
		cerr << "keyConfermation error (4)\n";
		return false;
	}
	if(HmacForChiperKeyReceived != HmacForChiperKey) {
		cerr << "keyConfermation error (5)\n";
		return false;
	}
	if(HmacForHashKeyReceived != HmacForHashKey) {
		cerr << "keyConfermation error (6)\n";
		return false;
	}
	
	return true;
}

/*

int Connection::askProtocol() {
	uint32_t kind;
	if( tcpSocket.receive(&(byte*)kind, sizeof(kind))  != sizeof(kind)) {
		handleError();
	}
	if(kind == DHEC_TYPE) {
		uint32_t response = REPLAY_TRUE;
	  if( tcp.send(&(byte*)response, sizeof(response) ) != sizeof(response) ) {
			handleError();
		}
		return DHEC_TYPE;
	}else if(kind == DH_TYPE) {
		uint32_t response = REPLAY_TRUE;
		if( tcp.send(&(byte*)response, sizeof(response) ) != sizeof(response) ) {
			handleError();
		}
		return DH_TYPE;
	}else{
		uint32_t response = REPLAY_FALSE;
		if( tcp.send(&(byte*)response, sizeof(response) ) != sizeof(response) ) {
			handleError();
		}
		return -1;
	}
	uint32_t response = REPLAY_FALSE;
	if( tcp.send(&(byte*)response, sizeof(response) ) != sizeof(response) ) {
		handleError();
	}
	return -1;
}



int Connection::sendMyProtocol(uint32_t kindOfProtocol) {
	if( kindOfProtocol != DH_TYPE && kindOfProtocol != DHEC_TYPE) {
		return -1;
	}
	if( tcp.send(&(byte*)kindOfProtocol, sizeof(kindOfProtocol) ) != sizeof(kindOfProtocol) ) {
		handleError();
	}

	uint32_t response;
	if( tcpSocket.receive(&(byte*)response, sizeof(response))  != sizeof(response)) {
		handleError();
	}
	if (response == REPLAY_TRUE) return kindOfProtocol;
	else if(response != REPLAY_TRUE) return -1;


}

*/
