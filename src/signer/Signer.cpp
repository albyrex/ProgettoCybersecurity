#include "Signer.h"

Signer::Signer(EVP_PKEY* privK){
  this->digestAlg = EVP_sha256();
  this->ctx = EVP_MD_CTX_create();
  this->privK = privK;

  if(EVP_SignInit(this->ctx, this->digestAlg) == 0){
    exit(1);
  }
}

Signer::~Signer(){
  if(this->ctx != NULL)
    EVP_MD_CTX_free(this->ctx);
}

void Signer::updateSign(const vector<byte>& source){
  if(EVP_SignUpdate(this->ctx, source.data(), source.size()) == 0){
    exit(1);
  }
}

void Signer::updateSign(const void* source, const size_t source_size){
  if(EVP_SignUpdate(this->ctx, (byte*)source, source_size) == 0){
    exit(1);
  }
}

vector<byte> Signer::finalizeSign(){
  uint32_t size = EVP_PKEY_size(this->privK);
  vector<byte> dest(size);
  if(EVP_SignFinal(this->ctx, dest.data(), &size, this->privK)  == 0){
    exit(1);
  }
  dest.resize(size);
  return dest;
}

uint32_t Signer::finalizeSign(void* sign){
  uint32_t size;
  if(EVP_SignFinal(this->ctx, (byte*)sign, &size, this->privK) == 0){
    exit(1);
  }
  return size;
}

vector<byte> Signer::staticSign(EVP_PKEY* privK, const vector<byte>& source){
  EVP_MD_CTX* ctx = EVP_MD_CTX_create();
  const EVP_MD* digestAlg = EVP_sha256();

  if(EVP_SignInit(ctx, digestAlg) == 0){
    exit(1);
  }

  if(EVP_SignUpdate(ctx, source.data(), source.size()) == 0){
    exit(1);
  }

  uint32_t size = EVP_PKEY_size(privK);
  vector<byte> dest(size);

  if(EVP_SignFinal(ctx, dest.data(), &size, privK)  == 0){
    exit(1);
  }
  
  dest.resize(size);
  
  EVP_MD_CTX_free(ctx);
  return dest;
}

uint32_t Signer::staticSign(void* dest, EVP_PKEY* privK, const void* source, const size_t source_size){
  EVP_MD_CTX* ctx = EVP_MD_CTX_create();
  const EVP_MD* digestAlg = EVP_sha256();

  if(EVP_SignInit(ctx, digestAlg) == 0){
	EVP_MD_CTX_free(ctx);
    exit(1);
  }

  if(EVP_SignUpdate(ctx, (byte*)source, source_size) == 0){
	EVP_MD_CTX_free(ctx);
    exit(1);
  }

  uint32_t size;

  if(EVP_SignFinal(ctx, (byte*)dest, &size, privK) == 0){
    EVP_MD_CTX_free(ctx);
    exit(1);
  }
  EVP_MD_CTX_free(ctx);
  
  return size;
}
