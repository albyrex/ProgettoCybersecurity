#include "Verifier.h"

Verifier::Verifier(EVP_PKEY* pubK){
  this->digestAlg = EVP_sha256();
  this->ctx = EVP_MD_CTX_create();
  this->pubK = pubK;

  if(EVP_VerifyInit(this->ctx, this->digestAlg) == 0){
    exit(1);
  }
}

Verifier::~Verifier(){
  if(this->ctx != NULL)
    EVP_MD_CTX_free(this->ctx);
}


void Verifier::updateVerification(const vector<byte>& source){
  if(EVP_VerifyUpdate(this->ctx, source.data(),
  source.size()) == 0){
    exit(1);
  }
}

void Verifier::updateVerification(const void* source, const size_t size){
  if(EVP_VerifyUpdate(this->ctx, (byte*)source, size) == 0){
    exit(1);
  }
}

bool Verifier::finalizeVerification(const vector<byte>& sign){
  if(EVP_VerifyFinal(this->ctx, sign.data(), sign.size(), this->pubK) == 1){
    return true;
  }
  return false;
}

bool Verifier::finalizeVerification(const void* source, const size_t size){
  if(EVP_VerifyFinal(this->ctx, (byte*)source, size, this->pubK) == 1){
    return true;
  }
  return false;
}

bool Verifier::staticVerification(EVP_PKEY* pubK, const vector<byte>& source, const vector<byte>& sign) {
  EVP_MD_CTX* ctx = EVP_MD_CTX_create();
  const EVP_MD* digestAlg = EVP_sha256();

  if(EVP_VerifyInit(ctx, digestAlg) == 0){
    exit(1);
  }

  if(EVP_VerifyUpdate(ctx, source.data(),source.size()) == 0){
    exit(1);
  }


  if(EVP_VerifyFinal(ctx, sign.data(), sign.size(), pubK) == 1){
    EVP_MD_CTX_free(ctx);
    return true;
  }else{
    EVP_MD_CTX_free(ctx);
    return false;
  }
}

bool Verifier::staticVerification(EVP_PKEY* pubK, const void* source, const size_t source_size, const void* sign, const uint32_t signSize) {
  EVP_MD_CTX* ctx = EVP_MD_CTX_create();
  const EVP_MD* digestAlg = EVP_sha256();

  if(EVP_VerifyInit(ctx, digestAlg) == 0){
    exit(1);
  }

  if(EVP_VerifyUpdate(ctx, (byte*)source, source_size) == 0){
    exit(1);
  }
  
  if(EVP_VerifyFinal(ctx, (byte*)sign, signSize, pubK) == 1){
    EVP_MD_CTX_free(ctx);
    return true;
  }else{
    EVP_MD_CTX_free(ctx);
    return false;
  }
}
