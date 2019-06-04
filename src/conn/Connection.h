//#ifndef __CONNECTION_H__
//#define __CONNECTION_H__


#include <vector>
#include "TcpSocket.h"
#include "DiskManager.h"
#include "Encryptor.h"
#include "Decryptor.h"
#include "HmacCalculator.h"
#include "Signer.h"
#include "Verifier.h"
#include "KeyManager.h"
#include "Cmanager.h"
#include "Utilities.h"

#define RECEIVE_FILEBLOCK_OK     0
#define RECEIVE_FILEBLOCK_ERROR -1
#define RECEIVE_FILEBLOCK_LAST   1

#define OK               1
#define CRITICAL_ERROR   0
#define NEGLIGIBLE_ERROR 2


#define KEYS_LIFESPAN (1 << 30)

using namespace std;


class Connection {

protected:
	TcpSocket tcpSocket;
	byte chipherKey[AES_128_KEY_SIZE];
	byte hashKey[32];
	uint32_t orderNumber;
	Decryptor* decryptor;
	Encryptor* encryptor;
	uint32_t consecutiveErrors;
	Cmanager cmanager;

protected:
	Connection(); //Maschero il costruttore
	~Connection();
	bool receiveMessage(Message* mess);
	bool sendMessage(Message* mess);
	inline uint32_t getOrder();
	inline void stepBackOrder();
	bool keysAreOld();
	vector<byte> getIv();
	int receiveFileBlock(vector<byte>& dest);
	bool sendFileBlock(vector<byte>& dest, bool lastPart);
	void setChiperKeyFromSecret(const byte* source, const uint32_t sourceSize);
	void setHashKeyFromSecret(const byte* source, const uint32_t sourceSize);
	bool exchangeKey(const string& pathMyCert, X509** otherCert);
	bool exchangeKey_EC(const string& pathMyCert, X509** otherCert, uint32_t sizeReq);
	bool keyConfermation();

	//int askProtocol();
	//int sendMyProtocol(uint32_t kindOfProtocol);

};


//#endif
