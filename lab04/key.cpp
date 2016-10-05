#include <stdlib.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <openssl/pem.h>
#include <errno.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define BUFFER_SIZE 1024

using namespace std;

int main(int argc, char * argv[]) {
  const char * filename = argv[1];
  const char * keyFile = argv[2];
  const char * outFilename = argv[3];
	
  if ( argc != 4 )
  {
    fprintf(stderr, "Chybi parametry, zadejte: ./key soubor_s_daty soubor_s_verejnym_klicem vystupni_soubor \n" );
    return errno;
  }

  unsigned char st[BUFFER_SIZE];  // sifrovany text
  unsigned char * key;  // klic pro sifrovani
  unsigned char iv[EVP_MAX_IV_LENGTH];  // inicializacni vektor
  unsigned char readBuffer[BUFFER_SIZE];

  int stLength = 0;
  int tmpLength = 0;
  int readlen = 0;
  int keyLength = 0;
  
  EVP_CIPHER_CTX ctx;
  EVP_PKEY * pubKey = NULL;
  FILE * fPlainIn = fopen(filename, "rb");
  if ( fPlainIn == 0 ) {
    cout << "Soubor " << filename << " nelze otevrit." << endl;
    return 0;
  }
  FILE * fCipherOut = fopen(outFilename, "w+b");
  
  FILE * fPubKey = fopen(keyFile, "rb");  
  if ( fPubKey == 0 ) {
    cout << "Soubor " << keyFile << " nelze otevrit." << endl;
    return 0;
  }
  
  pubKey = PEM_read_PUBKEY(fPubKey, NULL, NULL, NULL);
  fclose(fPubKey);

  keyLength = EVP_PKEY_size(pubKey);
  key = (unsigned char*) malloc(keyLength);
  EVP_SealInit(&ctx, EVP_des_cbc(), &key, &keyLength, iv, &pubKey, 1); // use only 1 public key

  int nid = EVP_CIPHER_CTX_nid(&ctx);
  fwrite(&nid,sizeof(nid),1,fCipherOut);
  fwrite(iv,1,EVP_MAX_IV_LENGTH,fCipherOut);  
  
  fwrite(&keyLength,sizeof(keyLength),1,fCipherOut);
  fwrite(key,1,keyLength,fCipherOut);

  // cout << "NID: " << nid << endl;
  
  while( (readlen = fread(readBuffer, 1, BUFFER_SIZE, fPlainIn)) != 0 )
  {
	  EVP_SealUpdate(&ctx, st,&stLength, readBuffer, readlen);
	  fwrite(st,1,stLength,fCipherOut);
  }
  EVP_SealFinal(&ctx, st, &tmpLength);

  fwrite(st,1,tmpLength,fCipherOut);
  fclose(fPlainIn);
  fclose(fCipherOut);

  cout << "Soubor uspesne zasifrovan jako " << outFilename << endl;
   
  free(key);
 }