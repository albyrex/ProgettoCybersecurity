#include "Cmanager.h"
#include <iostream>
#include <string.h>

using namespace std;

Cmanager::Cmanager(){
  this->store = X509_STORE_new();

  //per aggiungere tutti i certificati
  for (const auto & entry : std::experimental::filesystem::directory_iterator("./cert")){
	string filePath = "./cert/";
	filePath += entry.path().filename();
    FILE* fp = fopen(filePath.c_str(), "r");
    if(fp == NULL){
		cerr << "fp == NULL\n";
		continue;
	}
    X509* c = PEM_read_X509(fp, NULL, NULL, NULL);
    addCERT(c);
	X509_free(c);
    fclose(fp);
  }

  //tutte le CRL
  bool atLeastOneCrl = false;
  for (const auto & entry : std::experimental::filesystem::directory_iterator("./crl")){
	string filePath = "./crl/";
	filePath += entry.path().filename();
    FILE* fp = fopen(filePath.c_str(), "r");
    if(fp == NULL){
		continue;
	}
    X509_CRL* cr = PEM_read_X509_CRL(fp, NULL, NULL, NULL);
    addCRL(cr);
	X509_CRL_free(cr);
    fclose(fp);
	atLeastOneCrl = true;
  }

  
  if(atLeastOneCrl)
	X509_STORE_set_flags(this->store, X509_V_FLAG_CRL_CHECK);
}

Cmanager::~Cmanager(){
  if(this->store != NULL)
    X509_STORE_free(this->store);
}

bool Cmanager::verify(X509* cert) {
  if(cert == NULL) return false;

  X509_STORE_CTX* ctx = X509_STORE_CTX_new();
  if(ctx == NULL) {
    cout << "Error: ctx is NULL\n";
    return false;
  }

  if(X509_STORE_CTX_init(ctx, this->store, cert, NULL) != 1) {
    cout << "Error: no init\n";
    return false;
  }

  int ret = X509_verify_cert(ctx);
  X509_STORE_CTX_free(ctx);
  return (ret == 1) ? true : false;

}

bool Cmanager::addCERT(X509* cert) {
  if(cert == NULL){
	  cerr << "Cmanager::addCERT(...) -> cert == NULL\n";
	  return false;
  }
  if(X509_STORE_add_cert(this->store, cert) == 1){
    return true;
  }else{
	cerr << "Cmanager::addCERT(...) -> error\n";
    return false;
  }
}

bool Cmanager::addCRL(X509_CRL* crl) {
  if(crl == NULL) return false;
  if(X509_STORE_add_crl(this->store, crl) == 1){
    return true;
  }else{
    return false;
  }
}

X509* Cmanager::loadCERT(string filename) {
  FILE* fp = fopen(filename.c_str(), "r");
  if(fp == NULL){
	  cerr << "Cmanager::loadCERT(...) -> cannot open the file\n";
	  return NULL;
  }
  X509* ret = PEM_read_X509(fp, NULL, NULL, NULL);
  fclose(fp);
  return ret;
}

X509_CRL* Cmanager::loadCRL(string filename) {
  FILE* fp = fopen(filename.c_str(), "r");
  if(fp == NULL) return NULL;
  X509_CRL* ret = PEM_read_X509_CRL(fp, NULL, NULL, NULL);
  fclose(fp);
  return ret;
}

EVP_PKEY* Cmanager::getPUBKEY(X509* cert) {
  if(cert == NULL) return NULL;
  EVP_PKEY* pubKey = X509_get_pubkey(cert);
  return pubKey;
}

EVP_PKEY* Cmanager::getPUBKEY(string filename) {
  X509* cert = loadCERT(filename);
  if(cert == NULL) return NULL;
  EVP_PKEY* pubKey = X509_get_pubkey(cert);
  return pubKey;
}

vector<byte> Cmanager::serialize(X509* cert) {
  int len;
  byte *buf;
  buf = NULL;

  len = i2d_X509(cert, &buf);

  if(len < 0)
    cerr << "Cmanager::serialize(...) -> error\n";

  vector<byte> ret(len);
  memcpy(ret.data(), buf, len);
  OPENSSL_free(buf);
  return ret;
}

X509* Cmanager::deserialize(const vector<byte>& cert) {
  X509* _cert = X509_new();
  const byte* data = cert.data();
  d2i_X509(&_cert, (const byte**)&data, cert.size());
  return _cert;
}

string Cmanager::getSubjectName(X509* cert) {
	X509_NAME* x509Name = X509_get_subject_name(cert);
	char* str = X509_NAME_oneline(x509Name, NULL, 0);
	string ret(str);
	OPENSSL_free(str);
	return ret;
}

string Cmanager::getCA(X509* cert) {
	X509_NAME* x509Name = X509_get_issuer_name(cert);
	char* str = X509_NAME_oneline(x509Name, NULL, 0);
	string ret(str);
	OPENSSL_free(str);
	return ret;
}
