#include <openssl/dh.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;


typedef unsigned char byte;

namespace KeyManager {
  //Diffie - Helman
  vector<byte> getSimmetricKeyDH(vector<byte>& pubK, DH* dh_session);
  DH* generateDHPair_2048(vector<byte>& pub, vector<byte>& priv);
  DH* generateDHPair_3072(vector<byte>& pub, vector<byte>& priv);

  //RSA
  void generateRSA(uint32_t sizeKey, string path);  //genera e salva nella cartella corrente di esecuzione o in quella passata
  void generateRSA(uint32_t sizeKey, RSA** rsa);

  bool savePUB(string path, RSA* rsa);
  bool savePRIV(string path, RSA* rsa);

  void RSAtoEVP(RSA* rsa, EVP_PKEY* evp);
  void EC_PKEYtoEVP(EC_KEY* ec, EVP_PKEY* evp);

  //PEM
  EVP_PKEY* loadPUB(string path);	//for EC, RSA
  EVP_PKEY* loadPRIV(string path);

  //EC
  EVP_PKEY* createDH_PUB_EC(uint32_t reqSize);
  vector<byte> getSimmetricKeyDH_EC(EVP_PKEY* pkey, EVP_PKEY* PUB_peerkey);

  //serialization EVP_PKEY
  vector<byte> serializeEVP_PUB(EVP_PKEY* key);
  EVP_PKEY* deserializeEVP_PUB(vector<byte>& serialized);

}
