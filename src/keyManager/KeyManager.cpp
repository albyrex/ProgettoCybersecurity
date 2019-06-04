#include "KeyManager.h"
#include <openssl/ec.h>
#include <openssl/x509.h>
#include <string.h>

/* ============================= DH ==================================================== */

//Code generated from 'openssl dhparam -C 2048' command line
DH* get_dh2048()
{
    static unsigned char dhp_2048[] = {
	0xDF, 0x25, 0x53, 0xC2, 0x81, 0x9E, 0x57, 0x8A, 0xCC, 0xAF,
	0xC0, 0xF2, 0x5A, 0x83, 0xD9, 0xFD, 0xDB, 0x1A, 0x43, 0x0D,
	0x3E, 0xB2, 0xBC, 0x91, 0x58, 0x1D, 0x08, 0x65, 0x2B, 0x77,
	0x5C, 0xB4, 0x21, 0xDC, 0xA5, 0x51, 0x07, 0xBF, 0x54, 0xEB,
	0x33, 0x55, 0x7B, 0x65, 0x9D, 0x64, 0x07, 0x3F, 0xC9, 0x2C,
	0x21, 0xFB, 0x16, 0xF9, 0xFA, 0xB2, 0xF7, 0xCE, 0x88, 0x55,
	0xCC, 0xC5, 0xA7, 0x44, 0x1B, 0xDE, 0xDB, 0x89, 0xD3, 0x07,
	0xD2, 0x1E, 0x97, 0xCA, 0x05, 0xF3, 0x5B, 0xE1, 0x09, 0xB3,
	0xAD, 0xDC, 0xBC, 0xF2, 0x05, 0xBF, 0x80, 0xBF, 0x7A, 0xC6,
	0xC2, 0x30, 0xFF, 0x4C, 0xDE, 0x75, 0x7A, 0x9A, 0x45, 0xE7,
	0x5D, 0x59, 0x69, 0x2E, 0xC1, 0x6D, 0xAF, 0x62, 0x57, 0xB0,
	0xC0, 0x4D, 0x7B, 0xA4, 0x94, 0x26, 0x9F, 0xB2, 0x06, 0x2D,
	0xED, 0xF0, 0xD3, 0x5E, 0x06, 0xB0, 0x4E, 0xE2, 0xF6, 0x6C,
	0xF2, 0x49, 0xA8, 0xDD, 0x8E, 0xFA, 0x35, 0x6E, 0xCD, 0x00,
	0x3B, 0x44, 0x98, 0xD6, 0xE5, 0x69, 0x1F, 0xC2, 0x86, 0x3F,
	0x85, 0x32, 0x43, 0x5A, 0x56, 0x7D, 0x7A, 0xB2, 0x62, 0xCE,
	0x09, 0xFC, 0x4B, 0x59, 0xD7, 0x3A, 0x11, 0x48, 0x89, 0x5A,
	0x64, 0x63, 0xC0, 0xE8, 0x79, 0x29, 0xF6, 0x78, 0x43, 0x30,
	0xA5, 0x93, 0x6C, 0xD2, 0x74, 0x11, 0x17, 0x60, 0xFE, 0x2B,
	0xFC, 0x0E, 0xB1, 0x7E, 0xB5, 0x24, 0x7C, 0xEF, 0x66, 0x10,
	0xB5, 0x19, 0xB9, 0x78, 0x82, 0xEF, 0xB4, 0xBD, 0x9F, 0x75,
	0x81, 0xDA, 0xCB, 0x7F, 0x01, 0xEF, 0x91, 0x31, 0x87, 0x25,
	0xB4, 0xA2, 0x0C, 0x15, 0xEE, 0x9D, 0xBE, 0x96, 0x8F, 0x37,
	0x1B, 0xF9, 0xC6, 0x79, 0x2F, 0x1D, 0x83, 0x7C, 0x2A, 0x17,
	0xC7, 0x84, 0x91, 0x3B, 0xEE, 0x8E, 0x85, 0xA3, 0xE2, 0xC0,
	0x4E, 0x85, 0x53, 0xD3, 0xAE, 0x8B
    };
    static unsigned char dhg_2048[] = {
	0x02
    };
    DH *dh = DH_new();
    BIGNUM *dhp_bn, *dhg_bn;

    if (dh == NULL)
        return NULL;
    dhp_bn = BN_bin2bn(dhp_2048, sizeof (dhp_2048), NULL);
    dhg_bn = BN_bin2bn(dhg_2048, sizeof (dhg_2048), NULL);
    if (dhp_bn == NULL || dhg_bn == NULL || !DH_set0_pqg(dh, dhp_bn, NULL, dhg_bn)) {
        DH_free(dh);
        BN_free(dhp_bn);
        BN_free(dhg_bn);
        return NULL;
    }
    return dh;
};

DH *get_dh3072()
{
    static unsigned char dhp_3072[] = {
        0xB8, 0x62, 0xFB, 0x84, 0x85, 0x81, 0x17, 0x03, 0x9F, 0x69,
        0x1F, 0xF1, 0x45, 0x3D, 0x61, 0x0D, 0x62, 0x75, 0xF4, 0xA6,
        0x14, 0x5F, 0x3E, 0xA2, 0xD4, 0xAA, 0xF2, 0xD9, 0x27, 0x84,
        0x34, 0x31, 0x73, 0xAE, 0x2A, 0x19, 0x7F, 0xAD, 0x45, 0xEB,
        0xA0, 0xF6, 0x12, 0x97, 0xFE, 0xB2, 0xA7, 0xC6, 0xC5, 0x4E,
        0x1A, 0x43, 0x5A, 0x87, 0xE4, 0xF5, 0x4C, 0x4C, 0x52, 0xA4,
        0x05, 0x5E, 0x5B, 0x90, 0x38, 0x52, 0x87, 0xC7, 0xBB, 0xB3,
        0x18, 0x60, 0x42, 0x72, 0x09, 0xA1, 0x47, 0xE1, 0xF0, 0x96,
        0xD2, 0x92, 0x0B, 0xF0, 0xF4, 0x0A, 0x21, 0x8A, 0xE3, 0x30,
        0x68, 0x61, 0x27, 0x1C, 0x6D, 0xA8, 0x75, 0xD9, 0x9B, 0x90,
        0x94, 0xB7, 0x08, 0x10, 0x5B, 0x35, 0x70, 0x23, 0x2A, 0x50,
        0x56, 0xD8, 0x01, 0x4C, 0xBD, 0x1F, 0xA9, 0xEF, 0x27, 0xFD,
        0xAE, 0x62, 0x74, 0x25, 0x8E, 0x6A, 0x82, 0x54, 0x82, 0xDA,
        0x42, 0x58, 0x54, 0x56, 0x77, 0x9D, 0x02, 0x3D, 0x03, 0x9E,
        0x44, 0x21, 0xB6, 0x18, 0x33, 0xF1, 0xC2, 0x94, 0xD4, 0xEE,
        0xAD, 0x81, 0x0C, 0x89, 0xA5, 0x40, 0x45, 0x01, 0x77, 0x3F,
        0x11, 0xE0, 0xB8, 0x63, 0x6C, 0x6A, 0x0F, 0x94, 0x07, 0x46,
        0x84, 0x0E, 0x84, 0x4D, 0x02, 0xA3, 0xC5, 0x38, 0x0A, 0x28,
        0x7E, 0x14, 0xF5, 0x8A, 0x6F, 0xCF, 0xCD, 0x53, 0xA0, 0xFC,
        0x8E, 0x3C, 0x66, 0xFF, 0x9B, 0x65, 0x2B, 0x07, 0x14, 0xD3,
        0x57, 0xC4, 0xD5, 0xE0, 0x47, 0xE3, 0xC6, 0xF4, 0x9D, 0x90,
        0xCD, 0x1C, 0x75, 0x4A, 0x16, 0xDC, 0x6B, 0x62, 0x13, 0xA1,
        0xF8, 0xFE, 0x36, 0xD7, 0x8B, 0xE0, 0x4D, 0xA0, 0xFC, 0xB8,
        0x37, 0x89, 0xF6, 0x74, 0x59, 0x13, 0x5F, 0x1A, 0x53, 0x31,
        0xFC, 0x4B, 0x2F, 0xA1, 0xC8, 0x47, 0xF1, 0x2D, 0xB3, 0x87,
        0xDE, 0x46, 0x92, 0x5A, 0x2A, 0xE0, 0xEC, 0xD1, 0xE4, 0x51,
        0xA9, 0xC2, 0xA0, 0x01, 0x18, 0x35, 0x24, 0xAB, 0x27, 0x92,
        0x8D, 0x27, 0x5B, 0x85, 0xA3, 0x15, 0x30, 0x82, 0x5D, 0x17,
        0xC6, 0xF8, 0xF0, 0x69, 0x21, 0xCF, 0xE2, 0x56, 0xED, 0xF5,
        0xA6, 0x58, 0xB7, 0xC0, 0x8F, 0x20, 0xBB, 0xC7, 0xC5, 0x4B,
        0x61, 0xF8, 0xEF, 0x27, 0x6E, 0xC5, 0xEC, 0x48, 0xCE, 0xE1,
        0x1F, 0x55, 0x75, 0x4A, 0x29, 0x5F, 0xDF, 0x69, 0xEB, 0x52,
        0x1A, 0x1B, 0xCE, 0x39, 0x17, 0x06, 0x09, 0xB6, 0x50, 0x21,
        0xF7, 0x82, 0x0F, 0x61, 0xB6, 0xD7, 0xF5, 0xB7, 0x82, 0x2F,
        0xDE, 0x78, 0x04, 0x81, 0xE6, 0x78, 0xA7, 0xC3, 0x9D, 0x5B,
        0xF4, 0x7A, 0xDF, 0xE6, 0x05, 0xC8, 0xB8, 0x2C, 0x88, 0x12,
        0x22, 0x80, 0xE6, 0xEE, 0xF6, 0x6E, 0xC3, 0x53, 0x19, 0x69,
        0x66, 0x75, 0x64, 0x91, 0x59, 0xCD, 0x5B, 0x27, 0x1C, 0xB6,
        0x30, 0x19, 0x22, 0x53
    };
    static unsigned char dhg_3072[] = {
        0x02
    };
    DH *dh = DH_new();
    BIGNUM *dhp_bn, *dhg_bn;

    if (dh == NULL)
        return NULL;
    dhp_bn = BN_bin2bn(dhp_3072, sizeof (dhp_3072), NULL);
    dhg_bn = BN_bin2bn(dhg_3072, sizeof (dhg_3072), NULL);
    if (dhp_bn == NULL || dhg_bn == NULL || !DH_set0_pqg(dh, dhp_bn, NULL, dhg_bn)) {
        DH_free(dh);
        BN_free(dhp_bn);
        BN_free(dhg_bn);
        return NULL;
    }
    return dh;
}
/*
-----BEGIN DH PARAMETERS-----
MIIBiAKCAYEAuGL7hIWBFwOfaR/xRT1hDWJ19KYUXz6i1Kry2SeENDFzrioZf61F
66D2Epf+sqfGxU4aQ1qH5PVMTFKkBV5bkDhSh8e7sxhgQnIJoUfh8JbSkgvw9Aoh
iuMwaGEnHG2oddmbkJS3CBBbNXAjKlBW2AFMvR+p7yf9rmJ0JY5qglSC2kJYVFZ3
nQI9A55EIbYYM/HClNTurYEMiaVARQF3PxHguGNsag+UB0aEDoRNAqPFOAoofhT1
im/PzVOg/I48Zv+bZSsHFNNXxNXgR+PG9J2QzRx1Shbca2ITofj+NteL4E2g/Lg3
ifZ0WRNfGlMx/EsvochH8S2zh95Gkloq4OzR5FGpwqABGDUkqyeSjSdbhaMVMIJd
F8b48Gkhz+JW7fWmWLfAjyC7x8VLYfjvJ27F7EjO4R9VdUopX99p61IaG845FwYJ
tlAh94IPYbbX9beCL954BIHmeKfDnVv0et/mBci4LIgSIoDm7vZuw1MZaWZ1ZJFZ
zVsnHLYwGSJTAgEC
-----END DH PARAMETERS-----
*/


vector<byte> KeyManager::getSimmetricKeyDH(vector<byte>& pubK, DH* dh_session) {
  vector<byte> ret(sizeof(unsigned char) * (DH_size(dh_session)));

  BIGNUM* recv_pk = BN_new();

  recv_pk = BN_bin2bn(pubK.data(), pubK.size(), recv_pk);

  int sharedkey_size;
  sharedkey_size = DH_compute_key(ret.data(), recv_pk, dh_session);

  ret.resize(sharedkey_size); //safe

  BN_free(recv_pk);
  DH_free(dh_session);

  return ret;
}

DH* KeyManager::generateDHPair_2048(vector<byte>& pub, vector<byte>& priv){
  DH *dh_session = get_dh2048();

  int ec;
  if(DH_check(dh_session,&ec) != 1) {
	cerr << "DH_check() failed" << endl;
	exit(1);
  }

  DH_generate_key(dh_session);

  //Riprendiamo le chiavi
  const BIGNUM* pubkey;
  const BIGNUM* privkey;
  DH_get0_key(dh_session,&pubkey,&privkey);

  pub = vector<byte>(256);
  priv = vector<byte>(256);

  pub.resize(BN_bn2bin(pubkey, pub.data()));    //safe
  priv.resize(BN_bn2bin(privkey, priv.data())); //safe

  return dh_session;
}

DH* KeyManager::generateDHPair_3072(vector<byte>& pub, vector<byte>& priv){
  DH *dh_session = get_dh3072();

  int ec;
  if(DH_check(dh_session,&ec) != 1) {
	cerr << "DH_check() failed" << endl;
	exit(1);
  }

  DH_generate_key(dh_session);

  //Riprendiamo le chiavi
  const BIGNUM* pubkey;
  const BIGNUM* privkey;
  DH_get0_key(dh_session,&pubkey,&privkey);

  pub = vector<byte>(384);
  priv = vector<byte>(384);

  pub.resize(BN_bn2bin(pubkey, pub.data()));    //safe
  priv.resize(BN_bn2bin(privkey, priv.data())); //safe

  return dh_session;
}



/* ============================ RSA =================================================== */

void KeyManager::generateRSA(uint32_t sizeKey, string path) {
  if(sizeKey <= 0) {
    sizeKey = 256;
  }
  RSA* r = RSA_new();
  generateRSA(sizeKey, &r);

  savePUB(path, r);
  savePRIV(path, r);

  RSA_free(r);
  //BN_free(bne);

}

void KeyManager::generateRSA(uint32_t sizeKey, RSA** rsa) {
  if(sizeKey <= 0) {
    sizeKey = 256;
  }

  unsigned long e = RSA_F4;

  BIGNUM* bne = BN_new();

  if(BN_set_word(bne, e) != 1){
    cout << "Error:" << endl;
    BN_free(bne);
  }

  *rsa = RSA_new();

  do{
    RSA_free(*rsa);
    *rsa = RSA_new();
    if(RSA_generate_key_ex(*rsa, sizeKey, bne, NULL) != 1){
      RSA_free(*rsa);
      BN_free(bne);
      exit(1);
    }
  }while(RSA_check_key(*rsa) != 1);

  BN_free(bne);

}

bool KeyManager::savePUB(string path, RSA* rsa){
  // save public key
  string filepath = path + "publicKEY.pem";
  BIO* bp_public = NULL;
  bp_public = BIO_new_file(filepath.data(), "w+");
  int ret = PEM_write_bio_RSAPublicKey(bp_public, rsa);
  if(ret != 1){
      BIO_free_all(bp_public);
      return false;
  }
  BIO_free_all(bp_public);
  return true;
}

bool KeyManager::savePRIV(string path, RSA* rsa){
  //save private key
  string filepath = path + "privateKEY.pem";
  BIO* bp_private = NULL;
  bp_private = BIO_new_file(filepath.data(), "w+");
  int ret = PEM_write_bio_RSAPrivateKey(bp_private, rsa, NULL, NULL, 0, NULL, NULL);
  if(ret != 1){
    BIO_free_all(bp_private);
    return false;
  }
  BIO_free_all(bp_private);
  return true;
}

void KeyManager::RSAtoEVP(RSA* rsa, EVP_PKEY* evp){
  if(evp)
    EVP_PKEY_free(evp);
  evp = EVP_PKEY_new();
  EVP_PKEY_assign_RSA(evp, rsa);
}

void KeyManager::EC_PKEYtoEVP(EC_KEY* ec, EVP_PKEY* evp){
  if(evp)
    EVP_PKEY_free(evp);
  evp = EVP_PKEY_new();
  EVP_PKEY_assign_EC_KEY(evp, ec);
}

/* ==================== PEM ======================== */
EVP_PKEY* KeyManager::loadPUB(string path){
  FILE* pubKey_file = fopen(path.c_str(), "r");
  if(!pubKey_file){
    cerr << "Error: pubKey not found" << endl;
    exit(1);
  }
  EVP_PKEY* evp = PEM_read_PUBKEY(pubKey_file, NULL, NULL, NULL);
  fclose(pubKey_file);
  if(!evp){
    cerr << "Error: pubKey not found" << endl;
    exit(1);
  }

  return evp;
}

EVP_PKEY* KeyManager::loadPRIV(string path){
  FILE* privKey_file = fopen(path.c_str(), "r");
  if(!privKey_file){
    cerr << "Error: privK not found" << endl;
    exit(1);
  }
  EVP_PKEY* evp = PEM_read_PrivateKey(privKey_file, NULL, NULL, NULL);
  fclose(privKey_file);
  if(!evp){
    cerr << "Error: privK not found" << endl;
    exit(1);
  }

  return evp;
}




/************** EC *********************/


//



EVP_PKEY* KeyManager::createDH_PUB_EC(uint32_t reqSize) {
  EVP_PKEY_CTX *pctx, *kctx;
  EVP_PKEY *pkey = NULL, *params = NULL;

  if(NULL == (pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL))) exit(1);

  if(1 != EVP_PKEY_paramgen_init(pctx)) exit(1);

  int nid;
  if(reqSize == 256) nid = NID_X9_62_prime256v1;
  else if(reqSize == 521) nid = 716;
  else return NULL;

  if(1 != EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, nid )) exit(1);

  if (!EVP_PKEY_paramgen(pctx, &params)) exit(1);

  if(NULL == (kctx = EVP_PKEY_CTX_new(params, NULL))) exit(1);

  if(1 != EVP_PKEY_keygen_init(kctx)) exit(1);
  if (1 != EVP_PKEY_keygen(kctx, &pkey)) exit(1);

  EVP_PKEY_CTX_free(kctx);
  EVP_PKEY_free(params);
  EVP_PKEY_CTX_free(pctx);

  return pkey;
}


vector<byte> KeyManager::getSimmetricKeyDH_EC(EVP_PKEY* pkey, EVP_PKEY* PUB_peerkey) {
  byte *secret;
  size_t* secret_len = new size_t;
  EVP_PKEY_CTX *ctx = NULL;
  
  if(NULL == (ctx = EVP_PKEY_CTX_new(pkey, NULL))) exit(1);

  if(1 != EVP_PKEY_derive_init(ctx)) exit(1);

  if(1 != EVP_PKEY_derive_set_peer(ctx, PUB_peerkey)) exit(1);

  if(1 != EVP_PKEY_derive(ctx, NULL, secret_len)) exit(1);

  if(NULL == (secret = (byte*)OPENSSL_malloc(*secret_len))) exit(1);

  if(1 != (EVP_PKEY_derive(ctx, secret, secret_len))) exit(1);

  EVP_PKEY_CTX_free(ctx);
  
  vector<byte> secret_(*secret_len);
  memcpy(secret_.data(), secret, *secret_len);
  
  OPENSSL_free(secret);

  delete secret_len;

  return secret_;
}



vector<byte> KeyManager::serializeEVP_PUB(EVP_PKEY* key) {
  byte* ptr = NULL;
  uint32_t len = i2d_PUBKEY(key, &ptr);
  if(len <= 0 ) {
    cout << "error: " << len << endl;
    exit(1);
  }
  vector<byte> serialized(len);
  memcpy(serialized.data(), ptr, len);
  OPENSSL_free(ptr);
  return serialized;
}


EVP_PKEY* KeyManager::deserializeEVP_PUB(vector<byte>& serialized) {
  uint32_t size = serialized.size();
  const byte* ppin = (const byte*)serialized.data();
  EVP_PKEY* evp = NULL;
  d2i_PUBKEY(&evp, &ppin, size);
  if(evp == NULL)
	  cout << "Error: evp is null" << endl;
  return evp;
}

/*
BLINDING PROTECTION

int RSA_blinding_on(RSA *rsa, BN_CTX *ctx); 1 successo 0 errore
.....SOME CODE......
void RSA_blinding_off(RSA *rsa);

*/
