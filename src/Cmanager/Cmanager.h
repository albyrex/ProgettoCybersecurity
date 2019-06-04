#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <string>
#include <vector>
#include <fstream>
#include <experimental/filesystem>

using namespace std;
typedef unsigned char byte;


/*
  X509: certificato
  X509_CRL: lista di revoca per certificati
  X509_STORE: è uno store
*/


class Cmanager {
  
  X509_STORE* store;

public:
  Cmanager();
  ~Cmanager();
  bool addCERT(X509* cert);
  bool addCRL(X509_CRL* crl);
  bool verify(X509* cert);

public:
  static X509_CRL* loadCRL(string filename);
  static X509* loadCERT(string filename);

  //le EVP_PKEY vanno deallocate
  static EVP_PKEY* getPUBKEY(X509* cert);
  static EVP_PKEY* getPUBKEY(byte* cert);
  static EVP_PKEY* getPUBKEY(string filename);

  static string getSubjectName(X509* cert);
  static string getCA(X509* cert);

  //serialization
  static vector<byte> serialize(X509* cert);
  static X509* deserialize(const vector<byte>& cert);

};
