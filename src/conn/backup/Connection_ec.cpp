#include "Connection.h"
#include "Utilities.h"
#include "Random.h"
#include <iostream>
#include <string.h>
#include <openssl/rand.h>

#define IV_SIZE 16

using namespace std;

Connection::Connection() {
	orderNumber = 0;
	decryptor = NULL;
	encryptor = NULL;
	hashCalculator = NULL;
}

Connection::~Connection() {
	Utilities::eraseMemory(chipherKey, sizeof(chipherKey));
	Utilities::eraseMemory(hashKey, sizeof(hashKey));
	
	if(decryptor)
		delete decryptor;
	if(encryptor)
		delete encryptor;
	if(hashCalculator)
		delete hashCalculator;
}

bool Connection::receiveMessage(Message* mess) {
	uint32_t expectedOrder = getOrder();
	vector<byte> currentIv = tcpSocket.receive(IV_SIZE);

	if(currentIv.size() != (int)IV_SIZE){
		cerr << "Errore di connessione durante la ricezione dell' iv del messaggio: impossibile ricevere il messaggio correttamente.\n\n";
		return false;
	}

	vector<byte> crypted = tcpSocket.receive(C_MESSAGE_SIZE);

	if(crypted.size() != C_MESSAGE_SIZE){
		cerr << "Errore di connessione durante la ricezione del messaggio: impossibile ricevere il messaggio correttamente.\n\n";
		return false;
	}

	vector<byte> hashReceived = this->tcpSocket.receive(SHA256_HASH_SIZE);
	if(hashReceived.size() != SHA256_HASH_SIZE){
		cerr << "Error: hashReceived has differend size" << endl;
		return false;
	}

	HashCalculator hc(this->hashKey);
	hc.updateHash(currentIv);
	hc.updateHash(crypted);
	vector<byte> hashCalculated = hc.getHash();

	if(!(hashReceived == hashCalculated)){
		cerr << "Error: hash received != hash calculated" << endl;
		for(uint32_t i = 0; i < hashReceived.size(); ++i){
			cerr<<std::hex<<(int)hashReceived[i]<<" ";
		}

		cerr<<"\n HASH calculated"<<endl;
		for(uint32_t i = 0; i < hashCalculated.size(); ++i){
			cerr<<std::hex<<(int)hashCalculated[i]<<" ";
		}
		cerr << "\n";

		return false;
	}

	//Ricorda: i parametri sono: key, source, sourceSize, dest
	uint32_t decrLen = Decryptor::staticDecrypt(this->chipherKey, currentIv.data(), crypted.data(), crypted.size(), mess);

	if(decrLen != sizeof(Message)){
		cerr << "Error: plaintext has a different size" << endl;
	}


	if(mess->order != expectedOrder) {
		cerr << "mess->order: "<<mess->order<<endl;
		cerr << "expectedOrder: "<<expectedOrder<<endl;
		cerr << "Errore numero d'ordine sbagliato! Termino\n";
		stepBackOrder();
		return false;
	}
	
	return true;
}

void Connection::sendMessage(Message* mess) {
	mess->order = getOrder();
	vector<byte> currentIv = getIv();

	if(tcpSocket.send(currentIv) != (int)currentIv.size()){
		cerr << "Errore nella sendMessage (0)\n";
	}

	vector<byte> crypted(C_MESSAGE_SIZE);
	uint32_t crLen = Encryptor::staticEncrypt(this->chipherKey, currentIv.data(), mess, sizeof(Message), crypted.data());

	if(crLen != C_MESSAGE_SIZE) {
		cerr << "Errore nella sendMessage (1)\n";
	}

	if(tcpSocket.send(crypted) != C_MESSAGE_SIZE){
		cerr << "Errore nella sendMessage (2)\n";
	}

	HashCalculator hc(this->hashKey);
	hc.updateHash(currentIv);
	hc.updateHash(crypted);
	vector<byte> hash = hc.getHash();
	if((int)hash.size() != this->tcpSocket.send(hash)){
		cerr << "Errore nella sendMessage (3)\n";
	}
}

uint32_t Connection::getOrder() {
	return orderNumber++;
}

bool Connection::sendFileBlock(vector<byte>& fileBlock, bool lastPart) {
	vector<byte> currentIv = getIv();

	if(tcpSocket.send(currentIv) != (int)currentIv.size()){
		cerr << "Errore nella sendFileBlock (0)\n";
	}

	FileBlockDescriptor fbd;
	fbd.order = getOrder();
	fbd.flag = (lastPart == true) ? FILEBLOCK_LAST : FILEBLOCK_ELEMENT;

	if(this->encryptor == NULL) {
		this->encryptor = new Encryptor(this->chipherKey, currentIv.data());
		this->hashCalculator = new HashCalculator(this->hashKey);
	}

	vector<byte> crypted = encryptor->encrypt(fileBlock);
	this->hashCalculator->updateHash(fileBlock);
	
	if(lastPart){
		vector<byte> finalCrypt = encryptor->finalize();
		crypted.insert(crypted.end(), finalCrypt.begin(), finalCrypt.end());
		delete this->encryptor;
		this->encryptor = NULL;
	}

	fbd.blockDim = crypted.size();
	vector<byte> fbd_crypted(SIZE_AFTER_AES(sizeof(fbd)));
	Encryptor::staticEncrypt(this->chipherKey, currentIv.data(), &fbd, sizeof(fbd), fbd_crypted.data());


	if(tcpSocket.send(fbd_crypted) != (int)fbd_crypted.size() || tcpSocket.send(crypted) != (int)crypted.size()){
		cerr << "sendFileBlock() -> error (1)\n";
		return false;
	}

	HashCalculator hc(this->hashKey);
	hc.updateHash(currentIv);
	hc.updateHash(fbd_crypted);
	hc.updateHash(crypted);
	vector<byte> hmac_calculated = hc.getHash();

	if(tcpSocket.send(hmac_calculated) != (int)hmac_calculated.size()){
		cerr << "sendFileBlock() -> error (2)\n";
		return false;
	}
	
	if(lastPart) {
		vector<byte> totalHash = this->hashCalculator->getHash();
		delete this->hashCalculator;
		this->hashCalculator = NULL;
		if(tcpSocket.send(totalHash) != (int)totalHash.size()){
			cerr << "sendFileBlock() -> error (3)\n";
			return false;
		}
	}

	return true;
}

int Connection::receiveFileBlock(vector<byte>& dest) {
	uint32_t expectedOrder = getOrder();
	vector<byte> currentIv = tcpSocket.receive(IV_SIZE);

	if(currentIv.size() != (int)IV_SIZE){
		cerr << "Errore di connessione durante la ricezione dell' iv del fileBlock: impossibile ricevere il fileBlock correttamente.\n\n";
		return false;
	}

	if(this->decryptor == NULL) {
		this->decryptor = new Decryptor(this->chipherKey, currentIv.data());
		this->hashCalculator = new HashCalculator(this->hashKey);
	}

	FileBlockDescriptor fbd;
	vector<byte> fbd_crypted = tcpSocket.receive(SIZE_AFTER_AES(sizeof(fbd)));
	if(fbd_crypted.size() != SIZE_AFTER_AES(sizeof(fbd))) {
		cerr << "receiveFileBlock -> (1)" << endl;
		return RECEIVE_FILEBLOCK_ERROR;
	}

	Decryptor::staticDecrypt(this->chipherKey, currentIv.data(), fbd_crypted.data(),fbd_crypted.size(), &fbd);
	if(fbd.order != expectedOrder){
		cerr << "receiveFileBlock -> (2)" << endl;
		stepBackOrder();
		return RECEIVE_FILEBLOCK_ERROR;
	}

	if(fbd.blockDim > DEFAULT_BLOCK_SIZE + 32) {
		cerr << "receiveFileBlock -> (5)" << endl;
		return RECEIVE_FILEBLOCK_ERROR;
	}

	vector<byte> crypted = tcpSocket.receive(fbd.blockDim);
	if(crypted.size() != fbd.blockDim) {
		cerr << "receiveFileBlock -> (3)" << endl;
		return RECEIVE_FILEBLOCK_ERROR;
	}

	vector<byte> hmac_received = tcpSocket.receive(32);
	if(hmac_received.size() != 32) {
		cerr << "receiveFileBlock -> (4)" << endl;
		return RECEIVE_FILEBLOCK_ERROR;
	}

	HashCalculator hc(this->hashKey);
	hc.updateHash(currentIv);
	hc.updateHash(fbd_crypted);
	hc.updateHash(crypted);
	vector<byte> hmac_calculated = hc.getHash();

	if(hmac_received != hmac_calculated){
		cerr << "Error: tampering" << endl;
		return RECEIVE_FILEBLOCK_ERROR;
	}

	dest = decryptor->decrypt(crypted);
	if(fbd.flag == FILEBLOCK_LAST) {
		vector<byte> final = decryptor->finalize();
		dest.insert(dest.end(), final.begin(), final.end());
		delete this->decryptor;
		this->decryptor = NULL;
	}
	
	this->hashCalculator->updateHash(dest);
	
	if(fbd.flag == FILEBLOCK_LAST) {
		vector<byte> totalHash = this->hashCalculator->getHash();
		delete this->hashCalculator;
		this->hashCalculator = NULL;
		//Adesso devo ricevere il totalHash
		vector<byte> receivedToatalHash = tcpSocket.receive(32);
		if(receivedToatalHash.size() != 32){
			cerr << "Error aaaa\n";
			return RECEIVE_FILEBLOCK_ERROR;
		}
		if(totalHash != receivedToatalHash) {
			cerr << "Warming: the final hashes of the files are not eqaul\n" <<
			        "Anyway, this could be safe: the only thing an attacker could do is to change the final hmac tag only\n" <<
					"The transferred file should be correctly sent\n";
			//return RECEIVE_FILEBLOCK_ERROR;
		}
		return RECEIVE_FILEBLOCK_LAST;
	}
	return RECEIVE_FILEBLOCK_OK;
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
	   Step 1. Creazione sessione Diffie-Hellman e invio di pub_dh (cioè la chiave pubblica, la Ya).
	*/
	vector<byte> pub_dh;
	//vector<byte> priv_dh;
	//DH* dh_session = KeyManager::generateDHPair_3072(pub_dh, priv_dh);
	EVP_PKEY* Ya = KeyManager::createDH_PUB_EC(256);
	pub_dh = KeyManager::serializeEVP_PUB(Ya);
	cout << "\nInvio:\n";
	KeyManager::printVector(pub_dh);
	cout << "\n";
	if(!tcpSocket.sendObject(pub_dh)) {
		cerr << "Error: can not send Y" << endl;
		return false;
	}
	
	
	/*
	   Step 2. La pub_dh (chiave pubblica) appena mandata deve essere firmata, perciò
	   creo una firma e la mando.
	   La firma deve essere fatta con la chiave privata che è archivita in un file pem
	   sul disco, quindi prima leggo il file e poi firmo.
	   Distruggo la chiave privata il prima possibile.
	*/
	string pathPRIV = "./priv.pem";
	EVP_PKEY* my_priv = KeyManager::loadPRIV(pathPRIV);
	vector<byte> sign = Signer::staticSign(my_priv, pub_dh);
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
	vector<byte> Y_other = tcpSocket.receiveObject();
	if(Y_other.size() == 0) {
		cerr << "Can not receive Y" << endl;
		return false;
	}
	
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
	Cmanager cmanager;
	
	*otherCert = Cmanager::deserialize(serializedOtherCert);
	bool validCert = cmanager.verify(*otherCert);
	
	if(!validCert) {
		cerr << "Certificato non valido\n";
		return false;
	}
	
	EVP_PKEY* pub_other = Cmanager::getPUBKEY(*otherCert);
	if(!Verifier::staticVerification(pub_other, Y_other, sign_other)) {
		cerr << "Errore controllo firma\n";
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
	  Alla fine distruggo la mia chiave privata Diffie-Hellman e la k_simmetrica.
	*/
	//vector<byte> k_simmetrica = KeyManager::getSimmetricKeyDH(Y_other, dh_session);
	cout << "\nRicevuto:\n";
	KeyManager::printVector(Y_other);
	cout << "\n";
	EVP_PKEY* Y_other_des = KeyManager::deserializeEVP_PUB(Y_other);
	vector<byte> k_simmetrica = KeyManager::getSimmetricKeyDH_EC(Ya, Y_other_des);
	EVP_PKEY_free(Ya);
	EVP_PKEY_free(Y_other_des);
	setChiperKeyFromSecret(k_simmetrica.data(),k_simmetrica.size());
	setHashKeyFromSecret(k_simmetrica.data(),k_simmetrica.size());
	//Utilities::eraseMemory(priv_dh.data(), priv_dh.size());
	Utilities::eraseMemory(k_simmetrica.data(), k_simmetrica.size());
	
	cout << "\nSimmetrica:\n";
	KeyManager::printVector(k_simmetrica);
	cout << "\n";
	
	return keyConfermation();
}

void Connection::setChiperKeyFromSecret(const byte* source, const uint32_t sourceSize) {
	//Assumo che source sia un vettore di 384 byte (perché ho usato Diffie-Hellman a 3072bit)
	
	//Scramble della chiave preservando la randomicità
	byte tmp[sourceSize];
	for(uint32_t i = 0; i < sourceSize; ++i)
		tmp[i] = source[i] ^ (byte)i;
	
	//Genero un hash e lo comprimo preservando la randomicità, poi lo metto nella chiave
	vector<byte> tmpChipherKey = HashCalculator::simpleHash(source,sourceSize);
	for(uint32_t i = 0; i < sizeof(this->chipherKey); ++i)
		this->chipherKey[i] = tmpChipherKey[i];
	
	//Pulizia
	Utilities::eraseMemory(tmp,sourceSize);
	Utilities::eraseMemory(tmpChipherKey.data(), tmpChipherKey.size());
}

void Connection::setHashKeyFromSecret(const byte* source, const uint32_t sourceSize) {
	//Assumo che source sia un vettore di 384 byte (perché ho usato Diffie-Hellman a 3072bit)
	HashCalculator::simpleHash(source,sourceSize,this->hashKey);
}

bool Connection::keyConfermation() {
	//Creo un numero random e lo invio
	uint32_t randToSend = Random::getRandomUint32();
	if(tcpSocket.send((byte*)&randToSend,sizeof(randToSend)) != sizeof(randToSend)) {
		cerr << "gg\n";
		return false;
	}
	
	//Riceviamo il random e rimandiamo indietro l'hmac cifrato
	uint32_t receivedRand;
	if(tcpSocket.receive((byte*)&receivedRand,sizeof(receivedRand)) != sizeof(receivedRand)) {
		cerr << "hh\n";
		return false;
	}
	byte ivToReply[16];
	memcpy(ivToReply, &receivedRand, sizeof(ivToReply));
	memset(ivToReply+sizeof(receivedRand), 0x00, sizeof(ivToReply)-sizeof(receivedRand));
	vector<byte> plainHmac_ = HashCalculator::staticHash(this->hashKey,&receivedRand,sizeof(receivedRand));
	vector<byte> encryptedHmac_ = Encryptor::staticEncrypt(this->chipherKey, ivToReply, plainHmac_.data(), plainHmac_.size());
	if(!tcpSocket.sendObject(encryptedHmac_)) {
		cerr << "zz\n";
		return false;
	}
	
	//Adesso ricevo l'hmac cifrato del random che ho inviato all'inizio e controllo se va bene
	vector<byte> receivedEcryptedHmac = tcpSocket.receiveObject();
	if(receivedEcryptedHmac.size() == 0) {
		cerr << "zzz\n";
		return false;
	}
	byte iv[16];
	memcpy(iv,&randToSend,sizeof(iv));
	memset(iv+sizeof(randToSend),0x00,sizeof(iv)-sizeof(randToSend));
	vector<byte> expectedPlainHmac = HashCalculator::staticHash(this->hashKey,&randToSend,sizeof(randToSend));
	vector<byte> expectedEncryptedHmac = Encryptor::staticEncrypt(this->chipherKey, iv, expectedPlainHmac.data(), expectedPlainHmac.size());
	
	if(expectedEncryptedHmac != receivedEcryptedHmac) {
		cerr << "Error: key confermation failed\n";
		return false;
	}
	
	return true;
}



















